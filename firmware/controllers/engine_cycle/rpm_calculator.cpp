/**
 * @file    rpm_calculator.cpp
 * @brief   RPM calculator
 *
 * Here we listen to position sensor events in order to figure our if engine is currently running or not.
 * Actual getRpm() is calculated once per crankshaft revolution, based on the amount of time passed
 * since the start of previous shaft revolution.
 *
 * We also have 'instant RPM' logic separate from this 'cycle RPM' logic. Open question is why do we not use
 * instant RPM instead of cycle RPM more often.
 *
 * @date Jan 1, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#include "globalaccess.h"
#include "os_access.h"
#include "engine.h"
#include "rpm_calculator.h"

#include "trigger_central.h"
#include "engine_configuration.h"
#include "engine_math.h"
#include "perf_trace.h"
#include "tooth_logger.h"

#if EFI_PROD_CODE
#include "os_util.h"
#endif /* EFI_PROD_CODE */


// See RpmCalculator::checkIfSpinning()
#ifndef NO_RPM_EVENTS_TIMEOUT_SECS
#define NO_RPM_EVENTS_TIMEOUT_SECS 2
#endif /* NO_RPM_EVENTS_TIMEOUT_SECS */

float RpmCalculator::getRpmAcceleration() const {
	return 1.0 * previousRpmValue / rpmValue;
}

bool RpmCalculator::isStopped() const {
	// Spinning-up with zero RPM means that the engine is not ready yet, and is treated as 'stopped'.
	return state == STOPPED || (state == SPINNING_UP && rpmValue == 0);
}

bool RpmCalculator::isCranking() const {
	// Spinning-up with non-zero RPM is suitable for all engine math, as good as cranking
	return state == CRANKING || (state == SPINNING_UP && rpmValue > 0);
}

bool RpmCalculator::isSpinningUp() const {
	return state == SPINNING_UP;
}

uint32_t RpmCalculator::getRevolutionCounterSinceStart(void) const {
	return revolutionCounterSinceStart;
}

/**
 * @return -1 in case of isNoisySignal(), current RPM otherwise
 * See NOISY_RPM
 */
// todo: migrate to float return result or add a float version? this would have with calculations
int RpmCalculator::getRpm() const {
	return rpmValue;
}

EXTERN_ENGINE;


RpmCalculator::RpmCalculator() : StoredValueSensor(SensorType::Rpm, 0) {

	// todo: reuse assignRpmValue() method which needs PASS_ENGINE_PARAMETER_SUFFIX
	// which we cannot provide inside this parameter-less constructor. need a solution for this minor mess

	// we need this initial to have not_running at first invocation
	lastRpmEventTimeNt = (efitick_t) DEEP_IN_THE_PAST_SECONDS * NT_PER_SECOND;
}

/**
 * @return true if there was a full shaft revolution within the last second
 */
bool RpmCalculator::isRunning() const {
	return state == RUNNING;
}

/**
 * @return true if engine is spinning (cranking or running)
 */
bool RpmCalculator::checkIfSpinning(efitick_t nowNt) const {
	if (ENGINE(needToStopEngine(nowNt))) {
		return false;
	}

	/**
	 * note that the result of this subtraction could be negative, that would happen if
	 * we have a trigger event between the time we've invoked 'getTimeNow' and here
	 */
	bool noRpmEventsForTooLong = nowNt - lastRpmEventTimeNt >= NT_PER_SECOND * NO_RPM_EVENTS_TIMEOUT_SECS; // Anything below 60 rpm is not running
	/**
	 * Also check if there were no trigger events
	 */
	bool noTriggerEventsForTooLong = nowNt - engine->triggerCentral.triggerState.previousShaftEventTimeNt >= NT_PER_SECOND;
	if (noRpmEventsForTooLong || noTriggerEventsForTooLong) {
		return false;
	}

	return true;
}

void RpmCalculator::assignRpmValue(float floatRpmValue) {
	previousRpmValue = rpmValue;
	// we still persist integer RPM! todo: figure out the next steps
	rpmValue = floatRpmValue;

	if (rpmValue <= 0) {
		oneDegreeUs = NAN;
		invalidate();
	} else {
		setValidValue(floatRpmValue, 0);	// 0 for current time since RPM sensor never times out

		// here it's really important to have more precise float RPM value, see #796
		oneDegreeUs = getOneDegreeTimeUs(floatRpmValue);
		if (previousRpmValue == 0) {
			/**
			 * this would make sure that we have good numbers for first cranking revolution
			 * #275 cranking could be improved
			 */
			ENGINE(periodicFastCallback(PASS_ENGINE_PARAMETER_SIGNATURE));
		}
	}
}

void RpmCalculator::setRpmValue(float value) {
	assignRpmValue(value);
	spinning_state_e oldState = state;
	// Change state
	if (rpmValue == 0) {
		state = STOPPED;
	} else if (rpmValue >= CONFIG(cranking.rpm)) {
		state = RUNNING;
	} else if (state == STOPPED || state == SPINNING_UP) {
		/**
		 * We are here if RPM is above zero but we have not seen running RPM yet.
		 * This gives us cranking hysteresis - a drop of RPM during running is still running, not cranking.
		 */
		state = CRANKING;
	}

	// This presumably fixes injection mode change for cranking-to-running transition.
	// 'isSimultanious' flag should be updated for events if injection modes differ for cranking and running.
	if (state != oldState && CONFIG(crankingInjectionMode) != CONFIG(injectionMode)) {
		// Reset the state of all injectors: when we change fueling modes, we could
		// immediately reschedule an injection that's currently underway.  That will cause
		// the injector's overlappingCounter to get out of sync with reality.  As the fix,
		// every injector's state is forcibly reset just before we could cause that to happen.
		engine->injectionEvents.resetOverlapping();

		// reschedule all injection events now that we've reset them
		engine->injectionEvents.addFuelEvents(PASS_ENGINE_PARAMETER_SIGNATURE);
	}

}

spinning_state_e RpmCalculator::getState() const {
	return state;
}

void RpmCalculator::onNewEngineCycle() {
	revolutionCounterSinceBoot++;
	revolutionCounterSinceStart++;
}

uint32_t RpmCalculator::getRevolutionCounterM(void) const {
	return revolutionCounterSinceBoot;
}

void RpmCalculator::setStopped() {
	revolutionCounterSinceStart = 0;
	if (rpmValue != 0) {
		assignRpmValue(0);

	}
	state = STOPPED;
}

void RpmCalculator::setStopSpinning() {
	isSpinning = false;
	setStopped();
}

void RpmCalculator::setSpinningUp(efitick_t nowNt) {
	if (!CONFIG(isFasterEngineSpinUpEnabled))
		return;
	// Only a completely stopped and non-spinning engine can enter the spinning-up state.
	if (isStopped() && !isSpinning) {
		state = SPINNING_UP;
		engine->triggerCentral.triggerState.spinningEventIndex = 0;
		isSpinning = true;
	}
	// update variables needed by early instant RPM calc.
	if (isSpinningUp()) {
		engine->triggerCentral.triggerState.setLastEventTimeForInstantRpm(nowNt PASS_ENGINE_PARAMETER_SUFFIX);
	}
	/**
	 * Update ignition pin indices if needed. Here we potentially switch to wasted spark temporarily.
	 */
	prepareIgnitionPinIndices(getCurrentIgnitionMode(PASS_ENGINE_PARAMETER_SIGNATURE) PASS_ENGINE_PARAMETER_SUFFIX);
}

/**
 * @brief Shaft position callback used by RPM calculation logic.
 *
 * This callback should always be the first of trigger callbacks because other callbacks depend of values
 * updated here.
 * This callback is invoked on interrupt thread.
 */
void rpmShaftPositionCallback(trigger_event_e ckpSignalType,
		uint32_t index, efitick_t nowNt DECLARE_ENGINE_PARAMETER_SUFFIX) {
	UNUSED(ckpSignalType);
	efiAssertVoid(CUSTOM_ERR_6632, getCurrentRemainingStack() > EXPECTED_REMAINING_STACK, "lowstckRCL");

	RpmCalculator *rpmState = &engine->rpmCalculator;

	if (index == 0) {
		bool hadRpmRecently = rpmState->checkIfSpinning(nowNt);

		if (hadRpmRecently) {
			efitick_t diffNt = nowNt - rpmState->lastRpmEventTimeNt;
		/**
		 * Four stroke cycle is two crankshaft revolutions
		 *
		 * We always do '* 2' because the event signal is already adjusted to 'per engine cycle'
		 * and each revolution of crankshaft consists of two engine cycles revolutions
		 *
		 */
			if (diffNt == 0) {
				rpmState->setRpmValue(NOISY_RPM);
			} else {
				int mult = (int)getEngineCycle(engine->getOperationMode(PASS_ENGINE_PARAMETER_SIGNATURE)) / 360;
				float rpm = 60.0 * NT_PER_SECOND * mult / diffNt;
				rpmState->setRpmValue(rpm > UNREALISTIC_RPM ? NOISY_RPM : rpm);
			}
		}
		rpmState->onNewEngineCycle();
		rpmState->lastRpmEventTimeNt = nowNt;
	}



	if (rpmState->isSpinningUp()) {
		// we are here only once trigger is synchronized for the first time
		// while transitioning  from 'spinning' to 'running'
		// Replace 'normal' RPM with instant RPM for the initial spin-up period
		engine->triggerCentral.triggerState.movePreSynchTimestamps(PASS_ENGINE_PARAMETER_SIGNATURE);
		int prevIndex;
		int instantRpm = engine->triggerCentral.triggerState.calculateInstantRpm(&engine->triggerCentral.triggerFormDetails,
				&prevIndex, nowNt PASS_ENGINE_PARAMETER_SUFFIX);
		// validate instant RPM - we shouldn't skip the cranking state
		instantRpm = minI(instantRpm, CONFIG(cranking.rpm) - 1);
		rpmState->assignRpmValue(instantRpm);
	}
}

static scheduling_s tdcScheduler[2];

static char rpmBuffer[_MAX_FILLER];

/**
 * This callback has nothing to do with actual engine control, it just sends a Top Dead Center mark to the rusEfi console
 * digital sniffer.
 */

/**
 * This trigger callback schedules the actual physical TDC callback in relation to trigger synchronization point.
 */
static void tdcMarkCallback(trigger_event_e ckpSignalType,
		uint32_t index0, efitick_t edgeTimestamp DECLARE_ENGINE_PARAMETER_SUFFIX) {
	(void) ckpSignalType;
	bool isTriggerSynchronizationPoint = index0 == 0;

}


/**
 * @return Current crankshaft angle, 0 to 720 for four-stroke
 */
float getCrankshaftAngleNt(efitick_t timeNt DECLARE_ENGINE_PARAMETER_SUFFIX) {
	efitick_t timeSinceZeroAngleNt = timeNt
			- engine->rpmCalculator.lastRpmEventTimeNt;

	/**
	 * even if we use 'getOneDegreeTimeUs' macros here, it looks like the
	 * compiler is not smart enough to figure out that "A / ( B / C)" could be optimized into
	 * "A * C / B" in order to replace a slower division with a faster multiplication.
	 */
	int rpm = GET_RPM();
	return rpm == 0 ? NAN : timeSinceZeroAngleNt / getOneDegreeTimeNt(rpm);
}

void initRpmCalculator(DECLARE_ENGINE_PARAMETER_SUFFIX) {
	INJECT_ENGINE_REFERENCE(&ENGINE(rpmCalculator));

	if (hasFirmwareError()) {
		return;
	}

	// Only register if not configured to read RPM over OBD2
	if (!CONFIG(consumeObdSensors)) {
		ENGINE(rpmCalculator).Register();
	}

	addTriggerEventListener(tdcMarkCallback, "chart TDC mark", engine);

	addTriggerEventListener(rpmShaftPositionCallback, "rpm reporter", engine);
}

/**
 * Schedules a callback 'angle' degree of crankshaft from now.
 * The callback would be executed once after the duration of time which
 * it takes the crankshaft to rotate to the specified angle.
 */
efitick_t scheduleByAngle(scheduling_s *timer, efitick_t edgeTimestamp, angle_t angle,
		action_s action DECLARE_ENGINE_PARAMETER_SUFFIX) {
	float delayUs = ENGINE(rpmCalculator.oneDegreeUs) * angle;

	efitime_t delayNt = US2NT(delayUs);
	efitime_t delayedTime = edgeTimestamp + delayNt;

	ENGINE(executor.scheduleByTimestampNt(timer, delayedTime, action));

	return delayedTime;
}


