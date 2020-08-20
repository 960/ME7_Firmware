/**
 * @file map.cpp
 *
 * See also map_averaging.cpp
 *
 * @author Andrey Belomutskiy, (c) 2012-2020
 */
#include "global.h"
#include "engine_configuration.h"
#include "engine_math.h"
#include "adc_inputs.h"
#include "interpolation.h"
#include "map.h"
#include "engine_controller.h"

#if EFI_PROD_CODE
#include "digital_input_icu.h"
#include "digital_input_exti.h"
#include "pin_repository.h"
#endif



EXTERN_ENGINE;



// See 'useFixedBaroCorrFromMap'
static float storedInitialBaroPressure = NAN;

/**
 * @brief	MAP value decoded for a 1.83 Honda sensor
 * -6.64kPa at zero volts
 * 182.78kPa at 5 volts
 *
 * about 3 volts at 100kPa
 *
 * @returns kPa value
 */





static FastInterpolation customMap;

float decodePressure(float voltage, air_pressure_sensor_config_s * mapConfig DECLARE_ENGINE_PARAMETER_SUFFIX) {
		return interpolateMsg("map", engineConfiguration->mapLowValueVoltage, mapConfig->lowValue,
				engineConfiguration->mapHighValueVoltage, mapConfig->highValue, voltage);

}

/**
 * This function adds an error if MAP sensor value is outside of expected range
 * @return unchanged mapKPa parameter
 */
float validateMap(float mapKPa DECLARE_ENGINE_PARAMETER_SUFFIX) {
	if (cisnan(mapKPa) || mapKPa < CONFIG(mapErrorDetectionTooLow) || mapKPa > CONFIG(mapErrorDetectionTooHigh)) {
		warning(OBD_Manifold_Absolute_Pressure_Circuit_Malfunction, "unexpected MAP value: %.2f", mapKPa);
		return 0;
	}
	return mapKPa;
}

/**
 * This function checks if Baro/MAP sensor value is inside of expected range
 * @return unchanged mapKPa parameter or NaN
 */
static float validateBaroMap(float mapKPa DECLARE_ENGINE_PARAMETER_SUFFIX) {
	const float atmoPressure = 100.0f;
	const float atmoPressureRange = 15.0f;	// 85..115
	if (cisnan(mapKPa) || absF(mapKPa - atmoPressure) > atmoPressureRange) {
		warning(OBD_Barometric_Press_Circ, "Invalid start-up baro pressure = %.2fkPa", mapKPa);
		return NAN;
	}
	return mapKPa;
}

/**
 * @brief	MAP value decoded according to current settings
 * @returns kPa value
 */
float getMapByVoltage(float voltage DECLARE_ENGINE_PARAMETER_SUFFIX) {
#if EFI_ENABLE_MOCK_ADC
	int mapChannel = engineConfiguration->map.sensor.hwChannel;
	if (engine->engineState.mockAdcState.hasMockAdc[mapChannel])
		voltage = adcToVolts(engine->engineState.mockAdcState.getMockAdcValue(mapChannel) * engineConfiguration->analogInputDividerCoefficient);
#endif

	// todo: migrate to mapDecoder once parameter listeners are ready
	air_pressure_sensor_config_s * apConfig = &engineConfiguration->map.sensor;
	return decodePressure(voltage, apConfig PASS_ENGINE_PARAMETER_SUFFIX);
}

/**
 * @return Manifold Absolute Pressure, in kPa
 */
float getRawMap(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	float voltage = getVoltageDivided("map", engineConfiguration->map.sensor.hwChannel PASS_ENGINE_PARAMETER_SUFFIX);
	return getMapByVoltage(voltage PASS_ENGINE_PARAMETER_SUFFIX);
}


float getMap(void) {

	return getRawMap();

}

/**
 * Returns true if a real Baro sensor is present.
 * Also if 'useFixedBaroCorrFromMap' option is enabled, and we have the initial pressure value stored and passed validation.
 */
bool hasBaroSensor(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	return engineConfiguration->baroSensor.hwChannel != EFI_ADC_NONE || !cisnan(storedInitialBaroPressure);
}

bool hasMapSensor(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	return engineConfiguration->map.sensor.hwChannel != EFI_ADC_NONE;
}

float getBaroPressure(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	// Override the real Baro sensor with the stored initial MAP value, if the option is set.
	if (CONFIG(useFixedBaroCorrFromMap))
		return storedInitialBaroPressure;
	float voltage = getVoltageDivided("baro", engineConfiguration->baroSensor.hwChannel PASS_ENGINE_PARAMETER_SUFFIX);
	return decodePressure(voltage, &engineConfiguration->baroSensor PASS_ENGINE_PARAMETER_SUFFIX);
}


static void applyConfiguration(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	air_pressure_sensor_config_s * apConfig = &engineConfiguration->map.sensor;
	customMap.init(0, apConfig->lowValue, 5, apConfig->highValue);

}





void initMapDecoder( DECLARE_ENGINE_PARAMETER_SUFFIX) {
	
	applyConfiguration(PASS_ENGINE_PARAMETER_SIGNATURE);
	//engine->configurationListeners.registerCallback(applyConfiguration);



	if (CONFIG(useFixedBaroCorrFromMap)) {
		// Read initial MAP sensor value and store it for Baro correction.
		storedInitialBaroPressure = getRawMap(PASS_ENGINE_PARAMETER_SIGNATURE);

		// validate if it's within a reasonable range (the engine should not be spinning etc.)
		storedInitialBaroPressure = validateBaroMap(storedInitialBaroPressure PASS_ENGINE_PARAMETER_SUFFIX);
		if (!cisnan(storedInitialBaroPressure)) {

		}
	}
	
}


