/**
 * @file init_sensorss.cpp
 */

#include "init.h"

#include "sensor.h"



void initNewSensors(DECLARE_ENGINE_PARAMETER_SUFFIX) {
#if EFI_CAN_SUPPORT
	initCanSensors();
#endif

	initTps(PASS_ENGINE_PARAMETER_SIGNATURE);
	initOilPressure(PASS_ENGINE_PARAMETER_SIGNATURE);
	initNewThermistors(PASS_ENGINE_PARAMETER_SIGNATURE);

	// Init CLI functionality for sensors (mocking)

}

void reconfigureSensors(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	reconfigureTps(PASS_ENGINE_PARAMETER_SIGNATURE);
	reconfigureOilPressure(PASS_ENGINE_PARAMETER_SIGNATURE);
	reconfigureThermistors(PASS_ENGINE_PARAMETER_SIGNATURE);
}

