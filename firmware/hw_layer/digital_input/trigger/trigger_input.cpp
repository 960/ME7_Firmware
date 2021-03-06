/*
 * @file	trigger_input.cpp
 *
 * @date Nov 11, 2019
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#include "global.h"
#include "trigger_input.h"

/* TODO:
 * - merge comparator trigger
 */


EXTERN_ENGINE;

#if (HAL_USE_ICU == TRUE) || (HAL_TRIGGER_USE_PAL == TRUE)

#if (HAL_USE_ICU == TRUE)
	void icuTriggerTurnOnInputPins();
	int  icuTriggerTurnOnInputPin(const char *msg, int index, bool isTriggerShaft);
	void icuTriggerTurnOffInputPin(brain_pin_e brainPin);
	void icuTriggerSetPrimaryChannel(brain_pin_e brainPin);
#else
	#define icuTriggerTurnOnInputPins() ((void)0)
	int  icuTriggerTurnOnInputPin(const char *msg, int index, bool isTriggerShaft) {
		UNUSED(msg);
		UNUSED(index);
		UNUSED(isTriggerShaft);

		return -2;
	}
	#define icuTriggerTurnOffInputPin(brainPin) ((void)0)
	#define icuTriggerSetPrimaryChannel(brainPin) ((void)0)
#endif

#if (HAL_TRIGGER_USE_PAL == TRUE)
	void extiTriggerTurnOnInputPins();
	int  extiTriggerTurnOnInputPin(const char *msg, int index, bool isTriggerShaft);
	void extiTriggerTurnOffInputPin(brain_pin_e brainPin);
	void extiTriggerSetPrimaryChannel(brain_pin_e brainPin);
#else
	#define extiTriggerTurnOnInputPins() ((void)0)
	int  extiTriggerTurnOnInputPin(const char *msg, int index, bool isTriggerShaft) {
		UNUSED(msg);
		UNUSED(index);
		UNUSED(isTriggerShaft);

		return -2;
	}
	#define extiTriggerTurnOffInputPin(brainPin) ((void)0)
	#define extiTriggerSetPrimaryChannel(brainPin) ((void)0)
#endif

enum triggerType {
	TRIGGER_NONE,
	TRIGGER_ICU,
	TRIGGER_EXTI
};

static triggerType shaftTriggerType[TRIGGER_SUPPORTED_CHANNELS];
static triggerType camTriggerType[CAM_INPUTS_COUNT];

static int turnOnTriggerInputPin(const char *msg, int index, bool isTriggerShaft) {
	brain_pin_e brainPin = isTriggerShaft ?
		CONFIG(pinTrigger)[index] : engineConfiguration->pinCam[index];

	if (isTriggerShaft)
		shaftTriggerType[index] = TRIGGER_NONE;
	else
		camTriggerType[index] = TRIGGER_NONE;

	if (brainPin == GPIO_UNASSIGNED)
		return 0;

	/* try ICU first */
#if EFI_ICU_INPUTS
	if (icuTriggerTurnOnInputPin(msg, index, isTriggerShaft) >= 0) {
		if (isTriggerShaft)
			shaftTriggerType[index] = TRIGGER_ICU;
		else
			camTriggerType[index] = TRIGGER_ICU;
		return 0;
	}
#endif

	/* ... then EXTI */
	if (extiTriggerTurnOnInputPin(msg, index, isTriggerShaft) >= 0) {
		if (isTriggerShaft)
			shaftTriggerType[index] = TRIGGER_EXTI;
		else
			camTriggerType[index] = TRIGGER_EXTI;
		return 0;
	}

	firmwareError(CUSTOM_ERR_NOT_INPUT_PIN, "%s: Not input pin %s", msg, hwPortname(brainPin));

	return -1;
}

static void turnOffTriggerInputPin(int index, bool isTriggerShaft) {
	brain_pin_e brainPin = isTriggerShaft ?
		activeConfiguration.pinTrigger[index] : activeConfiguration.pinCam[index];

	if (isTriggerShaft) {
#if EFI_ICU_INPUTS
		if (shaftTriggerType[index] == TRIGGER_ICU)
			icuTriggerTurnOffInputPin(brainPin);
#endif
		if (shaftTriggerType[index] == TRIGGER_EXTI)
			extiTriggerTurnOffInputPin(brainPin);

		shaftTriggerType[index] = TRIGGER_NONE;
	} else {
#if EFI_ICU_INPUTS
		if (camTriggerType[index] == TRIGGER_ICU)
			icuTriggerTurnOffInputPin(brainPin);
#endif
		if (camTriggerType[index] == TRIGGER_EXTI)
			extiTriggerTurnOffInputPin(brainPin);

		camTriggerType[index] = TRIGGER_NONE;
	}
}

/*==========================================================================*/
/* Exported functions.														*/
/*==========================================================================*/

void stopTriggerInputPins(void) {
	for (int i = 0; i < TRIGGER_SUPPORTED_CHANNELS; i++) {
		if (isConfigurationChanged(pinTrigger[i])) {
			turnOffTriggerInputPin(i, true);
		}
	}
	for (int i = 0; i < CAM_INPUTS_COUNT; i++) {
		if (isConfigurationChanged(pinCam[i])) {
			turnOffTriggerInputPin(i, false);
		}
	}
}

void startTriggerInputPins(void) {
	for (int i = 0; i < TRIGGER_SUPPORTED_CHANNELS; i++) {
		if (isConfigurationChanged(pinTrigger[i])) {
			const char * msg = (i == 0 ? "Trigger #1" : (i == 1 ? "Trigger #2" : "Trigger #3"));
			turnOnTriggerInputPin(msg, i, true);
		}
	}

	for (int i = 0; i < CAM_INPUTS_COUNT; i++) {
		if (isConfigurationChanged(pinCam[i])) {
			turnOnTriggerInputPin("Cam", i, false);
		}
	}

	icuTriggerSetPrimaryChannel(CONFIG(pinTrigger)[0]);
	extiTriggerSetPrimaryChannel(CONFIG(pinTrigger)[0]);
}

void turnOnTriggerInputPins() {
	/* init all trigger HW available */
	icuTriggerTurnOnInputPins();
	extiTriggerTurnOnInputPins();

	applyNewTriggerInputPins();
}

#endif /* (HAL_USE_ICU == TRUE) || (HAL_TRIGGER_USE_PAL == TRUE) */

void applyNewTriggerInputPins(void) {
	// first we will turn off all the changed pins
	stopTriggerInputPins();
	// then we will enable all the changed pins
	startTriggerInputPins();
}


