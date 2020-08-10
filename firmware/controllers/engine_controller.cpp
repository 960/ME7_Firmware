/**
 * @file    engine_controller.cpp
 * @brief   Controllers package entry point code
 *
 *
 *
 * @date Feb 7, 2013
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
#include "os_access.h"
#include "trigger_central.h"
#include "engine_controller.h"
#include "idle_thread.h"
#include "advance_map.h"
#include "rpm_calculator.h"
#include "main_trigger_callback.h"
#include "io_pins.h"
#include "flash_main.h"
#include "bench_test.h"
#include "os_util.h"
#include "engine_math.h"
#include "allsensors.h"
#include "electronic_throttle.h"

#include "malfunction_central.h"

#include "speed_density.h"
#include "local_version_holder.h"
#include "alternator_controller.h"
#include "fuel_math.h"
#include "spark_logic.h"
#include "counter64.h"
#include "perf_trace.h"
#include "boost_control.h"
#include "vvt_control.h"
#include "launch_control.h"
#include "gppwm.h"
#include "tunerstudio.h"
#include "AdcConfiguration.h"
#include "periodic_task.h"
#include "init.h"
#include "adc_inputs.h"
#include "pwm_generator_logic.h"

#include "pwm_tester.h"

#include "pin_repository.h"

#include "cj125.h"


EXTERN_ENGINE;
/**
 * todo: this should probably become 'static', i.e. private, and propagated around explicitly?
 */
Engine ___engine __attribute__((section(CCM_RAM)));
Engine * engine = &___engine;

void initDataStructures(DECLARE_ENGINE_PARAMETER_SIGNATURE) {

	initFuelMap(PASS_ENGINE_PARAMETER_SIGNATURE);
	initTimingMap(PASS_ENGINE_PARAMETER_SIGNATURE);
	initSpeedDensity(PASS_ENGINE_PARAMETER_SIGNATURE);

}

#if EFI_ENABLE_MOCK_ADC

static void initMockVoltage(void) {

}

#endif /* EFI_ENABLE_MOCK_ADC */

static void doPeriodicSlowCallback(DECLARE_ENGINE_PARAMETER_SIGNATURE);

class PeriodicFastController : public PeriodicTimerController {
	void PeriodicTask() override {
		engine->periodicFastCallback();
	}

	int getPeriodMs() override {
		return FAST_CALLBACK_PERIOD_MS;
	}
};

class PeriodicSlowController : public PeriodicTimerController {
	void PeriodicTask() override {
		doPeriodicSlowCallback(PASS_ENGINE_PARAMETER_SIGNATURE);
	}

	int getPeriodMs() override {
		// no reason to have this configurable, looks like everyone is happy with 20Hz
		return 50;
	}
};

static PeriodicFastController fastController;
static PeriodicSlowController slowController;

/**
 * number of SysClock ticks in one ms
 */
#define TICKS_IN_MS  (CH_CFG_ST_FREQUENCY / 1000)

// todo: this overflows pretty fast!
efitimems_t currentTimeMillis(void) {
	// todo: migrate to getTimeNowUs? or not?
	return chVTGetSystemTimeX() / TICKS_IN_MS;
}

// todo: this overflows pretty fast!
efitimesec_t getTimeNowSeconds(void) {
	return currentTimeMillis() / 1000;
}

static void resetAccel(void) {
	engine->engineLoadAccelEnrichment.resetAE();
	engine->tpsAccelEnrichment.resetAE();

	for (unsigned int i = 0; i < efi::size(engine->injectionEvents.elements); i++)
	{
		engine->injectionEvents.elements[i].wallFuel.resetWF();
	}
}

static void doPeriodicSlowCallback(DECLARE_ENGINE_PARAMETER_SIGNATURE) {

	efiAssertVoid(CUSTOM_ERR_6661, getCurrentRemainingStack() > 64, "lowStckOnEv");

	touchTimeCounter();

	slowStartStopButtonCallback(PASS_ENGINE_PARAMETER_SIGNATURE);


	efitick_t nowNt = getTimeNowNt();

	if (nowNt - engine->triggerCentral.vvtSyncTimeNt >= NT_PER_SECOND) {
		// loss of VVT sync
		engine->triggerCentral.vvtSyncTimeNt = 0;
	}
	/**
	 * Update engine RPM state if needed (check timeouts).
	 */
	bool isSpinning = engine->rpmCalculator.checkIfSpinning(nowNt PASS_ENGINE_PARAMETER_SUFFIX);
	if (!isSpinning) {
		engine->rpmCalculator.setStopSpinning(PASS_ENGINE_PARAMETER_SIGNATURE);
	}

	if (ENGINE(directSelfStimulation) || engine->rpmCalculator.isStopped(PASS_ENGINE_PARAMETER_SIGNATURE)) {
		/**
		 * rusEfi usually runs on hardware which halts execution while writing to internal flash, so we
		 * postpone writes to until engine is stopped. Writes in case of self-stimulation are fine.
		 *
		 * todo: allow writing if 2nd bank of flash is used
		 */
#if EFI_INTERNAL_FLASH
		writeToFlashIfPending();
#endif /* EFI_INTERNAL_FLASH */
		resetAccel();
	}

	if (!engine->rpmCalculator.isStopped(PASS_ENGINE_PARAMETER_SIGNATURE)) {
		updatePrimeInjectionPulseState(PASS_ENGINE_PARAMETER_SIGNATURE);
	}

	if (engine->versionForConfigurationListeners.isOld(engine->getGlobalConfigurationVersion())) {
		updateAccelParameters();
	}

	engine->periodicSlowCallback(PASS_ENGINE_PARAMETER_SIGNATURE);

}

void initPeriodicEvents(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	slowController.Start();
	fastController.Start();
}

char * getPinNameByAdcChannel(const char *msg, adc_channel_e hwChannel, char *buffer) {

	if (hwChannel == EFI_ADC_NONE) {
		strcpy(buffer, "NONE");
	} else {
		strcpy(buffer, portname(getAdcChannelPort(msg, hwChannel)));
		itoa10(&buffer[2], getAdcChannelPin(hwChannel));
	}

	return buffer;
}


extern AdcDevice fastAdc;



#define isOutOfBounds(offset) ((offset<0) || (offset) >= (int) sizeof(engine_configuration_s))


// this method is used by real firmware and simulator and unit test
void commonInitEngineController( DECLARE_ENGINE_PARAMETER_SUFFIX) {
	initInterpolation();
	/**
	 * This has to go after 'enginePins.startPins()' in order to
	 * properly detect un-assigned output pins
	 */
	prepareShapes(PASS_ENGINE_PARAMETER_SIGNATURE);
#if EFI_ENABLE_MOCK_ADC
	initMockVoltage();
#endif /* EFI_ENABLE_MOCK_ADC */

	startTunerStudioConnectivity();

#if EFI_PROD_CODE || EFI_SIMULATOR

	if (hasFirmwareError()) {
		return;
	}
#endif

#if !EFI_UNIT_TEST
	// This is tested independently - don't configure sensors for tests.
	// This lets us selectively mock them for each test.
	initNewSensors();
#endif /* EFI_UNIT_TEST */

	initSensors( PASS_ENGINE_PARAMETER_SUFFIX);

	initAccelEnrichment( PASS_ENGINE_PARAMETER_SUFFIX);


	initGpPwm(PASS_ENGINE_PARAMETER_SIGNATURE);

#if EFI_IDLE_CONTROL
	startIdleThread(PASS_ENGINE_PARAMETER_SUFFIX);
#endif /* EFI_IDLE_CONTROL */

#if EFI_ELECTRONIC_THROTTLE_BODY
	initElectronicThrottle(PASS_ENGINE_PARAMETER_SIGNATURE);
#endif /* EFI_ELECTRONIC_THROTTLE_BODY */


#if EFI_BOOST_CONTROL
	initBoostCtrl( PASS_ENGINE_PARAMETER_SUFFIX);
#endif /* EFI_BOOST_CONTROL */

#if EFI_LAUNCH_CONTROL
	initLaunchControl( PASS_ENGINE_PARAMETER_SUFFIX);
#endif

#if EFI_ANTILAG
	initAntiLag( PASS_ENGINE_PARAMETER_SUFFIX);
#endif

	/**
	 * there is an implicit dependency on the fact that 'tachometer' listener is the 1st listener - this case
	 * other listeners can access current RPM value
	 */
	initRpmCalculator( PASS_ENGINE_PARAMETER_SUFFIX);


		/**
		 * This method adds trigger listener which actually schedules ignition
		 */
	initSparkLogic();
	initMainEventListener( PASS_ENGINE_PARAMETER_SUFFIX);

}

#if !EFI_UNIT_TEST

void initEngineContoller( DECLARE_ENGINE_PARAMETER_SUFFIX) {
	enginePins.startPins();
	initBenchTest();
	commonInitEngineController();

#if EFI_CJ125
	/**
	 * this uses SimplePwm which depends on scheduler, has to be initialized after scheduler
	 */
	initCJ125(PASS_ENGINE_PARAMETER_SUFFIX);
#endif /* EFI_CJ125 */


	// periodic events need to be initialized after fuel&spark pins to avoid a warning
	initPeriodicEvents(PASS_ENGINE_PARAMETER_SIGNATURE);

	if (hasFirmwareError()) {
		return;
	}

#if EFI_ALTERNATOR_CONTROL
	initAlternatorCtrl( PASS_ENGINE_PARAMETER_SUFFIX);
#endif /* EFI_ALTERNATOR_CONTROL */

	initEgoAveraging(PASS_ENGINE_PARAMETER_SIGNATURE);

}

/**
 * these two variables are here only to let us know how much RAM is available, also these
 * help to notice when RAM usage goes up - if a code change adds to RAM usage these variables would fail
 * linking process which is the way to raise the alarm
 *
 * You get "cannot move location counter backwards" linker error when you run out of RAM. When you run out of RAM you shall reduce these
 * UNUSED_SIZE constants.
 */
#ifndef RAM_UNUSED_SIZE
#define RAM_UNUSED_SIZE 9600
#endif
#ifndef CCM_UNUSED_SIZE
#define CCM_UNUSED_SIZE 2900
#endif
static char UNUSED_RAM_SIZE[RAM_UNUSED_SIZE];
static char UNUSED_CCM_SIZE[CCM_UNUSED_SIZE] CCM_OPTIONAL;

/**
 * See also VCS_VERSION
 */
int getRusEfiVersion(void) {
	if (UNUSED_RAM_SIZE[0] != 0)
		return 123; // this is here to make the compiler happy about the unused array
	if (UNUSED_CCM_SIZE[0] * 0 != 0)
		return 3211; // this is here to make the compiler happy about the unused array
	return 1234;
}
#endif /* EFI_UNIT_TEST */
