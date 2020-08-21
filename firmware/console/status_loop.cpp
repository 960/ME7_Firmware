/**
 * @file	status_loop.cpp
 * @brief Human-readable protocol status messages
 *
 * http://rusefi.com/forum/viewtopic.php?t=263 rusEfi console overview
 * http://rusefi.com/forum/viewtopic.php?t=210 Commands overview
 *
 *
 * @date Mar 15, 2013
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
 *
 */

#include "global.h"
#include "status_loop.h"

#include "engine_controller.h"

#include "adc_inputs.h"
#if EFI_LOGIC_ANALYZER

#endif /* EFI_LOGIC_ANALYZER */

#include "trigger_central.h"
#include "allsensors.h"
#include "sensor_reader.h"
#include "io_pins.h"
#include "efi_gpio.h"

#include "console_io.h"
#include "malfunction_central.h"
#include "speed_density.h"

#include "advance_map.h"
#include "tunerstudio.h"
#include "fuel_math.h"
#include "main_trigger_callback.h"
#include "engine_math.h"
#include "spark_logic.h"
#include "idle_thread.h"
#include "engine_configuration.h"
#include "os_util.h"

#include "engine.h"


#include "can_hw.h"
#include "periodic_thread_controller.h"


extern afr_Map3D_t afrMap;
extern bool main_loop_started;

#if EFI_PROD_CODE
// todo: move this logic to algo folder!
#include "rtc_helper.h"

#include "rusefi.h"
#include "pin_repository.h"
#include "flash_main.h"
#include "can_vss.h"
#include "vehicle_speed.h"
#include "single_timer_executor.h"
#include "periodic_task.h"
extern int icuRisingCallbackCounter;
extern int icuFallingCallbackCounter;
#endif /* EFI_PROD_CODE */

#if EFI_CJ125
#include "cj125.h"
#endif /* EFI_CJ125 */

#if EFI_MAP_AVERAGING

#endif

#if EFI_FSIO

#endif /* EFI_FSIO */

#if (BOARD_TLE8888_COUNT > 0)
#include "tle8888.h"
#endif /* BOARD_TLE8888_COUNT */

#if EFI_ENGINE_SNIFFER
#include "engine_sniffer.h"
extern WaveChart waveChart;
#endif /* EFI_ENGINE_SNIFFER */

int warningEnabled = true;

extern int maxTriggerReentraint;
extern uint32_t maxLockedDuration;





EXTERN_ENGINE;

/**
 * This is useful if we are chan engine mode dynamically
 * For example http://rusefi.com/forum/viewtopic.php?f=5&t=1085
 */
static int packEngineMode(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	return (engineConfiguration->fuelAlgorithm << 4) +
			(engineConfiguration->injectionMode << 2) +
			engineConfiguration->ignitionMode;
}

static float getAirFlowGauge(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	return hasMafSensor() ? getRealMaf(PASS_ENGINE_PARAMETER_SIGNATURE) : engine->engineState.airFlow;
}




/**
 * Time when the firmware version was reported last time, in seconds
 * TODO: implement a request/response instead of just constantly sending this out
 */


#if EFI_PROD_CODE

#endif /* EFI_PROD_CODE */

#ifndef FIRMWARE_ID
#define FIRMWARE_ID "source"
#endif



static bool isTriggerErrorNow() {
	bool justHadError = (getTimeNowNt() - engine->triggerCentral.triggerState.lastDecodingErrorTime) < MS2NT(200);
	return justHadError || isTriggerDecoderError();
}

extern bool consoleByteArrived;


#if EFI_TUNER_STUDIO

void updateTunerStudioState(TunerStudioOutputChannels *tsOutputChannels DECLARE_ENGINE_PARAMETER_SUFFIX) {

	int rpm = GET_RPM();

#if EFI_PROD_CODE
	executorStatistics();
#endif /* EFI_PROD_CODE */

	// header
	tsOutputChannels->tsConfigVersion = TS_FILE_VERSION;

	// offset 0
	tsOutputChannels->rpm = rpm;

	SensorResult clt = Sensor::get(SensorType::Clt);
	tsOutputChannels->coolantTemperature = clt.Value;
	tsOutputChannels->isCltError = !clt.Valid;

	SensorResult iat = Sensor::get(SensorType::Iat);
	tsOutputChannels->intakeAirTemperature = iat.Value;
	tsOutputChannels->isIatError = !iat.Valid;

	SensorResult auxTemp1 = Sensor::get(SensorType::AuxTemp1);
	tsOutputChannels->auxTemp1 = auxTemp1.Value;

	SensorResult auxTemp2 = Sensor::get(SensorType::AuxTemp2);
	tsOutputChannels->auxTemp2 = auxTemp2.Value;

	SensorResult tps1 = Sensor::get(SensorType::Tps1);
	tsOutputChannels->throttlePosition = tps1.Value;
	tsOutputChannels->isTpsError = !tps1.Valid;
	tsOutputChannels->tpsADC = convertVoltageTo10bitADC(Sensor::getRaw(SensorType::Tps1Primary));

	SensorResult tps2 = Sensor::get(SensorType::Tps2);
	tsOutputChannels->throttle2Position = tps2.Value;

	SensorResult pedal = Sensor::get(SensorType::AcceleratorPedal);
	tsOutputChannels->pedalPosition = pedal.Value;
	// Only report fail if you have one (many people don't)
	tsOutputChannels->isPedalError = !pedal.Valid && Sensor::hasSensor(SensorType::AcceleratorPedalPrimary);

	// Set raw sensors
	tsOutputChannels->rawTps1Primary = Sensor::getRaw(SensorType::Tps1Primary);
	tsOutputChannels->rawPpsPrimary = Sensor::getRaw(SensorType::AcceleratorPedalPrimary);
	tsOutputChannels->rawClt = Sensor::getRaw(SensorType::Clt);
	tsOutputChannels->rawIat = Sensor::getRaw(SensorType::Iat);
	tsOutputChannels->rawOilPressure = Sensor::getRaw(SensorType::OilPressure);

	// offset 16
	tsOutputChannels->massAirFlowVoltage = hasMafSensor() ? getMafVoltage(PASS_ENGINE_PARAMETER_SIGNATURE) : 0;

	if (hasAfrSensor(PASS_ENGINE_PARAMETER_SIGNATURE)) {
		// offset 20
		tsOutputChannels->airFuelRatio = getAfr(PASS_ENGINE_PARAMETER_SIGNATURE);
	}
	// offset 24
	tsOutputChannels->engineLoad = getEngineLoadT(PASS_ENGINE_PARAMETER_SIGNATURE);

	tsOutputChannels->fuelingLoad = getFuelingLoad(PASS_ENGINE_PARAMETER_SIGNATURE);
	tsOutputChannels->ignitionLoad = getIgnitionLoad(PASS_ENGINE_PARAMETER_SIGNATURE);

	// KLUDGE? we always show VBatt because Proteus board has VBatt input sensor hardcoded
	// offset 28
	tsOutputChannels->vBatt = getVBatt(PASS_ENGINE_PARAMETER_SIGNATURE);

	// offset 36
#if EFI_ANALOG_SENSORS
	tsOutputChannels->baroPressure = hasBaroSensor() ? getBaroPressure() : 0;
#endif /* EFI_ANALOG_SENSORS */
	// 48
	tsOutputChannels->fuelBase = engine->engineState.baseFuel;
	// 64
	tsOutputChannels->actualLastInjection = ENGINE(actualLastInjection);


	// 104
	tsOutputChannels->rpmAcceleration = engine->rpmCalculator.getRpmAcceleration();
	// offset 108
	// For air-interpolated tCharge mode, we calculate a decent massAirFlow approximation, so we can show it to users even without MAF sensor!
	tsOutputChannels->massAirFlow = getAirFlowGauge(PASS_ENGINE_PARAMETER_SIGNATURE);
	// offset 116
	// TPS acceleration
	tsOutputChannels->deltaTps = engine->tpsAccelEnrichment.getMaxDelta();
	// 128
	tsOutputChannels->totalTriggerErrorCounter = engine->triggerCentral.triggerState.totalTriggerErrorCounter;
	// 132
	tsOutputChannels->orderingErrorCounter = engine->triggerCentral.triggerState.orderingErrorCounter;
	// 68
	tsOutputChannels->baroCorrection = engine->engineState.baroCorrection;
	// 140

	tsOutputChannels->injectorDutyCycle = getInjectorDutyCycle(rpm PASS_ENGINE_PARAMETER_SUFFIX);

	// 148
	tsOutputChannels->fuelTankLevel = engine->sensors.fuelTankLevel;
	// 160
	const auto& wallFuel = ENGINE(injectionEvents.elements[0].wallFuel);
	tsOutputChannels->wallFuelAmount = wallFuel.getWallFuel();
	// 168
	tsOutputChannels->wallFuelCorrection = wallFuel.wallFuelCorrection;

	// 164
	tsOutputChannels->iatCorrection = ENGINE(engineState.running.intakeTemperatureCoefficient);
	// 184
	tsOutputChannels->cltCorrection = ENGINE(engineState.running.coolantTemperatureCoefficient);
	// 188
	tsOutputChannels->fuelRunning = ENGINE(engineState.running.fuel);
	// 196
	tsOutputChannels->injectorLagMs = ENGINE(engineState.running.injectorLag);
	// 224
	efitimesec_t timeSeconds = getTimeNowSeconds();
	tsOutputChannels->timeSeconds = timeSeconds;


	// 248
	tsOutputChannels->vvtPosition = engine->triggerCentral.getVVTPosition();


	// 252
	tsOutputChannels->engineMode = packEngineMode(PASS_ENGINE_PARAMETER_SIGNATURE);
	// 120
	tsOutputChannels->firmwareVersion = getRusEfiVersion();
	// 268
	tsOutputChannels->shortTermFuelTrim = 100.0f * (ENGINE(engineState.running.pidCorrection) - 1.0f);
	// 276
	tsOutputChannels->accelerationX = engine->sensors.accelerometer.x;
	// 278
	tsOutputChannels->accelerationY = engine->sensors.accelerometer.y;
	// 280
	tsOutputChannels->oilPressure = Sensor::get(SensorType::OilPressure).Value;
	// 288
	tsOutputChannels->injectionOffset = engine->engineState.injectionOffset;

	if (hasMapSensor(PASS_ENGINE_PARAMETER_SIGNATURE)) {
		float mapValue = getMap(PASS_ENGINE_PARAMETER_SIGNATURE);
		// // offset 112
		tsOutputChannels->veValue = engine->engineState.currentBaroCorrectedVE * PERCENT_MULT;
		// todo: bug here? target afr could work based on real MAF?
		tsOutputChannels->currentTargetAfr = afrMap.getValue(rpm, mapValue);
		// offset 40
		tsOutputChannels->manifoldAirPressure = mapValue;
	}

	//tsOutputChannels->knockCount = engine->knockCount;
	//tsOutputChannels->knockLevel = engine->knockVolts;

	tsOutputChannels->hasCriticalError = hasFirmwareError();

	tsOutputChannels->isWarnNow = engine->engineState.warnings.isWarningNow(timeSeconds, true);


	tsOutputChannels->tpsAccelFuel = engine->engineState.tpsAccelEnrich;
	// engine load acceleration
	if (hasMapSensor(PASS_ENGINE_PARAMETER_SIGNATURE)) {
		tsOutputChannels->engineLoadAccelExtra = engine->engineLoadAccelEnrichment.getEngineLoadEnrichment(PASS_ENGINE_PARAMETER_SIGNATURE) * 100 / getMap(PASS_ENGINE_PARAMETER_SIGNATURE);
	}
	tsOutputChannels->engineLoadDelta = engine->engineLoadAccelEnrichment.getMaxDelta();

	tsOutputChannels->checkEngine = hasErrorCodes();

#if	HAL_USE_ADC
	tsOutputChannels->internalMcuTemperature = getMCUInternalTemperature();
#endif /* HAL_USE_ADC */

#if EFI_MAX_31855
	for (int i = 0; i < EGT_CHANNEL_COUNT; i++)
		tsOutputChannels->egtValues.values[i] = getEgtValue(i);
#endif /* EFI_MAX_31855 */

#if EFI_IDLE_CONTROL
	tsOutputChannels->idlePosition = getIdlePosition();
#endif

#if EFI_PROD_CODE
	tsOutputChannels->isTriggerError = isTriggerErrorNow();

#if EFI_INTERNAL_FLASH
	tsOutputChannels->needBurn = getNeedToWriteConfiguration();
#endif /* EFI_INTERNAL_FLASH */


	tsOutputChannels->isFuelPumpOn = enginePins.fuelPumpRelay.getLogicValue();
	tsOutputChannels->isFanOn = enginePins.fanRelay.getLogicValue();
	tsOutputChannels->isO2HeaterOn = enginePins.o2heater.getLogicValue();
	tsOutputChannels->enableIgnition = engineConfiguration->enableIgnition;
	tsOutputChannels->enableInjectors = engineConfiguration->enableInjectors;
	tsOutputChannels->isCylinderCleanupEnabled = engineConfiguration->isCylinderCleanupEnabled;
	tsOutputChannels->isCylinderCleanupActivated = engine->isCylinderCleanupMode;
	tsOutputChannels->secondTriggerChannelEnabled = engineConfiguration->secondTriggerChannelEnabled;
#if EFI_VEHICLE_SPEED
	float vehicleSpeed = getVehicleSpeed();
	tsOutputChannels->vehicleSpeedKph = vehicleSpeed;
	tsOutputChannels->speedToRpmRatio = vehicleSpeed / rpm;
#else
	float vehicleCanSpeed = getVehicleCanSpeed();
	tsOutputChannels->vehicleSpeedKph = vehicleCanSpeed;
	tsOutputChannels->speedToRpmRatio = vehicleCanSpeed / rpm;
#endif /* EFI_VEHICLE_SPEED */
#endif /* EFI_PROD_CODE */

	tsOutputChannels->fuelConsumptionPerHour = engine->engineState.fuelConsumption.perSecondConsumption;

	tsOutputChannels->warningCounter = engine->engineState.warnings.warningCounter;
	tsOutputChannels->lastErrorCode = engine->engineState.warnings.lastErrorCode;
	for (int i = 0; i < 8;i++) {
		tsOutputChannels->recentErrorCodes[i] = engine->engineState.warnings.recentWarnings.get(i);
	}

	tsOutputChannels->knockNowIndicator = engine->knockCount > 0;
	tsOutputChannels->knockEverIndicator = engine->knockEver;

	tsOutputChannels->clutchUpState = engine->clutchUpState;
	tsOutputChannels->clutchDownState = engine->clutchDownState;
	tsOutputChannels->brakePedalState = engine->brakePedalState;
	tsOutputChannels->acSwitchState = engine->acSwitchState;


	// tCharge depends on the previous state, so we should use the stored value.
	tsOutputChannels->tCharge = ENGINE(engineState.sd.tCharge);
	float timing = engine->engineState.timingAdvance;
	tsOutputChannels->ignitionAdvance = timing > 360 ? timing - 720 : timing;
	// 60
	tsOutputChannels->sparkDwell = ENGINE(engineState.sparkDwell);
	tsOutputChannels->crankingFuelMs = ENGINE(engineState.cranking.fuel);
	tsOutputChannels->chargeAirMass = engine->engineState.sd.airMassInOneCylinder;

	tsOutputChannels->coilDutyCycle = getCoilDutyCycle(rpm PASS_ENGINE_PARAMETER_SUFFIX);


	switch (engineConfiguration->debugMode)	{
	case DBG_START_STOP:
		tsOutputChannels->debugIntField1 = engine->startStopStateToggleCounter;
		break;
	case DBG_STATUS:
		tsOutputChannels->debugFloatField1 = timeSeconds;

		break;
	case DBG_METRICS:
#if EFI_CLOCK_LOCKS
		tsOutputChannels->debugIntField1 = maxLockedDuration;
		tsOutputChannels->debugIntField2 = maxTriggerReentraint;
#endif /* EFI_CLOCK_LOCKS */
		break;
	case DBG_TPS_ACCEL:
		tsOutputChannels->debugIntField1 = engine->tpsAccelEnrichment.cb.getSize();
		break;
	case DBG_SR5_PROTOCOL: {
		const int _10_6 = 100000;
		tsOutputChannels->debugIntField1 = tsState.textCommandCounter * _10_6 +  tsState.totalCounter;
		tsOutputChannels->debugIntField2 = tsState.outputChannelsCommandCounter * _10_6 + tsState.writeValueCommandCounter;
		tsOutputChannels->debugIntField3 = tsState.readPageCommandsCounter * _10_6 + tsState.burnCommandCounter;
		break;
		}
	case DBG_AUX_VALVES:
		tsOutputChannels->debugFloatField1 = engine->engineState.auxValveStart;
		tsOutputChannels->debugFloatField2 = engine->engineState.auxValveEnd;
		break;
	case DBG_TRIGGER_COUNTERS:
		tsOutputChannels->debugIntField1 = engine->triggerCentral.getHwEventCounter((int)SHAFT_PRIMARY_FALLING);
		tsOutputChannels->debugIntField2 = engine->triggerCentral.getHwEventCounter((int)SHAFT_SECONDARY_FALLING);
// no one uses shaft so far		tsOutputChannels->debugIntField3 = engine->triggerCentral.getHwEventCounter((int)SHAFT_3RD_FALLING);
#if EFI_PROD_CODE && HAL_USE_ICU == TRUE
		tsOutputChannels->debugIntField3 = icuRisingCallbackCounter + icuFallingCallbackCounter;
		tsOutputChannels->debugIntField4 = engine->triggerCentral.vvtEventRiseCounter;
		tsOutputChannels->debugIntField5 = engine->triggerCentral.vvtEventFallCounter;
#endif /* EFI_PROD_CODE */

		tsOutputChannels->debugFloatField1 = engine->triggerCentral.getHwEventCounter((int)SHAFT_PRIMARY_RISING);
		tsOutputChannels->debugFloatField2 = engine->triggerCentral.getHwEventCounter((int)SHAFT_SECONDARY_RISING);

		tsOutputChannels->debugIntField4 = engine->triggerCentral.triggerState.currentCycle.eventCount[0];
		tsOutputChannels->debugIntField5 = engine->triggerCentral.triggerState.currentCycle.eventCount[1];

		// debugFloatField6 used
		// no one uses shaft so far		tsOutputChannels->debugFloatField3 = engine->triggerCentral.getHwEventCounter((int)SHAFT_3RD_RISING);
		break;
	case DBG_FSIO_ADC:
		break;
	case DBG_FSIO_EXPRESSION:
		break;
	case DBG_VEHICLE_SPEED_SENSOR:
		tsOutputChannels->debugIntField1 = engine->engineState.vssEventCounter;
		break;
	case DBG_CRANKING_DETAILS:
		tsOutputChannels->debugIntField1 = engine->rpmCalculator.getRevolutionCounterSinceStart();
		break;
#if EFI_CJ125 && HAL_USE_SPI
	case DBG_CJ125:
		cjPostState(tsOutputChannels);
		break;
#endif /* EFI_CJ125 && HAL_USE_SPI */
#if EFI_MAP_AVERAGING
		case DBG_MAP:
		postMapState(tsOutputChannels);
		break;
#endif /* EFI_MAP_AVERAGING */
#if EFI_CAN_SUPPORT
	case DBG_CAN:
		postCanState(tsOutputChannels);
		break;
#endif /* EFI_CAN_SUPPORT */
	case DBG_ANALOG_INPUTS:
		tsOutputChannels->debugFloatField1 = (engineConfiguration->vbattAdcChannel != EFI_ADC_NONE) ? getVoltageDivided("vbatt", engineConfiguration->vbattAdcChannel PASS_ENGINE_PARAMETER_SUFFIX) : 0.0f;
		tsOutputChannels->debugFloatField2 = Sensor::getRaw(SensorType::Tps1);
		tsOutputChannels->debugFloatField3 = (engineConfiguration->mafAdcChannel != EFI_ADC_NONE) ? getVoltageDivided("maf", engineConfiguration->mafAdcChannel PASS_ENGINE_PARAMETER_SUFFIX) : 0.0f;
		tsOutputChannels->debugFloatField4 = (engineConfiguration->map.sensor.hwChannel != EFI_ADC_NONE) ? getVoltageDivided("map", engineConfiguration->map.sensor.hwChannel PASS_ENGINE_PARAMETER_SUFFIX) : 0.0f;
		tsOutputChannels->debugFloatField5 = (engineConfiguration->clt.adcChannel != EFI_ADC_NONE) ? getVoltageDivided("clt", engineConfiguration->clt.adcChannel PASS_ENGINE_PARAMETER_SUFFIX) : 0.0f;
		tsOutputChannels->debugFloatField6 = (engineConfiguration->iat.adcChannel != EFI_ADC_NONE) ? getVoltageDivided("iat", engineConfiguration->iat.adcChannel PASS_ENGINE_PARAMETER_SUFFIX) : 0.0f;
		tsOutputChannels->debugFloatField7 = (engineConfiguration->afr.hwChannel != EFI_ADC_NONE) ? getVoltageDivided("ego", engineConfiguration->afr.hwChannel PASS_ENGINE_PARAMETER_SUFFIX) : 0.0f;
		break;
	case DBG_ANALOG_INPUTS2:
		tsOutputChannels->debugFloatField1 = Sensor::get(SensorType::Tps1Primary).value_or(0) - Sensor::get(SensorType::Tps1Secondary).value_or(0);
		tsOutputChannels->debugFloatField2 = Sensor::get(SensorType::Tps2Primary).value_or(0) - Sensor::get(SensorType::Tps2Secondary).value_or(0);
		break;
	case DBG_INSTANT_RPM:
		{
			float instantRpm = engine->triggerCentral.triggerState.instantRpm;
			tsOutputChannels->debugFloatField1 = instantRpm;
			tsOutputChannels->debugFloatField2 = instantRpm / GET_RPM_VALUE;
		}
		break;
	case DBG_ION:
		break;
	case DBG_TLE8888:
#if (BOARD_TLE8888_COUNT > 0)
		tle8888PostState(tsOutputChannels->getDebugChannels());
#endif /* BOARD_TLE8888_COUNT */
		break;
	default:
		;
	}
}

static void initStatusLeds(void) {

	enginePins.triggerErrorPin.initPin("Trigger Error", CONFIG(triggerErrorPin));

	enginePins.debugTriggerSync.initPin("debug: sync", CONFIG(debugTriggerSync));
}
void prepareTunerStudioOutputs(void) {
	// sensor state for EFI Analytics Tuner Studio
	updateTunerStudioState(&tsOutputChannels PASS_ENGINE_PARAMETER_SUFFIX);

}

#endif /* EFI_TUNER_STUDIO */


void startStatusThreads(void) {

	initStatusLeds();

}
