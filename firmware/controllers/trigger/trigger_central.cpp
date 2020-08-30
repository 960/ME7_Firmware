/*
 * @file	trigger_central.cpp
 * Here we have a bunch of higher-level methods which are not directly related to actual signal decoding
 *
 * @date Feb 23, 2014
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#include "global.h"
#include "os_access.h"

#include "trigger_central.h"
#include "trigger_decoder.h"
#include "main_trigger_callback.h"
#include "engine_configuration.h"
#include "listener_array.h"
#include "pwm_generator_logic.h"
#include "tooth_logger.h"


#include "engine_math.h"
#include "local_version_holder.h"
#include "trigger_simulator.h"

#include "rpm_calculator.h"
#include "tooth_logger.h"
#include "perf_trace.h"

#if EFI_PROD_CODE
#include "pin_repository.h"
#endif /* EFI_PROD_CODE */

#if EFI_TUNER_STUDIO
#include "tunerstudio.h"
#endif /* EFI_TUNER_STUDIO */

trigger_central_s::trigger_central_s() : hwEventCounters() {
	static_assert(TRIGGER_TYPE_60_2 == TT_TOOTHED_WHEEL_60_2, "One day we will have one source of this magic constant");
	static_assert(TRIGGER_TYPE_36_1 == TT_TOOTHED_WHEEL_36_1, "One day we will have one source of this magic constant");
}

TriggerCentral::TriggerCentral() : trigger_central_s() {
	clearCallbacks(&triggerListeneres);
	triggerState.resetTriggerState();
	noiseFilter.resetAccumSignalData();
}

void TriggerNoiseFilter::resetAccumSignalData() {
	memset(lastSignalTimes, 0xff, sizeof(lastSignalTimes));	// = -1
	memset(accumSignalPeriods, 0, sizeof(accumSignalPeriods));
	memset(accumSignalPrevPeriods, 0, sizeof(accumSignalPrevPeriods));
}

int TriggerCentral::getHwEventCounter(int index) const {
	return hwEventCounters[index];
}


EXTERN_ENGINE;


void TriggerCentral::addEventListener(ShaftPositionListener listener, const char *name, Engine *engine) {
	UNUSED(name);
	triggerListeneres.registerCallback((VoidInt)(void*)listener, engine);
}

angle_t TriggerCentral::getVVTPosition() {
	return vvtPosition;
}

/**
 * @brief Adds a trigger event listener
 *
 * Trigger event listener would be invoked on each trigger event. For example, for a 60/2 wheel
 * that would be 116 events: 58 SHAFT_PRIMARY_RISING and 58 SHAFT_PRIMARY_FALLING events.
 */
void addTriggerEventListener(ShaftPositionListener listener, const char *name, Engine *engine) {
	engine->triggerCentral.addEventListener(listener, name, engine);
}

#define miataNbIndex (0)

static bool vvtWithRealDecoder(vvt_mode_e vvtMode) {
	return vvtMode == MIATA_NB2 || vvtMode == VVT_BOSCH_QUICK_START;
}

void hwHandleVvtCamSignal(trigger_value_e front, efitick_t nowNt DECLARE_ENGINE_PARAMETER_SUFFIX) {
	TriggerCentral *tc = &engine->triggerCentral;
	if (front == TV_RISE) {
		tc->vvtEventRiseCounter++;
	} else {
		tc->vvtEventFallCounter++;
	}

	if (!CONFIG(vvtCamSensorUseRise)) {
#if EFI_TOOTH_LOGGER
		if (front == TV_RISE) {
			LogTriggerTooth(SHAFT_SECONDARY_RISING, nowNt PASS_ENGINE_PARAMETER_SUFFIX);
		} else {
			LogTriggerTooth(SHAFT_SECONDARY_FALLING, nowNt PASS_ENGINE_PARAMETER_SUFFIX);
		}
#endif /* EFI_TOOTH_LOGGER */
	}


	if (!vvtWithRealDecoder(engineConfiguration->vvtMode) && (CONFIG(vvtCamSensorUseRise) ^ (front != TV_FALL))) {
		// todo: there should be a way to always use real trigger code for this logic?
		return;
	}


	floatus_t oneDegreeUs = engine->rpmCalculator.oneDegreeUs;
	if (cisnan(oneDegreeUs)) {
		// todo: this code branch is slowing NB2 cranking since we require RPM sync for VVT sync!
		// todo: smarter code
		//
		// we are here if we are getting VVT position signals while engine is not running
		// for example if crank position sensor is broken :)
		return;
	}

	ENGINE(triggerCentral).vvtState.decodeTriggerEvent(
			&ENGINE(triggerCentral).vvtShape,
			nullptr,
			nullptr,
			&engine->vvtTriggerConfiguration,
			front == TV_RISE ? SHAFT_PRIMARY_RISING : SHAFT_PRIMARY_FALLING, nowNt);


	tc->vvtCamCounter++;

	efitick_t offsetNt = nowNt - tc->timeAtVirtualZeroNt;
	angle_t currentPosition = NT2US(offsetNt) / oneDegreeUs;
	// convert engine cycle angle into trigger cycle angle
	currentPosition -= tdcPosition();
	// https://github.com/rusefi/rusefi/issues/1713 currentPosition could be negative that's expected

	tc->currentVVTEventPosition = currentPosition;
	if (engineConfiguration->debugMode == DBG_VVT) {
#if EFI_TUNER_STUDIO
		tsOutputChannels.debugFloatField1 = currentPosition;
#endif /* EFI_TUNER_STUDIO */
	}

	switch(engineConfiguration->vvtMode) {
	case VVT_2JZ:
		// we do not know if we are in sync or out of sync, so we have to be looking for both possibilities
		if ((currentPosition < engineConfiguration->vvtToothMinAngle       || currentPosition > engineConfiguration->vvtToothMaxAngle) &&
		    (currentPosition < engineConfiguration->vvtToothMinAngle + 360 || currentPosition > engineConfiguration->vvtToothMaxAngle + 360)) {
			// outside of the expected range
			return;
		}
		break;
	case MIATA_NB2:
	case VVT_BOSCH_QUICK_START:
	 {
		if (engine->triggerCentral.vvtState.currentCycle.current_index != 0) {
			// this is not NB2 sync tooth - exiting
			return;
		}
		if (engineConfiguration->debugMode == DBG_VVT) {
#if EFI_TUNER_STUDIO
			tsOutputChannels.debugIntField1++;
#endif /* EFI_TUNER_STUDIO */
		}
	}
	default:
		// else, do nothing
		break;
	}

	tc->vvtSyncTimeNt = nowNt;

    // we do NOT clamp VVT position into the [0, engineCycle) range - we expect vvtOffset to be configured so that
    // it's not necessary
	tc->vvtPosition = engineConfiguration->vvtOffset - currentPosition;
	if (tc->vvtPosition < 0 || tc->vvtPosition > ENGINE(engineCycle)) {
		warning(CUSTOM_ERR_VVT_OUT_OF_RANGE, "Please adjust vvtOffset since position %f", tc->vvtPosition);
	}

	switch (engineConfiguration->vvtMode) {
	case VVT_FIRST_HALF:
	{

		bool isEven = tc->triggerState.isEvenRevolution();
		if (!isEven) {
			/**
			 * we are here if we've detected the cam sensor within the wrong crank phase
			 * let's increase the trigger event counter, that would adjust the state of
			 * virtual crank-based trigger
			 */
			tc->triggerState.incrementTotalEventCounter();
			if (engineConfiguration->debugMode == DBG_VVT) {
#if EFI_TUNER_STUDIO
				tsOutputChannels.debugIntField1++;
#endif /* EFI_TUNER_STUDIO */
			}
		}
	}
		break;
	case VVT_SECOND_HALF:
	{
		bool isEven = tc->triggerState.isEvenRevolution();
		if (isEven) {
			// see above comment
			tc->triggerState.incrementTotalEventCounter();
			if (engineConfiguration->debugMode == DBG_VVT) {
#if EFI_TUNER_STUDIO
				tsOutputChannels.debugIntField1++;
#endif /* EFI_TUNER_STUDIO */
			}
		}
	}
		break;
	case MIATA_NB2:
		/**
		 * NB2 is a symmetrical crank, there are four phases total
		 */
		while (tc->triggerState.getTotalRevolutionCounter() % 4 != miataNbIndex) {
			tc->triggerState.incrementTotalEventCounter();
		}
		break;
	default:
	case VVT_INACTIVE:
		// do nothing
		break;
	}

}

#if EFI_PROD_CODE || EFI_SIMULATOR

int triggerReentraint = 0;
int maxTriggerReentraint = 0;
uint32_t triggerDuration;
uint32_t triggerMaxDuration = 0;

void hwHandleShaftSignal(trigger_event_e signal, efitick_t timestamp) {
	ScopePerf perf(PE::HandleShaftSignal);
		LogTriggerTooth(signal, timestamp PASS_ENGINE_PARAMETER_SUFFIX);
	// for effective noise filtering, we need both signal edges, 
	// so we pass them to handleShaftSignal() and defer this test
	
	if (!CONFIG(enableTriggerFilter)) {
		
		const TriggerConfiguration * triggerConfiguration = &engine->primaryTriggerConfiguration;
		if (!isUsefulSignal(signal, triggerConfiguration)) {
			/**
			 * no need to process VR falls further
			 */
			return;
		}
	}

	uint32_t triggerHandlerEntryTime = getTimeNowLowerNt();
	if (triggerReentraint > maxTriggerReentraint)
		maxTriggerReentraint = triggerReentraint;
	triggerReentraint++;

	efiAssertVoid(CUSTOM_ERR_6636, getCurrentRemainingStack() > 128, "lowstck#8");
	engine->triggerCentral.handleShaftSignal(signal, timestamp PASS_ENGINE_PARAMETER_SUFFIX);

	triggerReentraint--;
	triggerDuration = getTimeNowLowerNt() - triggerHandlerEntryTime;
	if (triggerDuration > triggerMaxDuration)
		triggerMaxDuration = triggerDuration;
}
#endif /* EFI_PROD_CODE */

void TriggerCentral::resetCounters() {
	memset(hwEventCounters, 0, sizeof(hwEventCounters));
}

static char shaft_signal_msg_index[15];

static const bool isUpEvent[6] = { false, true, false, true, false, true };
static const char *eventId[6] = { PROTOCOL_CRANK1, PROTOCOL_CRANK1, PROTOCOL_CRANK2, PROTOCOL_CRANK2, PROTOCOL_CRANK3, PROTOCOL_CRANK3 };


/**
 * This is used to filter noise spikes (interference) in trigger signal. See 
 * The basic idea is to use not just edges, but the average amount of time the signal stays in '0' or '1'.
 * So we update 'accumulated periods' to track where the signal is. 
 * And then compare between the current period and previous, with some tolerance (allowing for the wheel speed change).
 * @return true if the signal is passed through.
 */
bool TriggerNoiseFilter::noiseFilter(efitick_t nowNt,
		TriggerState * triggerState,
		trigger_event_e signal DECLARE_ENGINE_PARAMETER_SUFFIX) {
	// todo: find a better place for these defs
	static const trigger_event_e opposite[6] = { SHAFT_PRIMARY_RISING, SHAFT_PRIMARY_FALLING, SHAFT_SECONDARY_RISING, SHAFT_SECONDARY_FALLING, 
			SHAFT_3RD_RISING, SHAFT_3RD_FALLING };
	static const trigger_wheel_e triggerIdx[6] = { T_PRIMARY, T_PRIMARY, T_SECONDARY, T_SECONDARY, T_CHANNEL_3, T_CHANNEL_3 };
	// we process all trigger channels independently
	trigger_wheel_e ti = triggerIdx[signal];
	// falling is opposite to rising, and vise versa
	trigger_event_e os = opposite[signal];
	
	// todo: currently only primary channel is filtered, because there are some weird trigger types on other channels
	if (ti != T_PRIMARY)
		return true;
	
	// update period accumulator: for rising signal, we update '0' accumulator, and for falling - '1'
	if (lastSignalTimes[signal] != -1)
		accumSignalPeriods[signal] += nowNt - lastSignalTimes[signal];
	// save current time for this trigger channel
	lastSignalTimes[signal] = nowNt;
	
	// now we want to compare current accumulated period to the stored one 
	efitick_t currentPeriod = accumSignalPeriods[signal];
	// the trick is to compare between different
	efitick_t allowedPeriod = accumSignalPrevPeriods[os];

	// but first check if we're expecting a gap
	bool isGapExpected = TRIGGER_WAVEFORM(isSynchronizationNeeded) && triggerState->shaft_is_synchronized &&
			(triggerState->currentCycle.eventCount[ti] + 1) == TRIGGER_WAVEFORM(expectedEventCount[ti]);
	
	if (isGapExpected) {
		// usually we need to extend the period for gaps, based on the trigger info
		allowedPeriod *= TRIGGER_WAVEFORM(syncRatioAvg);
	}
	
	// also we need some margin for rapidly changing trigger-wheel speed,
	// that's why we expect the period to be no less than 2/3 of the previous period (this is just an empirical 'magic' coef.)
	efitick_t minAllowedPeriod = 2 * allowedPeriod / 3;
	// but no longer than 5/4 of the previous 'normal' period
	efitick_t maxAllowedPeriod = 5 * allowedPeriod / 4;
	
	// above all, check if the signal comes not too early
	if (currentPeriod >= minAllowedPeriod) {
		// now we store this period as a reference for the next time,
		// BUT we store only 'normal' periods, and ignore too long periods (i.e. gaps)
		if (!isGapExpected && (maxAllowedPeriod == 0 || currentPeriod <= maxAllowedPeriod)) {
			accumSignalPrevPeriods[signal] = currentPeriod;
		}
		// reset accumulator
		accumSignalPeriods[signal] = 0;
		return true;
	}
	// all premature or extra-long events are ignored - treated as interference
	return false;
}

/**
 * This method is NOT invoked for VR falls.
 */
void TriggerCentral::handleShaftSignal(trigger_event_e signal, efitick_t timestamp DECLARE_ENGINE_PARAMETER_SUFFIX) {
	efiAssertVoid(CUSTOM_CONF_NULL, engine!=NULL, "configuration");

	if (triggerShape.shapeDefinitionError) {
		// trigger is broken, we cannot do anything here
		warning(CUSTOM_ERR_UNEXPECTED_SHAFT_EVENT, "Shaft event while trigger is mis-configured");
		// magic value to indicate a problem
		hwEventCounters[0] = 155;
		return;
	}

	// This code gathers some statistics on signals and compares accumulated periods to filter interference
	if (CONFIG(enableTriggerFilter)) {
		if (!noiseFilter.noiseFilter(timestamp, &triggerState, signal PASS_ENGINE_PARAMETER_SUFFIX)) {
			return;
		}
		const TriggerConfiguration * triggerConfiguration = &engine->primaryTriggerConfiguration;
		// moved here from hwHandleShaftSignal()
		if (!isUsefulSignal(signal, triggerConfiguration)) {
			return;
		}
	}

	engine->onTriggerSignalEvent(timestamp);

	int eventIndex = (int) signal;
	efiAssertVoid(CUSTOM_TRIGGER_EVENT_TYPE, eventIndex >= 0 && eventIndex < HW_EVENT_TYPES, "signal type");
	hwEventCounters[eventIndex]++;


	/**
	 * This invocation changes the state of triggerState
	 */
	triggerState.decodeTriggerEvent(&triggerShape,
			nullptr,
			engine,
			&engine->primaryTriggerConfiguration,
			signal, timestamp);

	/**
	 * If we only have a crank position sensor with four stroke, here we are extending crank revolutions with a 360 degree
	 * cycle into a four stroke, 720 degrees cycle.
	 */
	int triggerIndexForListeners;
	operation_mode_e operationMode = engine->getOperationMode(PASS_ENGINE_PARAMETER_SIGNATURE);
	if (operationMode == FOUR_STROKE_CAM_SENSOR || operationMode == TWO_STROKE) {
		// That's easy - trigger cycle matches engine cycle
		triggerIndexForListeners = triggerState.getCurrentIndex();
	} else {
		int crankDivider = operationMode == FOUR_STROKE_CRANK_SENSOR ? 2 : SYMMETRICAL_CRANK_SENSOR_DIVIDER;

		int crankInternalIndex = triggerState.getTotalRevolutionCounter() % crankDivider;

		triggerIndexForListeners = triggerState.getCurrentIndex() + (crankInternalIndex * getTriggerSize());
	}
	if (triggerIndexForListeners == 0) {
		timeAtVirtualZeroNt = timestamp;
	}

	if (!triggerState.shaft_is_synchronized) {
		// we should not propagate event if we do not know where we are
		return;
	}

	if (triggerState.isValidIndex(&ENGINE(triggerCentral.triggerShape))) {
		ScopePerf perf(PE::ShaftPositionListeners);

		/**
		 * Here we invoke all the listeners - the main engine control logic is inside these listeners
		 */
		for (int i = 0; i < triggerListeneres.currentListenersCount; i++) {
			ShaftPositionListener listener = (ShaftPositionListener) (void*) triggerListeneres.callbacks[i];
			(listener)(signal, triggerIndexForListeners, timestamp PASS_ENGINE_PARAMETER_SUFFIX);
		}

	}
}

EXTERN_ENGINE;






#if EFI_PROD_CODE
extern PwmConfig triggerSignal;
#endif /* #if EFI_PROD_CODE */

extern uint32_t hipLastExecutionCount;
extern uint32_t hwSetTimerDuration;

extern uint32_t maxLockedDuration;
extern uint32_t maxEventCallbackDuration;

extern int perSecondIrqDuration;
extern int perSecondIrqCounter;

#if EFI_PROD_CODE
extern uint32_t maxPrecisionCallbackDuration;
#endif /* EFI_PROD_CODE  */

extern uint32_t maxSchedulingPrecisionLoss;
extern uint32_t *cyccnt;

void resetMaxValues() {
#if EFI_PROD_CODE || EFI_SIMULATOR
	maxEventCallbackDuration = triggerMaxDuration = 0;
#endif /* EFI_PROD_CODE || EFI_SIMULATOR */

	maxSchedulingPrecisionLoss = 0;

#if EFI_CLOCK_LOCKS
	maxLockedDuration = 0;
#endif /* EFI_CLOCK_LOCKS */

#if EFI_PROD_CODE
	maxPrecisionCallbackDuration = 0;
#endif /* EFI_PROD_CODE  */
}

#if HAL_USE_ICU == TRUE
extern int icuRisingCallbackCounter;
extern int icuFallingCallbackCounter;
#endif /* HAL_USE_ICU */

void triggerInfo(void) {
#if EFI_PROD_CODE || EFI_SIMULATOR

	TriggerWaveform *ts = &engine->triggerCentral.triggerShape;


#if (HAL_TRIGGER_USE_PAL == TRUE) && (PAL_USE_CALLBACKS == TRUE)

#else

#if HAL_USE_ICU == TRUE

#endif /* HAL_USE_ICU */

#endif /* HAL_TRIGGER_USE_PAL */


	if (engineConfiguration->trigger.type == TT_TOOTHED_WHEEL) {

	}


	if (ts->needSecondTriggerInput) {

	}


	if (TRIGGER_WAVEFORM(isSynchronizationNeeded)) {

	}

#endif /* EFI_PROD_CODE || EFI_SIMULATOR */

#if EFI_PROD_CODE
	if (HAVE_CAM_INPUT()) {
	}


	if (ts->needSecondTriggerInput) {


	}




	resetMaxValues();

#endif /* EFI_PROD_CODE */
}

static void resetRunningTriggerCounters() {
#if !EFI_UNIT_TEST
	engine->triggerCentral.resetCounters();
#endif
}

void onConfigurationChangeTriggerCallback(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	bool changed = false;
	for (int i = 0; i < CAM_INPUTS_COUNT; i++) {
		changed |= isConfigurationChanged(pinCam[i]);
	}

	changed |=
		isConfigurationChanged(trigger.type) ||
		isConfigurationChanged(ambiguousOperationMode) ||
		isConfigurationChanged(useOnlyRisingEdgeForTrigger) ||
		isConfigurationChanged(globalTriggerAngleOffset) ||
		isConfigurationChanged(trigger.numTeeth) ||
		isConfigurationChanged(trigger.missingTeeth) ||
		isConfigurationChanged(pinTrigger[0]) ||
		isConfigurationChanged(pinTrigger[1]) ||
		isConfigurationChanged(pinTrigger[2]) ||
		isConfigurationChanged(vvtMode) ||
		isConfigurationChanged(vvtCamSensorUseRise) ||
		isConfigurationChanged(vvtOffset);
	if (changed) {
		assertEngineReference();


		ENGINE(initializeTriggerWaveform(PASS_ENGINE_PARAMETER_SUFFIX));
		engine->triggerCentral.noiseFilter.resetAccumSignalData();

	}

	// we do not want to miss two updates in a row
	engine->isTriggerConfigChanged = engine->isTriggerConfigChanged || changed;
}

/**
 * @returns true if configuration just changed, and if that change has affected trigger
 */
bool checkIfTriggerConfigChanged(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	bool result = engine->triggerVersion.isOld(engine->getGlobalConfigurationVersion()) && engine->isTriggerConfigChanged;
	engine->isTriggerConfigChanged = false; // whoever has called the method is supposed to react to changes
	return result;
}

bool isTriggerConfigChanged(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	return engine->isTriggerConfigChanged;
}

void initTriggerCentral() {
	
	strcpy((char*) shaft_signal_msg_index, "x_");



}

/**
 * @return TRUE is something is wrong with trigger decoding
 */
bool isTriggerDecoderError(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	return engine->triggerErrorDetection.sum(6) > 4;
}

