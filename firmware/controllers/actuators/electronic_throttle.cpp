/**
 * @file	electronic_throttle.cpp
 * @brief	Electronic Throttle driver
 *
 * @see test test_etb.cpp
 *
 *
 * Limited user documentation at https://github.com/rusefi/rusefi/wiki/HOWTO_electronic_throttle_body
 *
 * todo: make this more universal if/when we get other hardware options
 *
 * May 2020 two vehicles have driver 500 miles each
 * Sep 2019 two-wire TLE9201 official driving around the block! https://www.youtube.com/watch?v=1vCeICQnbzI
 * May 2019 two-wire TLE7209 now behaves same as three-wire VNH2SP30 "eBay red board" on BOSCH 0280750009
 * Apr 2019 two-wire TLE7209 support added
 * Mar 2019 best results so far achieved with three-wire H-bridges like VNH2SP30 on BOSCH 0280750009
 * Jan 2019 actually driven around the block but still need some work.
 * Jan 2017 status:
 * Electronic throttle body with it's spring is definitely not linear - both P and I factors of PID are required to get any results
 *  PID implementation tested on a bench only
 *  it is believed that more than just PID would be needed, as is this is probably
 *  not usable on a real vehicle. Needs to be tested :)
 *
 * https://raw.githubusercontent.com/wiki/rusefi/rusefi_documentation/oem_docs/VAG/Bosch_0280750009_pinout.jpg
 *
 *  ETB is controlled according to pedal position input (pedal position sensor is a potentiometer)
 *    pedal 0% means pedal not pressed / idle
 *    pedal 100% means pedal all the way down
 *  (not TPS - not the one you can calibrate in TunerStudio)
 *
 *
 * See also pid.cpp
 *
 * Relevant console commands:
 *
 * ETB_BENCH_ENGINE
 * set engine_type 58
 *
 * enable verbose_etb
 * disable verbose_etb
 * ethinfo
 * set mock_pedal_position X
 *
 *
 * set debug_mode 17
 * for PID outputs
 *
 * set etb_p X
 * set etb_i X
 * set etb_d X
 * set etb_o X
 *
 * set_etb_duty X
 *
 * http://rusefi.com/forum/viewtopic.php?f=5&t=592
 *
 * @date Dec 7, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 *
 * This file is part of rusEfi - see http://rusefi.com
 *
 * rusEfi is free software; you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * rusEfi is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "global.h"

#if EFI_ELECTRONIC_THROTTLE_BODY

#include "electronic_throttle.h"
#include "tps.h"
#include "sensor.h"
#include "dc_motor.h"
#include "dc_motors.h"
#include "pid_auto_tune.h"

#if defined(HAS_OS_ACCESS)
#error "Unexpected OS ACCESS HERE"
#endif

#ifndef ETB_MAX_COUNT
#define ETB_MAX_COUNT 2
#endif /* ETB_MAX_COUNT */

static pedal2tps_t pedal2tpsMap("Pedal2Tps");

EXTERN_ENGINE;

static bool startupPositionError = false;

#define STARTUP_NEUTRAL_POSITION_ERROR_THRESHOLD 5

static SensorType indexToTpsSensor(size_t index, bool dcMotorIdleValve) {
	if (dcMotorIdleValve) {
		return SensorType::Tps2;
	}

	switch(index) {
		case 0:  return SensorType::Tps1;
		default: return SensorType::Tps2;
	}
}

static SensorType indexToTpsSensorPrimary(size_t index) {
	switch(index) {
		case 0:  return SensorType::Tps1Primary;
		default: return SensorType::Tps2Primary;
	}
}

static SensorType indexToTpsSensorSecondary(size_t index) {
	switch(index) {
		case 0:  return SensorType::Tps1Secondary;
		default: return SensorType::Tps2Secondary;
	}
}

#if EFI_TUNER_STUDIO
static TsCalMode indexToCalModePriMin(size_t index) {
	switch (index) {
		case 0:  return TsCalMode::Tps1Min;
		default: return TsCalMode::Tps2Min;
	}
}

static TsCalMode indexToCalModePriMax(size_t index) {
	switch (index) {
		case 0:  return TsCalMode::Tps1Max;
		default: return TsCalMode::Tps2Max;
	}
}

static TsCalMode indexToCalModeSecMin(size_t index) {
	switch (index) {
		case 0:  return TsCalMode::Tps1SecondaryMin;
		default: return TsCalMode::Tps2SecondaryMin;
	}
}

static TsCalMode indexToCalModeSecMax(size_t index) {
	switch (index) {
		case 0:  return TsCalMode::Tps1SecondaryMax;
		default: return TsCalMode::Tps2SecondaryMax;
	}
}
#endif // EFI_TUNER_STUDIO

static percent_t directPwmValue = NAN;
static percent_t currentEtbDuty;

#define ETB_DUTY_LIMIT 0.9
// this macro clamps both positive and negative percentages from about -100% to 100%
#define ETB_PERCENT_TO_DUTY(x) (clampF(-ETB_DUTY_LIMIT, 0.01f * (x), ETB_DUTY_LIMIT))

void EtbController::init(SensorType positionSensor, DcMotor *motor, int ownIndex, pid_s *pidParameters, const ValueProvider3D* pedalMap) {
	m_positionSensor = positionSensor;
	m_motor = motor;
	m_myIndex = ownIndex;
	m_pid.initPidClass(pidParameters);
	m_pedalMap = pedalMap;
}

void EtbController::reset() {
	m_shouldResetPid = true;
}

void EtbController::onConfigurationChange(pid_s* previousConfiguration) {
	if (m_motor && !m_pid.isSame(previousConfiguration)) {
		m_shouldResetPid = true;
	}
}

void EtbController::showStatus() {
	m_pid.showPidStatus("ETB");
}

expected<percent_t> EtbController::observePlant() const {
	return Sensor::get(m_positionSensor);
}

void EtbController::setIdlePosition(percent_t pos) {
	m_idlePosition = pos;
}

expected<percent_t> EtbController::getSetpoint() const {
	// A few extra preconditions if throttle control is invalid
	if (startupPositionError) {
		return unexpected;
	}

	// VW ETB idle mode uses an ETB only for idle (a mini-ETB sets the lower stop, and a normal cable
	// can pull the throttle up off the stop.), so we directly control the throttle with the idle position.
	if (CONFIG(dcMotorIdleValve)) {
#if EFI_TUNER_STUDIO
		tsOutputChannels.etbTarget = m_idlePosition;
#endif // EFI_TUNER_STUDIO
		return clampF(0, m_idlePosition, 100);
	}

	// If the pedal map hasn't been set, we can't provide a setpoint.
	if (!m_pedalMap) {
		return unexpected;
	}

	auto pedalPosition = Sensor::get(SensorType::AcceleratorPedal);

	// If the pedal has failed, just use 0 position.
	// This is safer than disabling throttle control - we can at least push the throttle closed
	// and let the engine idle.
	float sanitizedPedal = clampF(0, pedalPosition.value_or(0), 100);
	
	float rpm = GET_RPM();
	float targetFromTable = m_pedalMap->getValue(rpm / RPM_1_BYTE_PACKING_MULT, sanitizedPedal);
	engine->engineState.targetFromTable = targetFromTable;

	percent_t etbIdlePosition = clampF(
									0,
									CONFIG(useETBforIdleControl) ? m_idlePosition : 0,
									100
								);
	percent_t etbIdleAddition = 0.01f * CONFIG(etbIdleThrottleRange) * etbIdlePosition;

	// Interpolate so that the idle adder just "compresses" the throttle's range upward.
	// [0, 100] -> [idle, 100]
	// 0% target from table -> idle position as target
	// 100% target from table -> 100% target position
	percent_t targetPosition = interpolateClamped(0, etbIdleAddition, 100, 100, targetFromTable);
#if EFI_ANTILAG
	if (engine->isAntilagCondition && CONFIG(enableAntiLagAir) && CONFIG(antiLag.antiLagAirSupply == E_THROTTLE)) {
		targetPosition = CONFIG(antiLag.antilagExtraAir);
	}
#endif /* EFI_ANTILAG */
#if EFI_TUNER_STUDIO
	if (m_myIndex == 0) {
		tsOutputChannels.etbTarget = targetPosition;
	}
#endif // EFI_TUNER_STUDIO

	return targetPosition;
}

expected<percent_t> EtbController::getOpenLoop(percent_t target) const {
	float ff = interpolate2d("etbb", target, config->etbBiasBins, config->etbBiasValues);
	engine->engineState.etbFeedForward = ff;
	return ff;
}

expected<percent_t> EtbController::getClosedLoopAutotune(percent_t actualThrottlePosition) {
	// Estimate gain at 60% position - this should be well away from the spring and in the linear region
	bool isPositive = actualThrottlePosition > 60.0f;

	float autotuneAmplitude = 20;

	// End of cycle - record & reset
	if (!isPositive && m_lastIsPositive) {
		efitick_t now = getTimeNowNt();

		// Determine period
		float tu = NT2US((float)(now - m_cycleStartTime)) / 1e6;
		m_cycleStartTime = now;

		// Determine amplitude
		float a = m_maxCycleTps - m_minCycleTps;

		// Filter - it's pretty noisy since the ultimate period is not very many loop periods
		constexpr float alpha = 0.05;
		m_a  = alpha * a  + (1 - alpha) * m_a;
		m_tu = alpha * tu + (1 - alpha) * m_tu;

		// Reset bounds
		m_minCycleTps = 100;
		m_maxCycleTps = 0;

		// Math is for Åström–Hägglund (relay) auto tuning
		// https://warwick.ac.uk/fac/cross_fac/iatl/reinvention/archive/volume5issue2/hornsey

		// Publish to TS state
#if EFI_TUNER_STUDIO
		// Amplitude of input (duty cycle %)
		float b = 2 * autotuneAmplitude;

		// Ultimate gain per A-H relay tuning rule
		float ku = 4 * b / (3.14159f * m_a);

		// The multipliers below are somewhere near the "no overshoot" 
		// and "some overshoot" flavors of the Ziegler-Nichols method
		// Kp
		float kp = 0.35f * ku;
		float ki = 0.25f * ku / m_tu;
		float kd = 0.08f * ku * m_tu;

		// Every 5 cycles (of the throttle), cycle to the next value
		if (m_autotuneCounter == 5) {
			m_autotuneCounter = 0;
			m_autotuneCurrentParam++;

			if (m_autotuneCurrentParam >= 3) {
				m_autotuneCurrentParam = 0;
			}
		}

		m_autotuneCounter++;

		// Multiplex 3 signals on to the {mode, value} format
		tsOutputChannels.calibrationMode = static_cast<TsCalMode>(m_autotuneCurrentParam + 3);

		switch (m_autotuneCurrentParam) {
		case 0:
			tsOutputChannels.calibrationValue = kp;
			break;
		case 1:
			tsOutputChannels.calibrationValue = ki;
			break;
		case 2:
			tsOutputChannels.calibrationValue = kd;
			break;
		}

		// Also output to debug channels if configured
		if (engineConfiguration->debugMode == DBG_ETB_AUTOTUNE) {
			// a - amplitude of output (TPS %)
			tsOutputChannels.debugFloatField1 = m_a;
			// b - amplitude of input (Duty cycle %)
			tsOutputChannels.debugFloatField2 = b;
			// Tu - oscillation period (seconds)
			tsOutputChannels.debugFloatField3 = m_tu;

			tsOutputChannels.debugFloatField4 = ku;
			tsOutputChannels.debugFloatField5 = kp;
			tsOutputChannels.debugFloatField6 = ki;
			tsOutputChannels.debugFloatField7 = kd;
		}
#endif
	}

	m_lastIsPositive = isPositive;

	// Find the min/max of each cycle
	if (actualThrottlePosition < m_minCycleTps) {
		m_minCycleTps = actualThrottlePosition;
	}

	if (actualThrottlePosition > m_maxCycleTps) {
		m_maxCycleTps = actualThrottlePosition;
	}

	// Bang-bang control the output to induce oscillation
	return autotuneAmplitude * (isPositive ? -1 : 1);
}

expected<percent_t> EtbController::getClosedLoop(percent_t target, percent_t observation) {
	if (m_shouldResetPid) {
		m_pid.reset();
		m_shouldResetPid = false;
	}

	// Only report the 0th throttle
	if (m_myIndex == 0) {
#if EFI_TUNER_STUDIO
		// Error is positive if the throttle needs to open further
		tsOutputChannels.etb1Error = target - observation;
#endif /* EFI_TUNER_STUDIO */
	}

	// Only allow autotune with stopped engine
	if (GET_RPM() == 0 && engine->etbAutoTune) {
		return getClosedLoopAutotune(observation);
	} else {
		// Normal case - use PID to compute closed loop part
		return m_pid.getOutput(target, observation, 1.0f / ETB_LOOP_FREQUENCY);
	}
}

void EtbController::setOutput(expected<percent_t> outputValue) {
#if EFI_TUNER_STUDIO
	// Only report first-throttle stats
	if (m_myIndex == 0) {
		tsOutputChannels.etb1DutyCycle = outputValue.value_or(0);
	}
#endif

	if (!m_motor) return;

	// If output is valid and we aren't paused, output to motor.
	if (outputValue) {
		m_motor->enable();
		m_motor->set(ETB_PERCENT_TO_DUTY(outputValue.Value));
	} else {
		m_motor->disable();
	}
}

void EtbController::update(efitick_t) {
#if EFI_TUNER_STUDIO
	// Only debug throttle #0
	if (m_myIndex == 0) {
		// set debug_mode 17
		if (engineConfiguration->debugMode == DBG_ELECTRONIC_THROTTLE_PID) {
			m_pid.postState(&tsOutputChannels);
			tsOutputChannels.debugIntField5 = engine->engineState.etbFeedForward;
		} else if (engineConfiguration->debugMode == DBG_ELECTRONIC_THROTTLE_EXTRA) {
			// set debug_mode 29
			tsOutputChannels.debugFloatField1 = directPwmValue;
		}
	}
#endif /* EFI_TUNER_STUDIO */

	if (!cisnan(directPwmValue)) {
		m_motor->set(directPwmValue);
		return;
	}

#if EFI_TUNER_STUDIO
	if (engineConfiguration->debugMode == DBG_ETB_LOGIC) {
		tsOutputChannels.debugFloatField1 = engine->engineState.targetFromTable;
		tsOutputChannels.debugFloatField2 = engine->engineState.idle.etbIdleAddition;
	}
#endif

	m_pid.iTermMin = engineConfiguration->etb_iTermMin;
	m_pid.iTermMax = engineConfiguration->etb_iTermMax;


	ClosedLoopController::update();

}

void EtbController::autoCalibrateTps() {
	m_isAutocal = true;
}

#if !EFI_UNIT_TEST
/**
 * Things running on a timer (instead of a thread) don't participate it the RTOS's thread priority system,
 * and operate essentially "first come first serve", which risks starvation.
 * Since ETB is a safety critical device, we need the hard RTOS guarantee that it will be scheduled over other less important tasks.
 */
#include "periodic_thread_controller.h"
struct EtbImpl final : public EtbController, public PeriodicController<512> {
	EtbImpl() : PeriodicController("ETB", NORMALPRIO + 3, ETB_LOOP_FREQUENCY) {}

	void PeriodicTask(efitick_t nowNt) override {

#if EFI_TUNER_STUDIO
	if (m_isAutocal) {
		// Don't allow if engine is running!
		if (GET_RPM() > 0) {
			m_isAutocal = false;
			return;
		}

		auto motor = getMotor();
		if (!motor) {
			m_isAutocal = false;
			return;
		}

		size_t myIndex = getMyIndex();

		// First grab open
		motor->set(0.5f);
		motor->enable();
		chThdSleepMilliseconds(1000);
		float primaryMax = Sensor::getRaw(indexToTpsSensorPrimary(myIndex)) * TPS_TS_CONVERSION;
		float secondaryMax = Sensor::getRaw(indexToTpsSensorSecondary(myIndex)) * TPS_TS_CONVERSION;

		// Let it return
		motor->set(0);
		chThdSleepMilliseconds(200);

		// Now grab closed
		motor->set(-0.5f);
		chThdSleepMilliseconds(1000);
		float primaryMin = Sensor::getRaw(indexToTpsSensorPrimary(myIndex)) * TPS_TS_CONVERSION;
		float secondaryMin = Sensor::getRaw(indexToTpsSensorSecondary(myIndex)) * TPS_TS_CONVERSION;

		// Finally disable and reset state
		motor->disable();

		// Write out the learned values to TS, waiting briefly after setting each to let TS grab it
		tsOutputChannels.calibrationMode = indexToCalModePriMax(myIndex);
		tsOutputChannels.calibrationValue = primaryMax;
		chThdSleepMilliseconds(500);
		tsOutputChannels.calibrationMode = indexToCalModePriMin(myIndex);
		tsOutputChannels.calibrationValue = primaryMin;
		chThdSleepMilliseconds(500);

		tsOutputChannels.calibrationMode = indexToCalModeSecMax(myIndex);
		tsOutputChannels.calibrationValue = secondaryMax;
		chThdSleepMilliseconds(500);
		tsOutputChannels.calibrationMode = indexToCalModeSecMin(myIndex);
		tsOutputChannels.calibrationValue = secondaryMin;
		chThdSleepMilliseconds(500);

		tsOutputChannels.calibrationMode = TsCalMode::None;

		m_isAutocal = false;
		return;
	}
#endif /* EFI_TUNER_STUDIO */

		EtbController::update(nowNt);
	}

	void start() override {
		Start();
	}
};

// real implementation (we mock for some unit tests)
EtbImpl etbControllers[ETB_COUNT];
#endif


static void etbPidReset(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	for (int i = 0 ; i < engine->etbActualCount; i++) {
		engine->etbControllers[i]->reset();
	}
}

#if !EFI_UNIT_TEST

/**
 * At the moment there are TWO ways to use this
 * set_etb_duty X
 * set etb X
 * manual duty cycle control without PID. Percent value from 0 to 100
 */
void setThrottleDutyCycle(percent_t level) {

	if (cisnan(level)) {
		directPwmValue = NAN;
		return;
	}

	float dc = ETB_PERCENT_TO_DUTY(level);
	directPwmValue = dc;
	for (int i = 0 ; i < engine->etbActualCount; i++) {
		setDcMotorDuty(i, dc);
	}
}

static void setEtbFrequency(int frequency) {
	engineConfiguration->etbFreq = frequency;

	for (int i = 0 ; i < engine->etbActualCount; i++) {
		setDcMotorFrequency(i, frequency);
	}
}

static void etbReset() {
	
	for (int i = 0 ; i < engine->etbActualCount; i++) {
		setDcMotorDuty(i, 0);
	}

	etbPidReset();
}
#endif /* EFI_PROD_CODE */

#if !EFI_UNIT_TEST
/**
 * set etb_p X
 */
void setEtbPFactor(float value) {
	engineConfiguration->etb.pFactor = value;
	etbPidReset();

}

/**
 * set etb_i X
 */
void setEtbIFactor(float value) {
	engineConfiguration->etb.iFactor = value;
	etbPidReset();

}

/**
 * set etb_d X
 */
void setEtbDFactor(float value) {
	engineConfiguration->etb.dFactor = value;
	etbPidReset();

}

/**
 * set etb_o X
 */
void setEtbOffset(int value) {
	engineConfiguration->etb.offset = value;
	etbPidReset();

}

void etbAutocal(size_t throttleIndex) {
	if (throttleIndex >= ETB_COUNT) {
		return;
	}

	auto etb = engine->etbControllers[throttleIndex];

	if (etb) {
		etb->autoCalibrateTps();
	}
}

#endif /* !EFI_UNIT_TEST */

/**
 * This specific throttle has default position of about 7% open
 */
static const float boschBiasBins[] = {
	0, 1, 5, 7, 14, 100
};
static const float boschBiasValues[] = {
	-15, -15, -10, 0, 19, 20
};

void setBoschVNH2SP30Curve(DECLARE_CONFIG_PARAMETER_SIGNATURE) {
	copyArray(config->etbBiasBins, boschBiasBins);
	copyArray(config->etbBiasValues, boschBiasValues);
}

void setDefaultEtbParameters(DECLARE_CONFIG_PARAMETER_SIGNATURE) {
	CONFIG(etbIdleThrottleRange) = 5;

	setLinearCurve(config->pedalToTpsPedalBins, /*from*/0, /*to*/100, 1);
	setLinearCurve(config->pedalToTpsRpmBins, /*from*/0, /*to*/8000 / RPM_1_BYTE_PACKING_MULT, 1);

	for (int pedalIndex = 0;pedalIndex<PEDAL_TO_TPS_SIZE;pedalIndex++) {
		for (int rpmIndex = 0;rpmIndex<PEDAL_TO_TPS_SIZE;rpmIndex++) {
			config->pedalToTpsTable[pedalIndex][rpmIndex] = config->pedalToTpsPedalBins[pedalIndex];
		}
	}

	engineConfiguration->etbFreq = DEFAULT_ETB_PWM_FREQUENCY;

	// voltage, not ADC like with TPS
	engineConfiguration->throttlePedalUpVoltage = 0.75;
	engineConfiguration->throttlePedalWOTVoltage = 4;

	engineConfiguration->etb = {
		1,		// Kp
		10,		// Ki
		0.05,	// Kd
		0,		// offset
		0,		// Update rate, unused
		-100, 100 // min/max
	};



}

void onConfigurationChangeElectronicThrottleCallback(engine_configuration_s *previousConfiguration) {
#if !EFI_UNIT_TEST
	for (int i = 0; i < ETB_COUNT; i++) {
		etbControllers[i].onConfigurationChange(&previousConfiguration->etb);
	}
#endif
}

#if EFI_PROD_CODE && 0
static void setTempOutput(float value) {
	autoTune.output = value;
}

/**
 * set_etbat_step X
 */
static void setAutoStep(float value) {
	autoTune.reset();
	autoTune.SetOutputStep(value);
}

#endif /* EFI_PROD_CODE */

static const float defaultBiasBins[] = {
	0,  2, 4, 7, 98, 100
};
static const float defaultBiasValues[] = {
	-20, -17, 0, 20, 21, 25
};

void setDefaultEtbBiasCurve(DECLARE_CONFIG_PARAMETER_SIGNATURE) {
	copyArray(config->etbBiasBins, defaultBiasBins);
	copyArray(config->etbBiasValues, defaultBiasValues);
}

void unregisterEtbPins() {
	// todo: we probably need an implementation here?!
}

void doInitElectronicThrottle(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	efiAssertVoid(OBD_PCM_Processor_Fault, engine->etbControllers != NULL, "etbControllers NULL");

	// If you don't have a pedal (or VW idle valve mode), we have no business here.
	if (!CONFIG(dcMotorIdleValve) && !Sensor::hasSensor(SensorType::AcceleratorPedalPrimary)) {
		return;
	}

	pedal2tpsMap.init(config->pedalToTpsTable, config->pedalToTpsPedalBins, config->pedalToTpsRpmBins);

	if (CONFIG(dcMotorIdleValve)) {
		engine->etbActualCount = 1;
	} else {
		engine->etbActualCount = Sensor::hasSensor(SensorType::Tps2) ? 2 : 1;
	}

	for (int i = 0 ; i < engine->etbActualCount; i++) {
		auto motor = initDcMotor(i, engine->etb_use_two_wires PASS_ENGINE_PARAMETER_SUFFIX);

		// If this motor is actually set up, init the etb
		if (motor)
		{
			auto positionSensor = indexToTpsSensor(i, CONFIG(dcMotorIdleValve));
			engine->etbControllers[i]->init(positionSensor, motor, i, &engineConfiguration->etb, &pedal2tpsMap);
			INJECT_ENGINE_REFERENCE(engine->etbControllers[i]);
		}
	}


	etbPidReset(PASS_ENGINE_PARAMETER_SIGNATURE);

	for (int i = 0 ; i < engine->etbActualCount; i++) {
		engine->etbControllers[i]->start();
	}
}

void initElectronicThrottle(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	if (hasFirmwareError()) {
		return;
	}

#if !EFI_UNIT_TEST
	for (int i = 0; i < ETB_COUNT; i++) {
		engine->etbControllers[i] = &etbControllers[i];
	}
#endif

	doInitElectronicThrottle(PASS_ENGINE_PARAMETER_SIGNATURE);
}

void setEtbIdlePosition(percent_t pos DECLARE_ENGINE_PARAMETER_SUFFIX) {
	for (int i = 0; i < ETB_COUNT; i++) {
		auto etb = engine->etbControllers[i];

		if (etb) {
			etb->setIdlePosition(pos);
		}
	}
}

#endif /* EFI_ELECTRONIC_THROTTLE_BODY */
