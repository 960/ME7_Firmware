/**
 * @file    flash_main.cpp
 * @brief	Higher-level logic of saving data into internal flash memory
 *
 *
 * @date Sep 19, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#include "global.h"
#if EFI_INTERNAL_FLASH
#include "os_access.h"
#include "flash_main.h"

#include "flash_int.h"
#include "engine_math.h"

// this message is part of console API, see FLASH_SUCCESS_MSG in java code
#define FLASH_SUCCESS_MSG "FLASH_SUCESS"

#if EFI_TUNER_STUDIO
#include "tunerstudio.h"
#endif


#include "engine_controller.h"

static bool needToWriteConfiguration = false;

EXTERN_ENGINE;

extern persistent_config_container_s persistentState;

extern engine_configuration_s *engineConfiguration;

/**
 * https://sourceforge.net/p/rusefi/tickets/335/
 *
 * In order to preserve at least one copy of the tune in case of electrical issues address of second configuration copy
 * should be in a different sector of flash since complete flash sectors are erased on write.
 */

crc_t flashStateCrc(persistent_config_container_s *state) {
	return calc_crc((const crc_t*) &state->persistentConfiguration, sizeof(persistent_config_s));
}

void setNeedToWriteConfiguration(void) {

	needToWriteConfiguration = true;
}

bool getNeedToWriteConfiguration(void) {
	return needToWriteConfiguration;
}

void writeToFlashIfPending() {
	if (!getNeedToWriteConfiguration()) {
		return;
	}
	// todo: technically we need a lock here, realistically we should be fine.
	needToWriteConfiguration = false;

	writeToFlashNow();
}

// Erase and write a copy of the configuration at the specified address
template <typename TStorage>
int eraseAndFlashCopy(flashaddr_t storageAddress, const TStorage& data)
{
	intFlashErase(storageAddress, sizeof(TStorage));
	return intFlashWrite(storageAddress, reinterpret_cast<const char*>(&data), sizeof(TStorage));
}

void writeToFlashNow(void) {

	// Set up the container
	persistentState.size = sizeof(persistentState);
	persistentState.version = FLASH_DATA_VERSION;
	persistentState.value = flashStateCrc(&persistentState);

	// Flash two copies
	int result1 = eraseAndFlashCopy(getFlashAddrFirstCopy(), persistentState);
	int result2 = eraseAndFlashCopy(getFlashAddrSecondCopy(), persistentState);

	// handle success/failure
	bool isSuccess = (result1 == FLASH_RETURN_SUCCESS) && (result2 == FLASH_RETURN_SUCCESS);

	if (isSuccess) {

	} else {

	}
	assertEngineReference();


	resetMaxValues();

}

static bool isValidCrc(persistent_config_container_s *state) {
	crc_t result = flashStateCrc(state);
	int isValidCrc_b = result == state->value;
	return isValidCrc_b;
}

static void doResetConfiguration(void) {
	resetConfigurationExt(PASS_ENGINE_PARAMETER_SUFFIX);
}

persisted_configuration_state_e flashState;

static persisted_configuration_state_e doReadConfiguration(flashaddr_t address) {

	intFlashRead(address, (char *) &persistentState, sizeof(persistentState));

	if (!isValidCrc(&persistentState)) {
		return CRC_FAILED;
	} else if (persistentState.version != FLASH_DATA_VERSION || persistentState.size != sizeof(persistentState)) {
		return INCOMPATIBLE_VERSION;
	} else {
		return PC_OK;
	}
}

/**
 * this method could and should be executed before we have any
 * connectivity so no console output here
 */
persisted_configuration_state_e readConfiguration() {
	efiAssert(CUSTOM_ERR_ASSERT, getCurrentRemainingStack() > EXPECTED_REMAINING_STACK, "read f", PC_ERROR);
	persisted_configuration_state_e result = doReadConfiguration(getFlashAddrFirstCopy());
	if (result != PC_OK) {

		result = doReadConfiguration(getFlashAddrSecondCopy());
	}

	if (result == CRC_FAILED) {
//		warning(CUSTOM_ERR_FLASH_CRC_FAILED, "flash CRC failed");
		resetConfigurationExt(PASS_ENGINE_PARAMETER_SUFFIX);
	} else if (result == INCOMPATIBLE_VERSION) {
		resetConfigurationExt(PASS_ENGINE_PARAMETER_SUFFIX);
	} else {
		/**
		 * At this point we know that CRC and version number is what we expect. Safe to assume it's a valid configuration.
		 */
		applyNonPersistentConfiguration(PASS_ENGINE_PARAMETER_SUFFIX);
	}
	// we can only change the state after the CRC check
	validateConfiguration(PASS_ENGINE_PARAMETER_SIGNATURE);
	return result;
}

void readFromFlash(void) {
	persisted_configuration_state_e result = readConfiguration();

	if (result == CRC_FAILED) {

	} else if (result == INCOMPATIBLE_VERSION) {

	} else {

	}
}


void initFlash() {
	


#if EFI_TUNER_STUDIO
	/**
	 * This would schedule write to flash once the engine is stopped
	 */

#endif

}

#endif /* EFI_INTERNAL_FLASH */
