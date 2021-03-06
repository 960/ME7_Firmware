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
#include "fram.h"
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
	needToWriteConfiguration = false;
	writeToFlashNow();
}
template <typename EEtorage>
int writeToFram(flashaddr_t storageAddress, const EEtorage& data)
{
	return writeEeprom(storageAddress, reinterpret_cast<const uint8_t*>(&data), sizeof(data));

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

#if EFI_SPI_FRAM
	int result1 = writeEeprom(getFramAddrFirstCopy(), sizeof(persistentState), (uint8_t *) &persistentState);
	int result2 = writeEeprom(getFramAddrSecondCopy(), sizeof(persistentState), (uint8_t *) &persistentState);
#else
	int result1 = eraseAndFlashCopy(getFlashAddrFirstCopy(), persistentState);
	int result2 = eraseAndFlashCopy(getFlashAddrSecondCopy(), persistentState);
#endif
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
#if EFI_SPI_FRAM
static persisted_configuration_state_e doReadEEConfiguration(flashaddr_t address) {
	readEeprom(address, sizeof(persistentState),(uint8_t *) &persistentState);
	if (!isValidCrc(&persistentState)) {
		return CRC_FAILED;
	} else if (persistentState.version != FLASH_DATA_VERSION || persistentState.size != sizeof(persistentState)) {
		return INCOMPATIBLE_VERSION;
	} else {
		return PC_OK;
	}
}

#else
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
#endif
bool validateoffsets() {
	if (engineConfiguration->specs.cylindersCount > 16 || engineConfiguration->specs.cylindersCount < 1) {
	return false;
	}
	if (engineConfiguration->rpmLimit > 10000 || engineConfiguration->rpmLimit < 1000) {
		return false;
		}
	if (engine->triggerCentral.triggerShape.getSize() < 1 || engine->triggerCentral.triggerShape.getSize() > 130) {
		return false;
	}
	if (engineConfiguration->trigger.numTeeth < 1 || engineConfiguration->trigger.numTeeth > 130) {
		return false;
		} else {
	return true;
	}
}
/**
 * this method could and should be executed before we have any
 * connectivity so no console output here
 */
persisted_configuration_state_e readConfiguration() {
	//efiAssert(CUSTOM_ERR_ASSERT, getCurrentRemainingStack() > EXPECTED_REMAINING_STACK, "read f", PC_ERROR);
#if EFI_SPI_FRAM
	persisted_configuration_state_e result = doReadEEConfiguration(getFramAddrFirstCopy());
#else
	persisted_configuration_state_e result = doReadConfiguration(getFlashAddrFirstCopy());
#endif
	if (result != PC_OK) {
#if EFI_SPI_FRAM
	result = doReadEEConfiguration(getFramAddrSecondCopy());
#else
	result = doReadConfiguration(getFlashAddrSecondCopy());
#endif
	}
	if (result == CRC_FAILED) {
		resetConfigurationExt(PASS_ENGINE_PARAMETER_SUFFIX);
	} else if (result == INCOMPATIBLE_VERSION) {
		resetConfigurationExt(PASS_ENGINE_PARAMETER_SUFFIX);
	} else {
		int result2 = validateoffsets();
		if (result2 == false) {
		resetConfigurationExt(PASS_ENGINE_PARAMETER_SUFFIX);
		} else {
		applyNonPersistentConfiguration(PASS_ENGINE_PARAMETER_SUFFIX);
	}
	validateConfiguration(PASS_ENGINE_PARAMETER_SIGNATURE);
	}
	return result;
}

void readFromFlash(void) {
	persisted_configuration_state_e result = readConfiguration();
	if (result == CRC_FAILED) {
	} else if (result == INCOMPATIBLE_VERSION) {
	} else {
	}
}



#endif /* EFI_INTERNAL_FLASH */
