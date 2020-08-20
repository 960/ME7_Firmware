/**
 * @file	engine.cpp
 *
 *
 * This might be a http://en.wikipedia.org/wiki/God_object but that's best way I can
 * express myself in C/C++. I am open for suggestions :)
 *
 * @date May 21, 2014
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#include "engine.h"
#include "allsensors.h"
#include "efi_gpio.h"
#include "trigger_central.h"
#include "fuel_math.h"
#include "engine_math.h"
#include "advance_map.h"
#include "speed_density.h"
#include "advance_map.h"
#include "os_util.h"
#include "software_knock.h"
#include "auxiliaries.h"


#include "perf_trace.h"
#include "sensor.h"
#include "gppwm.h"

#if EFI_TUNER_STUDIO
#include "tunerstudio_outputs.h"
#endif /* EFI_TUNER_STUDIO */

#if EFI_PROD_CODE
#include "bench_test.h"
#else
#define isRunningBenchTest() true
#endif /* EFI_PROD_CODE */

#if (BOARD_TLE8888_COUNT > 0)
#include "gpio/tle8888.h"
#endif

static TriggerState initState CCM_OPTIONAL;


EXTERN_ENGINE;


void Engine::initializeTriggerWaveform( DECLARE_ENGINE_PARAMETER_SUFFIX) {

	// we have a confusing threading model so some synchronization would not hurt
	bool alreadyLocked = lockAnyContext();

	TRIGGER_WAVEFORM(initializeTriggerWaveform(
			engineConfiguration->ambiguousOperationMode,
			engineConfiguration->useOnlyRisingEdgeForTrigger, &engineConfiguration->trigger));

	if (TRIGGER_WAVEFORM(bothFrontsRequired) && engineConfiguration->useOnlyRisingEdgeForTrigger) {

		warning(CUSTOM_ERR_BOTH_FRONTS_REQUIRED, "trigger: both fronts required");
	}


	if (!TRIGGER_WAVEFORM(shapeDefinitionError)) {
		/**
	 	 * this instance is used only to initialize 'this' TriggerWaveform instance
	 	 * #192 BUG real hardware trigger events could be coming even while we are initializing trigger
	 	 */
		initState.resetTriggerState();
		calculateTriggerSynchPoint(&ENGINE(triggerCentral.triggerShape),
				&initState PASS_ENGINE_PARAMETER_SUFFIX);

		if (engine->triggerCentral.triggerShape.getSize() == 0) {
			warning(CUSTOM_ERR_TRIGGER_ZERO, "triggerShape size is zero");
		}
		engine->engineCycleEventCount = TRIGGER_WAVEFORM(getLength());
	}

	if (!alreadyLocked) {
		unlockAnyContext();
	}

	if (!TRIGGER_WAVEFORM(shapeDefinitionError)) {
		prepareOutputSignals(PASS_ENGINE_PARAMETER_SIGNATURE);
	}

}

static void cylinderCleanupControl(DECLARE_ENGINE_PARAMETER_SIGNATURE) {

	bool newValue;
	if (engineConfiguration->isCylinderCleanupEnabled) {
		newValue = !engine->rpmCalculator.isRunning(PASS_ENGINE_PARAMETER_SIGNATURE) && Sensor::get(SensorType::DriverThrottleIntent).value_or(0) > CLEANUP_MODE_TPS;
	} else {
		newValue = false;
	}
	if (newValue != engine->isCylinderCleanupMode) {
		engine->isCylinderCleanupMode = newValue;

	}

}

void Engine::periodicSlowCallback(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	ScopePerf perf(PE::EnginePeriodicSlowCallback);
	updateSlowSensors(PASS_ENGINE_PARAMETER_SIGNATURE);
	

#if EFI_AUXILIARIES
	initAuxiliaries(PASS_ENGINE_PARAMETER_SIGNATURE);
#endif /* EFI_FSIO */

	updateGppwm();

	cylinderCleanupControl(PASS_ENGINE_PARAMETER_SIGNATURE);

	standardAirCharge = getStandardAirCharge(PASS_ENGINE_PARAMETER_SIGNATURE);

#if (BOARD_TLE8888_COUNT > 0)
	static efitick_t tle8888CrankingResetTime = 0;

	if (CONFIG(useTLE8888_cranking_hack) && ENGINE(rpmCalculator).isCranking(PASS_ENGINE_PARAMETER_SIGNATURE)) {
		efitick_t nowNt = getTimeNowNt();
		if (nowNt - tle8888CrankingResetTime > MS2NT(300)) {
			requestTLE8888initialization();
			// let's reset TLE8888 every 300ms while cranking since that's the best we can do to deal with undervoltage reset
			// PS: oh yes, it's a horrible design! Please suggest something better!
			tle8888CrankingResetTime = nowNt;
		}
	}
#endif

	slowCallBackWasInvoked = true;
}


#if (BOARD_TLE8888_COUNT > 0)
extern float vBattForTle8888;
#endif /* BOARD_TLE8888_COUNT */

/**
 * We are executing these heavy (logarithm) methods from outside the trigger callbacks for performance reasons.
 * See also periodicFastCallback
 */

void Engine::setHardCodedPins(DECLARE_ENGINE_PARAMETER_SIGNATURE) {

	setPinConfigurationOverrides(PASS_ENGINE_PARAMETER_SIGNATURE);
}


void Engine::updateSlowSensors(DECLARE_ENGINE_PARAMETER_SIGNATURE) {

	engineState.updateSlowSensors(PASS_ENGINE_PARAMETER_SIGNATURE);

	// todo: move this logic somewhere to sensors folder?
	if (CONFIG(fuelLevelSensor) != EFI_ADC_NONE) {
		float fuelLevelVoltage = getVoltageDivided("fuel", engineConfiguration->fuelLevelSensor PASS_ENGINE_PARAMETER_SUFFIX);
		sensors.fuelTankLevel = interpolateMsg("fgauge", CONFIG(fuelLevelEmptyTankVoltage), 0,
				CONFIG(fuelLevelFullTankVoltage), 100,
				fuelLevelVoltage);
	}
	sensors.vBatt = hasVBatt(PASS_ENGINE_PARAMETER_SIGNATURE) ? getVBatt(PASS_ENGINE_PARAMETER_SIGNATURE) : 12;

#if (BOARD_TLE8888_COUNT > 0)
	// nasty value injection into C driver which would not be able to access Engine class
	vBattForTle8888 = sensors.vBatt;
#endif /* BOARD_TLE8888_COUNT */

	engineState.running.injectorLag = getInjectorLag(sensors.vBatt PASS_ENGINE_PARAMETER_SUFFIX);

}

void Engine::onTriggerSignalEvent(efitick_t nowNt) {
	isSpinning = true;
	lastTriggerToothEventTimeNt = nowNt;
}

Engine::Engine() {
	reset();
}

Engine::Engine(persistent_config_s *config) {
	setConfig(config);
	reset();
}

/**
 * @see scheduleStopEngine()
 * @return true if there is a reason to stop engine
 */
bool Engine::needToStopEngine(efitick_t nowNt) const {
	return stopEngineRequestTimeNt != 0 &&
			nowNt - stopEngineRequestTimeNt	< 3 * NT_PER_SECOND;
}

int Engine::getGlobalConfigurationVersion(void) const {
	return globalConfigurationVersion;
}

void Engine::reset() {
	/**
	 * it's important for fixAngle() that engineCycle field never has zero
	 */
	engineCycle = getEngineCycle(FOUR_STROKE_CRANK_SENSOR);
	memset(&ignitionPin, 0, sizeof(ignitionPin));
}


/**
 * Here we have a bunch of stuff which should invoked after configuration change
 * so that we can prepare some helper structures
 */
void Engine::preCalculate(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
// todo: start using this 'adcToVoltageInputDividerCoefficient' micro-optimization or... throw it away?
#if HAL_USE_ADC
	adcToVoltageInputDividerCoefficient = adcToVolts(1) * engineConfiguration->analogInputDividerCoefficient;
#else
	adcToVoltageInputDividerCoefficient = engineConfigurationPtr->analogInputDividerCoefficient;
#endif


}
void Engine::OnTriggerStateDecodingError() {




	warning(CUSTOM_SYNC_COUNT_MISMATCH, "Sync Mismatch");
	triggerCentral.triggerState.setTriggerErrorState();


	triggerCentral.triggerState.totalTriggerErrorCounter++;

}

void Engine::OnTriggerStateProperState(efitick_t nowNt) {
	Engine *engine = this;
	EXPAND_Engine;

	triggerCentral.triggerState.runtimeStatistics(nowNt PASS_ENGINE_PARAMETER_SUFFIX);

	rpmCalculator.setSpinningUp(nowNt PASS_ENGINE_PARAMETER_SUFFIX);
}

void Engine::OnTriggerSynchronizationLost() {
	Engine *engine = this;
	EXPAND_Engine;

	// Needed for early instant-RPM detection
	engine->rpmCalculator.setStopSpinning(PASS_ENGINE_PARAMETER_SIGNATURE);
}

void Engine::OnTriggerInvalidIndex(int currentIndex) {
	Engine *engine = this;
	EXPAND_Engine;
	// let's not show a warning if we are just starting to spin
	if (GET_RPM_VALUE != 0) {
		warning(CUSTOM_SYNC_ERROR, "sync error: index #%d above total size %d", currentIndex, triggerCentral.triggerShape.getSize());
		triggerCentral.triggerState.setTriggerErrorState();
	}
}

void Engine::OnTriggerSyncronization(bool wasSynchronized) {
	// We only care about trigger shape once we have synchronized trigger. Anything could happen
	// during first revolution and it's fine
	if (wasSynchronized) {
		Engine *engine = this;
		EXPAND_Engine;

		/**
	 	 * We can check if things are fine by comparing the number of events in a cycle with the expected number of event.
	 	 */
		bool isDecodingError = triggerCentral.triggerState.validateEventCounters(&triggerCentral.triggerShape);
		enginePins.triggerErrorPin.setValue(isDecodingError);


		// 'triggerStateListener is not null' means we are running a real engine and now just preparing trigger shape
		// that's a bit of a hack, a sweet OOP solution would be a real callback or at least 'needDecodingErrorLogic' method?
		if (isDecodingError) {
			OnTriggerStateDecodingError();
		}

		engine->triggerErrorDetection.add(isDecodingError);

	}

}

void Engine::setConfig(persistent_config_s *config) {
	this->config = config;
	engineConfigurationPtr = &config->engineConfiguration;
	memset(config, 0, sizeof(persistent_config_s));
}

void Engine::printKnockState(void) {

}

void Engine::knockLogic(float knockVolts DECLARE_ENGINE_PARAMETER_SUFFIX) {
	this->knockVolts = knockVolts;
    knockNow = knockVolts > engineConfiguration->knockVThreshold;
    /**
     * KnockCount is directly proportional to the degrees of ignition
     * advance removed
     * ex: degrees to subtract = knockCount;
     */

    /**
     * TODO use knockLevel as a factor for amount of ignition advance
     * to remove
     * Perhaps allow the user to set a multiplier
     * ex: degrees to subtract = knockCount + (knockLevel * X)
     * X = user configurable multiplier
     */
    if (knockNow) {
        knockEver = true;
        timeOfLastKnockEvent = getTimeNowUs();
        if (knockCount < engineConfiguration->maxKnockSubDeg)
            knockCount++;
    } else if (knockCount >= 1) {
        knockCount--;
	} else {
        knockCount = 0;
    }
}


void Engine::checkShutdown() {

}

bool Engine::isInShutdownMode() const {

	return false;
}

injection_mode_e Engine::getCurrentInjectionMode(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	return rpmCalculator.isCranking(PASS_ENGINE_PARAMETER_SIGNATURE) ? CONFIG(crankingInjectionMode) : CONFIG(injectionMode);
}

// see also in TunerStudio project '[doesTriggerImplyOperationMode] tag
static bool doesTriggerImplyOperationMode(trigger_type_e type) {
	return type != TT_TOOTHED_WHEEL
			&& type != TT_ONE
			&& type != TT_ONE_PLUS_ONE
			&& type != TT_3_1_CAM
			&& type != TT_TOOTHED_WHEEL_60_2
			&& type != TT_TOOTHED_WHEEL_36_1;
}

operation_mode_e Engine::getOperationMode(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	/**
	 * here we ignore user-provided setting for well known triggers.
	 * For instance for Miata NA, there is no reason to allow user to set FOUR_STROKE_CRANK_SENSOR
	 */
	return doesTriggerImplyOperationMode(engineConfiguration->trigger.type) ? triggerCentral.triggerShape.getOperationMode() : engineConfiguration->ambiguousOperationMode;
}

int Engine::getRpmHardLimit(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	return CONFIG(rpmLimit);
}

/**
 * The idea of this method is to execute all heavy calculations in a lower-priority thread,
 * so that trigger event handler/IO scheduler tasks are faster.
 */
void Engine::periodicFastCallback(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	ScopePerf pc(PE::EnginePeriodicFastCallback);
	engineState.periodicFastCallback(PASS_ENGINE_PARAMETER_SIGNATURE);
#if EFI_SOFTWARE_KNOCK
	processLastKnockEvent();
#endif
}

void doScheduleStopEngine(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	engine->stopEngineRequestTimeNt = getTimeNowNt();
	// let's close injectors or else if these happen to be open right now
	enginePins.stopPins();
}

void action_s::execute() {
	efiAssertVoid(CUSTOM_ERR_ASSERT, callback != NULL, "callback==null1");
	callback(param);
}

schfunc_t action_s::getCallback() const {
	return callback;
}

void * action_s::getArgument() const {
	return param;
}

