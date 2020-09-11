/**
 * @file init_sensorss.cpp
 */

#include "init.h"

#include "sensor.h"



void initNewSensors(DECLARE_ENGINE_PARAMETER_SUFFIX) {
#if EFI_CAN_SUPPORT
	initCanSensors();
#endif

	initTps(PASS_CONFIG_PARAMETER_SIGNATURE);
	initOilPressure(PASS_CONFIG_PARAMETER_SIGNATURE);
	initThermistors(PASS_CONFIG_PARAMETER_SIGNATURE);
	initLambda(PASS_ENGINE_PARAMETER_SIGNATURE);

	// Init CLI functionality for sensors (mocking)

}

void reconfigureSensors(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	reconfigureTps(PASS_CONFIG_PARAMETER_SIGNATURE);
	reconfigureOilPressure(PASS_CONFIG_PARAMETER_SIGNATURE);
	reconfigureThermistors(PASS_CONFIG_PARAMETER_SIGNATURE);
}

