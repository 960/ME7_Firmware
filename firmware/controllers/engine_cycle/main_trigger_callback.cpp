/**
 * @file    main_trigger_callback.cpp
 * @brief   Main logic is here!
 *
 * See http://rusefi.com/docs/html/
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


#include "main_trigger_callback.h"
#include "efi_gpio.h"
#include "engine_math.h"
#include "trigger_central.h"
#include "spark_logic.h"
#include "rpm_calculator.h"
#include "engine_configuration.h"
#include "interpolation.h"
#include "advance_map.h"
#include "allsensors.h"
#include "cyclic_buffer.h"
#include "fuel_math.h"

#include "engine_controller.h"
#include "efi_gpio.h"
#include "tooth_logger.h"
#include "os_util.h"
#include "local_version_holder.h"
#include "event_queue.h"
#include "engine.h"
#include "perf_trace.h"
#include "sensor.h"

#include "backup_ram.h"

EXTERN_ENGINE;

static const char *prevOutputName = nullptr;

static InjectionEvent primeInjEvent;


void startSimultaniousInjection(Engine *engine) {
	efitick_t nowNt = getTimeNowNt();
	for (int i = 0; i < engine->engineConfigurationPtr->specs.cylindersCount; i++) {
		enginePins.injectors[i].open(nowNt PASS_ENGINE_PARAMETER_SUFFIX);
	}
}

static void endSimultaniousInjectionOnlyTogglePins(Engine *engine) {
	efitick_t nowNt = getTimeNowNt();
	for (int i = 0; i < engine->engineConfigurationPtr->specs.cylindersCount; i++) {
		enginePins.injectors[i].close(nowNt PASS_ENGINE_PARAMETER_SUFFIX);
	}
}

void endSimultaniousInjection(InjectionEvent *event) {
	event->isScheduled = false;
	endSimultaniousInjectionOnlyTogglePins(engine);
	engine->injectionEvents.addFuelEventsForCylinder(event->ownIndex PASS_ENGINE_PARAMETER_SUFFIX);
}

void InjectorOutputPin::open(efitick_t nowNt DECLARE_ENGINE_PARAMETER_SUFFIX) {
	overlappingCounter++;
	if (overlappingCounter > 1) {
	} else {
		setHigh();
	}
}

void turnInjectionPinHigh(InjectionEvent *event) {
	efitick_t nowNt = getTimeNowNt();
	for (int i = 0;i < MAX_WIRES_COUNT;i++) {
		InjectorOutputPin *output = event->outputs[i];

		if (output) {
			output->open(nowNt PASS_ENGINE_PARAMETER_SUFFIX);
		}
	}
}

void InjectorOutputPin::close(efitick_t nowNt DECLARE_ENGINE_PARAMETER_SUFFIX) {

	overlappingCounter--;
	if (overlappingCounter > 0) {
	} else {
		setLow();
	}

	// Don't allow negative overlap count
	if (overlappingCounter < 0) {
		overlappingCounter = 0;
	}
}

void turnInjectionPinLow(InjectionEvent *event) {
	efitick_t nowNt = getTimeNowNt();


	event->isScheduled = false;
	for (int i = 0;i<MAX_WIRES_COUNT;i++) {
		InjectorOutputPin *output = event->outputs[i];
		if (output) {
			output->close(nowNt PASS_ENGINE_PARAMETER_SUFFIX);
		}
	}
	ENGINE(injectionEvents.addFuelEventsForCylinder(event->ownIndex PASS_ENGINE_PARAMETER_SUFFIX));
}

void InjectionEvent::onTriggerTooth(size_t trgEventIndex, int rpm, efitick_t nowNt) {
	uint32_t eventIndex = injectionStart.triggerEventIndex;
	if (eventIndex != trgEventIndex) {
		return;
	}

	const floatms_t injectionDuration = wallFuel.adjust(ENGINE(injectionDuration) PASS_ENGINE_PARAMETER_SUFFIX);

	bool isCranking = ENGINE(rpmCalculator).isCranking();
	/**
	 * todo: pre-calculate 'numberOfInjections'
	 * see also injectorDutyCycle
	 */
	if (!isCranking && injectionDuration * getNumberOfInjections(engineConfiguration->injectionMode PASS_ENGINE_PARAMETER_SUFFIX) > getEngineCycleDuration(rpm PASS_ENGINE_PARAMETER_SUFFIX)) {
		warning(CUSTOM_TOO_LONG_FUEL_INJECTION, "Too long fuel injection %.2fms", injectionDuration);
	} else if (isCranking && injectionDuration * getNumberOfInjections(engineConfiguration->crankingInjectionMode PASS_ENGINE_PARAMETER_SUFFIX) > getEngineCycleDuration(rpm PASS_ENGINE_PARAMETER_SUFFIX)) {
		warning(CUSTOM_TOO_LONG_CRANKING_FUEL_INJECTION, "Too long cranking fuel injection %.2fms", injectionDuration);
	}

	// Store 'pure' injection duration (w/o injector lag) for fuel rate calc.
	engine->engineState.fuelConsumption.addData(injectionDuration - ENGINE(engineState.running.injectorLag));
	
	ENGINE(actualLastInjection) = injectionDuration;
	if (cisnan(injectionDuration)) {
		warning(CUSTOM_OBD_NAN_INJECTION, "NaN injection pulse");
		return;
	}
	if (injectionDuration < 0) {
		warning(CUSTOM_OBD_NEG_INJECTION, "Negative injection pulse %.2f", injectionDuration);
		return;
	}

	if (injectionDuration < 0.050f)
	{
		return;
	}

	floatus_t durationUs = MS2US(injectionDuration);


	// we are ignoring low RPM in order not to handle "engine was stopped to engine now running" transition
	if (rpm > 2 * engineConfiguration->cranking.rpm) {
		const char *outputName = outputs[0]->name;
		if (prevOutputName == outputName
				&& engineConfiguration->injectionMode != IM_SIMULTANEOUS
				&& engineConfiguration->injectionMode != IM_SINGLE_POINT) {
			warning(CUSTOM_OBD_SKIPPED_FUEL, "looks like skipped fuel event revCounter=%d %s", getRevolutionCounter(), outputName);
		}
		prevOutputName = outputName;
	}

	if (isScheduled) {
		return; // this InjectionEvent is still needed for an extremely long injection scheduled previously
	}

	isScheduled = true;

	action_s startAction, endAction;
	// We use different callbacks based on whether we're running sequential mode or not - everything else is the same
	if (isSimultanious) {
		startAction = { &startSimultaniousInjection, engine };
		endAction = { &endSimultaniousInjection, this };
	} else {
		// sequential or batch
		startAction = { &turnInjectionPinHigh, this };
		endAction = { &turnInjectionPinLow, this };
	}

	efitick_t startTime = scheduleByAngle(&signalTimerUp, nowNt, injectionStart.angleOffsetFromTriggerEvent, startAction PASS_ENGINE_PARAMETER_SUFFIX);
	efitick_t turnOffTime = startTime + US2NT((int)durationUs);
	engine->executor.scheduleByTimestampNt(&endOfInjectionEvent, turnOffTime, endAction);

}

static ALWAYS_INLINE void handleFuel(const bool limitedFuel, uint32_t trgEventIndex, int rpm, efitick_t nowNt DECLARE_ENGINE_PARAMETER_SUFFIX) {
	ScopePerf perf(PE::HandleFuel);
	
	efiAssertVoid(CUSTOM_STACK_6627, getCurrentRemainingStack() > 128, "lowstck#3");
	efiAssertVoid(CUSTOM_ERR_6628, trgEventIndex < engine->engineCycleEventCount, "handleFuel/event index");

	if (!isInjectionEnabled(PASS_ENGINE_PARAMETER_SIGNATURE) || limitedFuel) {
		return;
	}
	if (ENGINE(isCylinderCleanupMode)) {
		return;
	}

	// If duty cycle is high, impose a fuel cut rev limiter.
	// This is safer than attempting to limp along with injectors or a pump that are out of flow.
	if (getInjectorDutyCycle(rpm PASS_ENGINE_PARAMETER_SUFFIX) > 96.0f) {
		return;
	}

	/**
	 * Injection events are defined by addFuelEvents() according to selected
	 * fueling strategy
	 */
	FuelSchedule *fs = &ENGINE(injectionEvents);
	if (!fs->isReady) {
		fs->addFuelEvents(PASS_ENGINE_PARAMETER_SIGNATURE);
	}

	ENGINE(tpsAccelEnrichment.onNewValue(Sensor::get(SensorType::Tps1).value_or(0) PASS_ENGINE_PARAMETER_SUFFIX));
	if (trgEventIndex == 0) {
		ENGINE(tpsAccelEnrichment.onEngineCycleTps(PASS_ENGINE_PARAMETER_SIGNATURE));
		ENGINE(engineLoadAccelEnrichment.onEngineCycle(PASS_ENGINE_PARAMETER_SIGNATURE));
	}

	fs->onTriggerTooth(trgEventIndex, rpm, nowNt PASS_ENGINE_PARAMETER_SUFFIX);
}

#if EFI_PROD_CODE
/**
 * this field is used as an Expression in IAR debugger
 */
uint32_t *cyccnt = (uint32_t*) &DWT->CYCCNT;
#endif

/**
 * This is the main trigger event handler.
 * Both injection and ignition are controlled from this method.
 */
static void mainTriggerCallback(trigger_event_e ckpSignalType, uint32_t trgEventIndex, efitick_t edgeTimestamp DECLARE_ENGINE_PARAMETER_SUFFIX) {
	ScopePerf perf(PE::MainTriggerCallback);

	(void) ckpSignalType;


	if (engineConfiguration->vvtMode == MIATA_NB2 && engine->triggerCentral.vvtSyncTimeNt == 0) {
		// this is a bit spaghetti code for sure
		// do not spark & do not fuel until we have VVT sync. NB2 is a special case
		// due to symmetrical crank wheel and we need to make sure no spark happens out of sync
		return;
	}

	if (hasFirmwareError()) {
		/**
		 * In case on a major error we should not process any more events.
		 * TODO: add 'pin shutdown' invocation somewhere - coils might be still open here!
		 */
		return;
	}


	if (trgEventIndex >= ENGINE(engineCycleEventCount)) {
		/**
		 * this could happen in case of a trigger error, just exit silently since the trigger error is supposed to be handled already
		 * todo: should this check be somewhere higher so that no trigger listeners are invoked with noise?
		 */
		return;
	}

	int rpm = GET_RPM();
	if (rpm == 0) {
		// this happens while we just start cranking
		// todo: check for 'trigger->is_synchnonized?'
		// TODO: add 'pin shutdown' invocation somewhere - coils might be still open here!
		return;
	}
	if (rpm == NOISY_RPM) {
		warning(OBD_Crankshaft_Position_Sensor_A_Circuit_Malfunction, "noisy trigger");
		// TODO: add 'pin shutdown' invocation somewhere - coils might be still open here!
		return;
	}
	bool limitedSpark = rpm > engine->getRpmHardLimit(PASS_ENGINE_PARAMETER_SIGNATURE);
	bool limitedFuel = rpm > engine->getRpmHardLimit(PASS_ENGINE_PARAMETER_SIGNATURE);

	if (CONFIG(boostCutPressure) != 0) {
		if (getMap(PASS_ENGINE_PARAMETER_SIGNATURE) > CONFIG(boostCutPressure)) {
			limitedSpark = true;
			limitedFuel = true;
		}
	}

#if EFI_LAUNCH_CONTROL
		int cutRpmRange = engineConfiguration->launch.hardCutRpmRange;
		int launchAdvanceRpmRange = engineConfiguration->launch.launchAdvanceRpmRange;
		int launchRpm = engineConfiguration->launch.launchRpm;
		if (engine->isLaunchCondition) {
			if ((engineConfiguration->enableLaunchRetard) && ((launchRpm + launchAdvanceRpmRange + cutRpmRange) < rpm)) {
				limitedSpark = (engineConfiguration->enableLaunchIgnCut);
				limitedFuel = (engineConfiguration->enableLaunchFuelCut);
			}
			if ((!engineConfiguration->enableLaunchRetard) && ((launchRpm + cutRpmRange) < rpm)) {
				limitedSpark = (engineConfiguration->enableLaunchIgnCut);
				limitedFuel = (engineConfiguration->enableLaunchFuelCut);
			}
		}
#endif


	if (trgEventIndex == 0) {
		if (HAVE_CAM_INPUT()) {
			engine->triggerCentral.validateCamVvtCounters();
		}

		if (checkIfTriggerConfigChanged(PASS_ENGINE_PARAMETER_SIGNATURE)) {
			engine->ignitionEvents.isReady = false; // we need to rebuild complete ignition schedule
			engine->injectionEvents.isReady = false;
			// moved 'triggerIndexByAngle' into trigger initialization (why was it invoked from here if it's only about trigger shape & optimization?)
			// see initializeTriggerWaveform() -> prepareOutputSignals(PASS_ENGINE_PARAMETER_SIGNATURE)

			// we need this to apply new 'triggerIndexByAngle' values
			engine->periodicFastCallback(PASS_ENGINE_PARAMETER_SIGNATURE);
		}
	}

	efiAssertVoid(CUSTOM_IGN_MATH_STATE, !CONFIG(useOnlyRisingEdgeForTrigger) || CONFIG(ignMathCalculateAtIndex) % 2 == 0, "invalid ignMathCalculateAtIndex");

	if (trgEventIndex == (uint32_t)CONFIG(ignMathCalculateAtIndex)) {
		if (CONFIG(externalKnockSenseAdc) != EFI_ADC_NONE) {
			float externalKnockValue = getVoltageDivided("knock", engineConfiguration->externalKnockSenseAdc PASS_ENGINE_PARAMETER_SUFFIX);
			engine->knockLogic(externalKnockValue PASS_ENGINE_PARAMETER_SUFFIX);
		}
	}


	/**
	 * For fuel we schedule start of injection based on trigger angle, and then inject for
	 * specified duration of time
	 */
	handleFuel(limitedFuel, trgEventIndex, rpm, edgeTimestamp PASS_ENGINE_PARAMETER_SUFFIX);
	/**
	 * For spark we schedule both start of coil charge and actual spark based on trigger angle
	 */
	onTriggerEventSparkLogic(limitedSpark, trgEventIndex, rpm, edgeTimestamp PASS_ENGINE_PARAMETER_SUFFIX);
}

// Check if the engine is not stopped or cylinder cleanup is activated
static bool isPrimeInjectionPulseSkipped(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	if (!engine->rpmCalculator.isStopped())
		return true;
	return CONFIG(isCylinderCleanupEnabled) && (Sensor::get(SensorType::Tps1).value_or(0) > CLEANUP_MODE_TPS);
}

/**
 * Prime injection pulse, mainly needed for mono-injectors or long intake manifolds.
 * See testStartOfCrankingPrimingPulse()
 */
void startPrimeInjectionPulse(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	INJECT_ENGINE_REFERENCE(&primeInjEvent);

	// First, we need a protection against 'fake' ignition switch on and off (i.e. no engine started), to avoid repeated prime pulses.
	// So we check and update the ignition switch counter in non-volatile backup-RAM
#if EFI_PROD_CODE
	uint32_t ignSwitchCounter = backupRamLoad(BACKUP_IGNITION_SWITCH_COUNTER);
#else /* EFI_PROD_CODE */
	uint32_t ignSwitchCounter = 0;
#endif /* EFI_PROD_CODE */

	// if we're just toying with the ignition switch, give it another chance eventually...
	if (ignSwitchCounter > 10)
		ignSwitchCounter = 0;
	// If we're going to skip this pulse, then save the counter as 0.
	// That's because we'll definitely need the prime pulse next time (either due to the cylinder cleanup or the engine spinning)
	if (isPrimeInjectionPulseSkipped(PASS_ENGINE_PARAMETER_SIGNATURE))
		ignSwitchCounter = -1;
	// start prime injection if this is a 'fresh start'
	if (ignSwitchCounter == 0) {
		primeInjEvent.ownIndex = 0;
		primeInjEvent.isSimultanious = true;

		scheduling_s *sDown = &ENGINE(injectionEvents.elements[0]).endOfInjectionEvent;
		// When the engine is hot, basically we don't need prime inj.pulse, so we use an interpolation over temperature (falloff).
		// If 'primeInjFalloffTemperature' is not specified (by default), we have a prime pulse deactivation at zero celsius degrees, which is okay.
		const float maxPrimeInjAtTemperature = -40.0f;	// at this temperature the pulse is maximal.
		floatms_t pulseLength = interpolateClamped(maxPrimeInjAtTemperature, CONFIG(startOfCrankingPrimingPulse),
			CONFIG(primeInjFalloffTemperature), 0.0f, Sensor::get(SensorType::Clt).value_or(70));
		if (pulseLength > 0) {
			startSimultaniousInjection(engine);
			efitimeus_t turnOffDelayUs = (efitimeus_t)efiRound(MS2US(pulseLength), 1.0f);
			engine->executor.scheduleForLater(sDown, turnOffDelayUs, { &endSimultaniousInjectionOnlyTogglePins, engine });
		}
	}
#if EFI_PROD_CODE
	// we'll reset it later when the engine starts
	backupRamSave(BACKUP_IGNITION_SWITCH_COUNTER, ignSwitchCounter + 1);
#endif /* EFI_PROD_CODE */
}

void updatePrimeInjectionPulseState(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
#if EFI_PROD_CODE
	static bool counterWasReset = false;
	if (counterWasReset)
		return;

	if (!engine->rpmCalculator.isStopped()) {
		backupRamSave(BACKUP_IGNITION_SWITCH_COUNTER, 0);
		counterWasReset = true;
	}
#endif /* EFI_PROD_CODE */
}

#if EFI_ENGINE_SNIFFER
#include "engine_sniffer.h"
#endif


void initMainEventListener( DECLARE_ENGINE_PARAMETER_SUFFIX) {
	
	efiAssertVoid(CUSTOM_ERR_6631, engine!=NULL, "null engine");


	addTriggerEventListener(mainTriggerCallback, "main loop", engine);

    // We start prime injection pulse at the early init stage - don't wait for the engine to start spinning!
    if (CONFIG(startOfCrankingPrimingPulse) > 0)
    	startPrimeInjectionPulse(PASS_ENGINE_PARAMETER_SIGNATURE);

}


