// this section was generated automatically by rusEfi tool ConfigDefinition.jar based on integration\rusefi_config.txt Tue Mar 10 23:17:54 CET 2020
// by class com.rusefi.output.CHeaderConsumer
// begin
#ifndef CONTROLLERS_GENERATED_ENGINE_CONFIGURATION_GENERATED_STRUCTURES_H
#define CONTROLLERS_GENERATED_ENGINE_CONFIGURATION_GENERATED_STRUCTURES_H
#include "rusefi_types.h"
// start of pid_s
struct pid_s {
	/**
	 * offset 0
	 */
	float pFactor;
	/**
	 * offset 4
	 */
	float iFactor;
	/**
	 * offset 8
	 */
	float dFactor;
	/**
	 * Linear addition to PID logic
	 * offset 12
	 */
	int16_t offset;
	/**
	 * PID dTime
	 * offset 14
	 */
	int16_t periodMs;
	/**
	 * Output min value
	 * offset 16
	 */
	int16_t minValue;
	/**
	 * Output max value
	 * offset 18
	 */
	int16_t maxValue;
	/** total size 20*/
};

typedef struct pid_s pid_s;

// start of cranking_parameters_s
struct cranking_parameters_s {
	/**
	 * Base duration of the fuel injection during cranking, this is modified by the multipliers for CLT, IAT, TPS ect, to give the final cranking pulse width.
	 * offset 0
	 */
	float baseFuel;
	/**
	 * This sets the RPM limit below which the ECU will use cranking fuel and ignition logic, typically this is around 350-450rpm. 
	 * set cranking_rpm X
	 * offset 4
	 */
	int16_t rpm;
	/**
	 * offset 6
	 */
	uint8_t unused120[2];
	/** total size 8*/
};

typedef struct cranking_parameters_s cranking_parameters_s;

// start of spi_pins
struct spi_pins {
	/**
	 * offset 0
	 */
	brain_pin_e mosiPin;
	/**
	 * offset 1
	 */
	brain_pin_e misoPin;
	/**
	 * offset 2
	 */
	brain_pin_e sckPin;
	/**
	 * offset 3
	 */
	uint8_t unused126;
	/** total size 4*/
};

typedef struct spi_pins spi_pins;

// start of air_pressure_sensor_config_s
struct air_pressure_sensor_config_s {
	/**
	 * kPa value at low volts
	 * offset 0
	 */
	float lowValue;
	/**
	 * kPa value at high volts
	 * offset 4
	 */
	float highValue;
	/**
	 * offset 8
	 */
	air_pressure_sensor_type_e type;
	/**
	 * offset 12
	 */
	adc_channel_e hwChannel;
	/**
	 * need 4 byte alignment
	 * offset 13
	 */
	uint8_t alignmentFill_at_13[3];
	/** total size 16*/
};

typedef struct air_pressure_sensor_config_s air_pressure_sensor_config_s;

/**
 * @brief MAP averaging configuration

*/
// start of MAP_sensor_config_s
struct MAP_sensor_config_s {
	/**
	 * offset 0
	 */
	air_pressure_sensor_config_s sensor;
	/** total size 16*/
};

typedef struct MAP_sensor_config_s MAP_sensor_config_s;

// start of MAP_sampling_config_s
struct MAP_sampling_config_s {
	/**
	 * offset 0
	 */
	float samplingAngleBins[MAP_ANGLE_SIZE];
	/**
	 * @brief MAP averaging sampling start angle, by RPM
	 * offset 32
	 */
	float samplingAngle[MAP_ANGLE_SIZE];
	/**
	 * offset 64
	 */
	float samplingWindowBins[MAP_WINDOW_SIZE];
	/**
	 * @brief MAP averaging angle duration, by RPM
	 * offset 96
	 */
	float samplingWindow[MAP_WINDOW_SIZE];
	/** total size 128*/
};

typedef struct MAP_sampling_config_s MAP_sampling_config_s;

/**
 * @brief Thermistor known values

*/
// start of thermistor_conf_s
struct thermistor_conf_s {
	/**
	 * these values are in Celcius
	 * offset 0
	 */
	float tempC_1;
	/**
	 * offset 4
	 */
	float tempC_2;
	/**
	 * offset 8
	 */
	float tempC_3;
	/**
	 * offset 12
	 */
	float resistance_1;
	/**
	 * offset 16
	 */
	float resistance_2;
	/**
	 * offset 20
	 */
	float resistance_3;
	/**
	 * Pull-up resistor value on your board
	 * offset 24
	 */
	float bias_resistor;
	/** total size 28*/
};

typedef struct thermistor_conf_s thermistor_conf_s;

/**
 * @brief Oil pressure sensor interpolation

*/
// start of oil_pressure_config_s
struct oil_pressure_config_s {
	/**
	 * offset 0
	 */
	adc_channel_e hwChannel;
	/**
	 * offset 1
	 */
	uint8_t align[3];
	/**
	 * offset 4
	 */
	float v1;
	/**
	 * offset 8
	 */
	float value1;
	/**
	 * offset 12
	 */
	float v2;
	/**
	 * offset 16
	 */
	float value2;
	/** total size 20*/
};

typedef struct oil_pressure_config_s oil_pressure_config_s;

/**
 * @brief Thermistor curve parameters

*/
// start of ThermistorConf
struct ThermistorConf {
	/**
	 * offset 0
	 */
	thermistor_conf_s config;
	/**
	 * offset 28
	 */
	adc_channel_e adcChannel;
	/**
	 * need 4 byte alignment
	 * offset 29
	 */
	uint8_t alignmentFill_at_29[3];
	/** total size 32*/
};

typedef struct ThermistorConf ThermistorConf;

// start of injector_s
struct injector_s {
	/**
	 * This is your injector flow at the fuel pressure used in the vehicle. cc/min, cubic centimetre per minute
	 * By the way, g/s = 0.125997881 * (lb/hr)
	 * g/s = 0.125997881 * (cc/min)/10.5
	 * g/s = 0.0119997981 * cc/min
	 * offset 0
	 */
	float flow;
	/**
	 * set_flat_injector_lag LAG
	 * set_injector_lag VOLTAGE LAG
	 * offset 4
	 */
	float battLagCorrBins[VBAT_INJECTOR_CURVE_SIZE];
	/**
	 * ms delay between injector open and close dead times
	 * offset 36
	 */
	float battLagCorr[VBAT_INJECTOR_CURVE_SIZE];
	/** total size 68*/
};

typedef struct injector_s injector_s;

// start of bi_quard_s
struct bi_quard_s {
	/**
	 * offset 0
	 */
	float a0;
	/**
	 * offset 4
	 */
	float a1;
	/**
	 * offset 8
	 */
	float a2;
	/**
	 * offset 12
	 */
	float b1;
	/**
	 * offset 16
	 */
	float b2;
	/** total size 20*/
};

typedef struct bi_quard_s bi_quard_s;

// start of specs_s
struct specs_s {
	/**
	 * Engine displacement, in litres
	 * see also cylindersCount
	 * offset 0
	 */
	float displacement;
	/**
	 * offset 4
	 */
	cylinders_count_t cylindersCount;
	/**
	 * offset 8
	 */
	firing_order_e firingOrder;
	/** total size 12*/
};

typedef struct specs_s specs_s;

/**
 * @brief Trigger wheel(s) configuration

*/
// start of trigger_config_s
struct trigger_config_s {
	/**
	 * set trigger_type X
	 * offset 0
	 */
	trigger_type_e type;
	/**
	 * offset 4
	 */
	int unused121;
	/**
	 * offset 8
	 */
	int customTotalToothCount;
	/**
	 * offset 12
	 */
	int customSkippedToothCount;
	/** total size 16*/
};

typedef struct trigger_config_s trigger_config_s;

// start of afr_sensor_s
struct afr_sensor_s {
	/**
	 * offset 0
	 */
	adc_channel_e hwChannel;
	/**
	 * offset 1
	 */
	uint8_t alignAf[3];
	/**
	 * offset 4
	 */
	float v1;
	/**
	 * offset 8
	 */
	float value1;
	/**
	 * offset 12
	 */
	float v2;
	/**
	 * offset 16
	 */
	float value2;
	/** total size 20*/
};

typedef struct afr_sensor_s afr_sensor_s;

// start of idle_hardware_s
struct idle_hardware_s {
	/**
	 * offset 0
	 */
	int solenoidFrequency;
	/**
	 * offset 4
	 */
	brain_pin_e solenoidPin;
	/**
	 * offset 5
	 */
	brain_pin_e stepperDirectionPin;
	/**
	 * offset 6
	 */
	brain_pin_e stepperStepPin;
	/**
	 * offset 7
	 */
	pin_output_mode_e solenoidPinMode;
	/**
	 * offset 8
	 */
	uint8_t unused171[4];
	/** total size 12*/
};

typedef struct idle_hardware_s idle_hardware_s;

// start of etb_io
struct etb_io {
	/**
	 * offset 0
	 */
	brain_pin_e directionPin1;
	/**
	 * offset 1
	 */
	brain_pin_e directionPin2;
	/**
	 * offset 2
	 */
	brain_pin_e controlPin1;
	/**
	 * offset 3
	 */
	pin_output_mode_e controlPinMode;
	/** total size 4*/
};

typedef struct etb_io etb_io;

// start of gp_pwm_s
struct gp_pwm_s {
	/**
	offset 0 bit 0 */
	bool conditionGpPwm4Tps : 1;
	/**
	offset 0 bit 1 */
	bool gpPwm4BelowOrAboveTps : 1;
	/**
	offset 0 bit 2 */
	bool conditionGpPwm4Map : 1;
	/**
	offset 0 bit 3 */
	bool gpPwm4BelowOrAboveMap : 1;
	/**
	offset 0 bit 4 */
	bool isGpPwm1Enabled : 1;
	/**
	offset 0 bit 5 */
	bool isGpPwm2Enabled : 1;
	/**
	offset 0 bit 6 */
	bool isGpPwm3Enabled : 1;
	/**
	offset 0 bit 7 */
	bool isGpPwm4Enabled : 1;
	/**
	offset 0 bit 8 */
	bool conditionGpPwm1Switch : 1;
	/**
	offset 0 bit 9 */
	bool conditionGpPwm1Rpm : 1;
	/**
	offset 0 bit 10 */
	bool gpPwm1BelowOrAboveRpm : 1;
	/**
	offset 0 bit 11 */
	bool conditionGpPwm1Clt : 1;
	/**
	offset 0 bit 12 */
	bool gpPwm1BelowOrAboveClt : 1;
	/**
	offset 0 bit 13 */
	bool conditionGpPwm1Tps : 1;
	/**
	offset 0 bit 14 */
	bool gpPwm1BelowOrAboveTps : 1;
	/**
	offset 0 bit 15 */
	bool conditionGpPwm1Map : 1;
	/**
	offset 0 bit 16 */
	bool gpPwm1BelowOrAboveMap : 1;
	/**
	offset 0 bit 17 */
	bool conditionGpPwm2Switch : 1;
	/**
	offset 0 bit 18 */
	bool conditionGpPwm2Rpm : 1;
	/**
	offset 0 bit 19 */
	bool gpPwm2BelowOrAboveRpm : 1;
	/**
	offset 0 bit 20 */
	bool conditionGpPwm2Clt : 1;
	/**
	offset 0 bit 21 */
	bool gpPwm2BelowOrAboveClt : 1;
	/**
	offset 0 bit 22 */
	bool conditionGpPwm2Tps : 1;
	/**
	offset 0 bit 23 */
	bool gpPwm2BelowOrAboveTps : 1;
	/**
	offset 0 bit 24 */
	bool conditionGpPwm2Map : 1;
	/**
	offset 0 bit 25 */
	bool gpPwm2BelowOrAboveMap : 1;
	/**
	offset 0 bit 26 */
	bool conditionGpPwm3Switch : 1;
	/**
	offset 0 bit 27 */
	bool conditionGpPwm3Rpm : 1;
	/**
	offset 0 bit 28 */
	bool gpPwm3BelowOrAboveRpm : 1;
	/**
	offset 0 bit 29 */
	bool conditionGpPwm3Clt : 1;
	/**
	offset 0 bit 30 */
	bool gpPwm3BelowOrAboveClt : 1;
	/**
	offset 0 bit 31 */
	bool conditionGpPwm3Tps : 1;
	/**
	 * offset 4
	 */
	int unused250;
	/**
	offset 8 bit 0 */
	bool gpPwm3BelowOrAboveTps : 1;
	/**
	offset 8 bit 1 */
	bool conditionGpPwm3Map : 1;
	/**
	offset 8 bit 2 */
	bool gpPwm3BelowOrAboveMap : 1;
	/**
	offset 8 bit 3 */
	bool conditionGpPwm4Switch : 1;
	/**
	offset 8 bit 4 */
	bool conditionGpPwm4Rpm : 1;
	/**
	offset 8 bit 5 */
	bool gpPwm4BelowOrAboveRpm : 1;
	/**
	offset 8 bit 6 */
	bool conditionGpPwm4Clt : 1;
	/**
	offset 8 bit 7 */
	bool gpPwm4BelowOrAboveClt : 1;
	/**
	offset 8 bit 8 */
	bool unusedBit_41_8 : 1;
	/**
	offset 8 bit 9 */
	bool unusedBit_41_9 : 1;
	/**
	offset 8 bit 10 */
	bool unusedBit_41_10 : 1;
	/**
	offset 8 bit 11 */
	bool unusedBit_41_11 : 1;
	/**
	offset 8 bit 12 */
	bool unusedBit_41_12 : 1;
	/**
	offset 8 bit 13 */
	bool unusedBit_41_13 : 1;
	/**
	offset 8 bit 14 */
	bool unusedBit_41_14 : 1;
	/**
	offset 8 bit 15 */
	bool unusedBit_41_15 : 1;
	/**
	offset 8 bit 16 */
	bool unusedBit_41_16 : 1;
	/**
	offset 8 bit 17 */
	bool unusedBit_41_17 : 1;
	/**
	offset 8 bit 18 */
	bool unusedBit_41_18 : 1;
	/**
	offset 8 bit 19 */
	bool unusedBit_41_19 : 1;
	/**
	offset 8 bit 20 */
	bool unusedBit_41_20 : 1;
	/**
	offset 8 bit 21 */
	bool unusedBit_41_21 : 1;
	/**
	offset 8 bit 22 */
	bool unusedBit_41_22 : 1;
	/**
	offset 8 bit 23 */
	bool unusedBit_41_23 : 1;
	/**
	offset 8 bit 24 */
	bool unusedBit_41_24 : 1;
	/**
	offset 8 bit 25 */
	bool unusedBit_41_25 : 1;
	/**
	offset 8 bit 26 */
	bool unusedBit_41_26 : 1;
	/**
	offset 8 bit 27 */
	bool unusedBit_41_27 : 1;
	/**
	offset 8 bit 28 */
	bool unusedBit_41_28 : 1;
	/**
	offset 8 bit 29 */
	bool unusedBit_41_29 : 1;
	/**
	offset 8 bit 30 */
	bool unusedBit_41_30 : 1;
	/**
	offset 8 bit 31 */
	bool unusedBit_41_31 : 1;
	/**
	 * offset 12
	 */
	float enableGpPwm1AtRpm;
	/**
	 * offset 16
	 */
	float enableGpPwm2AtRpm;
	/**
	 * offset 20
	 */
	float enableGpPwm3AtRpm;
	/**
	 * offset 24
	 */
	float enableGpPwm4AtRpm;
	/**
	 * offset 28
	 */
	float enableGpPwm1AtClt;
	/**
	 * offset 32
	 */
	float enableGpPwm2AtClt;
	/**
	 * offset 36
	 */
	float enableGpPwm3AtClt;
	/**
	 * offset 40
	 */
	float enableGpPwm4AtClt;
	/**
	 * offset 44
	 */
	float enableGpPwm1AtTps;
	/**
	 * offset 48
	 */
	float enableGpPwm2AtTps;
	/**
	 * offset 52
	 */
	float enableGpPwm3AtTps;
	/**
	 * offset 56
	 */
	float enableGpPwm4AtTps;
	/**
	 * offset 60
	 */
	float enableGpPwm1AtMap;
	/**
	 * offset 64
	 */
	float enableGpPwm2AtMap;
	/**
	 * offset 68
	 */
	float enableGpPwm3AtMap;
	/**
	 * offset 72
	 */
	float enableGpPwm4AtMap;
	/**
	 * offset 76
	 */
	int gpPwm1Frequency;
	/**
	 * offset 80
	 */
	int gpPwm2Frequency;
	/**
	 * offset 84
	 */
	int gpPwm3Frequency;
	/**
	 * offset 88
	 */
	int gpPwm4Frequency;
	/** total size 92*/
};

typedef struct gp_pwm_s gp_pwm_s;

// start of gp_pwmio_s
struct gp_pwmio_s {
	/**
	 * offset 0
	 */
	switch_input_pin_e gpPwm1InputPin;
	/**
	 * offset 1
	 */
	switch_input_pin_e gpPwm2InputPin;
	/**
	 * offset 2
	 */
	switch_input_pin_e gpPwm3InputPin;
	/**
	 * offset 3
	 */
	switch_input_pin_e gpPwm4InputPin;
	/**
	 * offset 4
	 */
	output_pin_e gpPwm1Pin;
	/**
	 * offset 5
	 */
	output_pin_e gpPwm2Pin;
	/**
	 * offset 6
	 */
	output_pin_e gpPwm3Pin;
	/**
	 * offset 7
	 */
	output_pin_e gpPwm4Pin;
	/**
	 * offset 8
	 */
	pin_output_mode_e gpPwm1PinMode;
	/**
	 * offset 9
	 */
	pin_output_mode_e gpPwm2PinMode;
	/**
	 * offset 10
	 */
	pin_output_mode_e gpPwm3PinMode;
	/**
	 * offset 11
	 */
	pin_output_mode_e gpPwm4PinMode;
	/** total size 12*/
};

typedef struct gp_pwmio_s gp_pwmio_s;

// start of engine_configuration_s
struct engine_configuration_s {
	/**
	 * http://rusefi.com/wiki/index.php?title=Manual:Engine_Type
	 * set engine_type X
	 * offset 0
	 */
	engine_type_e engineType;
	/**
	 * This setting controls which fuel quantity control algorithm is used.
	 * See also useTPSAdvanceTable
	 * set algorithm X
	 * offset 4
	 */
	engine_load_mode_e fuelAlgorithm;
	/**
	 * offset 8
	 */
	specs_s specs;
	/**
	 * offset 20
	 */
	trigger_config_s trigger;
	/**
	 * offset 36
	 */
	cranking_parameters_s cranking;
	/**
	 * This is the injection strategy during engine start. See Fuel/Injection settings for more detail. It is suggested to use "Simultaneous".
	 * offset 44
	 */
	injection_mode_e crankingInjectionMode;
	/**
	 * This is where the fuel injection type is defined: "Simultaneous" means all injectors will fire together at once. "Sequential" fires the injectors on a per cylinder basis, which requires individually wired injectors. "Batched" will fire the injectors in groups. If your injectors are individually wired you will also need to enable "Two wire batch emulation". 
	 * set injection_mode X
	 * See also twoWireBatchInjection
	 * offset 48
	 */
	injection_mode_e injectionMode;
	/**
	 * offset 52
	 */
	maf_sensor_type_e mafSensorType;
	/**
	 * offset 56
	 */
	idle_hardware_s idle;
	/**
	 * See also idleRpmPid
	 * offset 68
	 */
	idle_mode_e idleMode;
	/**
	 * offset 72
	 */
	boostType_e boostType;
	/**
	 * offset 76
	 */
	launchActivationMode_e launchActivationMode;
	/**
	 * offset 80
	 */
	antiLagActivationMode_e antiLagActivationMode;
	/**
	 * AFR, WBO, EGO - whatever you like to call it
	 * offset 84
	 */
	ego_sensor_e afr_type;
	/**
	 * set vvt_mode X
	 * offset 88
	 */
	vvt_mode_e vvtMode;
	/**
	 * offset 92
	 */
	afr_sensor_s afr;
	/**
	 * todo: finish implementation #332
	 * offset 112
	 */
	ThermistorConf auxTempSensor1;
	/**
	 * todo: finish implementation #332
	 * offset 144
	 */
	ThermistorConf auxTempSensor2;
	/**
	 * set can_mode X
	 * offset 176
	 */
	can_nbc_e canNbcType;
	/**
	 * 'Some triggers could be mounted differently. Most well-known triggers imply specific sensor setup. 4 stroke with symmetrical crank' is a pretty special case for example on Miata NB2
	 * See engineCycle
	 * set operation_mode X
	 * offset 180
	 */
	operation_mode_e ambiguousOperationMode;
	/**
	 * Dynamic uses the timing map to decide the ignition timing, Static timing fixes the timing to the value set below (only use for checking static timing).
	 * offset 184
	 */
	timing_mode_e timingMode;
	/**
	 * "One Coil" is for use on distributed ignition system. "Individual Coils" is to be used when you have one coil per cylinder (COP or similar). "Wasted" means one coil is driving two spark plugs in two cylinders, with one of the sparks not doing anything since it's happening on the exhaust cycle
	 * set ignition_mode X
	 * offset 188
	 */
	ignition_mode_e ignitionMode;
	/**
	 * offset 192
	 */
	tChargeMode_e tChargeMode;
	/**
	 * See http://rusefi.com/s/debugmode
	 * 
	 * set debug_mode X
	 * offset 196
	 */
	debug_mode_e debugMode;
	/**
	 * offset 200
	 */
	log_format_e logFormat;
	/**
	 * offset 204
	 */
	uint8_t freeSpaceForConstants[20];
	/**
	 * Electronic throttle pedal position input
	 * First channel
	 * See also tps1_1AdcChannel
	 * offset 224
	 */
	adc_channel_e throttlePedalPositionAdcChannel;
	/**
	 * First throttle body, first sensor. See also pedalPositionAdcChannel
	 * offset 225
	 */
	adc_channel_e tps1_1AdcChannel;
	/**
	 * First throttle body, second sensor.
	 * offset 226
	 */
	adc_channel_e tps1_2AdcChannel;
	/**
	 * Second throttle body, second sensor.
	 * offset 227
	 */
	adc_channel_e tps2_2AdcChannel;
	/**
	 * Electronic throttle pedal position input
	 * Second channel
	 * See also tps1_1AdcChannel
	 * offset 228
	 */
	adc_channel_e throttlePedalPositionSecondAdcChannel;
	/**
	 * This is the processor input pin that the battery voltage circuit is connected to, if you are unsure of what pin to use, check the schematic that corresponds to your PCB.
	 * offset 229
	 */
	adc_channel_e vbattAdcChannel;
	/**
	 * This is the processor pin that your fuel level sensor in connected to. This is a non standard input so will need to be user defined.
	 * offset 230
	 */
	adc_channel_e fuelLevelSensor;
	/**
	 * Second throttle body position sensor, single channel so far
	 * offset 231
	 */
	adc_channel_e tps2_1AdcChannel;
	/**
	 * See hasMafSensor
	 * offset 232
	 */
	adc_channel_e mafAdcChannel;
	/**
	 * offset 233
	 */
	adc_channel_e vRefAdcChannel;
	/**
	 * lambda input
	 * offset 234
	 */
	adc_channel_e cj125ua;
	/**
	 * heater input
	 * offset 235
	 */
	adc_channel_e cj125ur;
	/**
	 * offset 236
	 */
	adc_channel_e externalKnockSenseAdc;
	/**
	 * A/C button input handled as analogue input
	 * offset 237
	 */
	adc_channel_e acSwitchAdc;
	/**
	 * Useful in Research&Development phase
	 * offset 238
	 */
	adc_channel_e auxFastSensor1_adcChannel;
	/**
	 * todo: rename to fsioAnalogInputs
	 * offset 239
	 */
	adc_channel_e fsioAdc[FSIO_ANALOG_INPUT_COUNT];
	/**
	 * offset 243
	 */
	uint8_t freeSpaceForadc_channel_e[25];
	/**
	 * offset 268
	 */
	brain_pin_e cj125CsPin;
	/**
	 * offset 269
	 */
	brain_pin_e canTxPin;
	/**
	 * offset 270
	 */
	brain_pin_e canRxPin;
	/**
	 * offset 271
	 */
	brain_pin_e spi1mosiPin;
	/**
	 * offset 272
	 */
	brain_pin_e spi1misoPin;
	/**
	 * offset 273
	 */
	brain_pin_e spi1sckPin;
	/**
	 * offset 274
	 */
	brain_pin_e spi2mosiPin;
	/**
	 * offset 275
	 */
	brain_pin_e spi2misoPin;
	/**
	 * offset 276
	 */
	brain_pin_e spi2sckPin;
	/**
	 * offset 277
	 */
	brain_pin_e spi3mosiPin;
	/**
	 * offset 278
	 */
	brain_pin_e spi3misoPin;
	/**
	 * offset 279
	 */
	brain_pin_e spi3sckPin;
	/**
	 * offset 280
	 */
	brain_pin_e binarySerialTxPin;
	/**
	 * offset 281
	 */
	brain_pin_e binarySerialRxPin;
	/**
	 *  todo: finish pin migration from hard-coded to configurable?
	 * offset 282
	 */
	brain_pin_e consoleSerialTxPin;
	/**
	 * todo: finish pin migration from hard-coded to configurable?
	 * offset 283
	 */
	brain_pin_e consoleSerialRxPin;
	/**
	 * This implementation makes a pulse every time one of the coils is charged, using coil dwell for pulse width. See also tachOutputPin
	 * offset 284
	 */
	brain_pin_e dizzySparkOutputPin;
	/**
	 * offset 285
	 */
	brain_pin_e cj125ModePin;
	/**
	 * offset 286
	 */
	brain_pin_e wboHeaterPin;
	/**
	 * offset 287
	 */
	brain_pin_e stepperEnablePin;
	/**
	 * offset 288
	 */
	brain_pin_e tle8888_cs;
	/**
	 * offset 289
	 */
	brain_pin_e debugTriggerSync;
	/**
	 * offset 290
	 */
	brain_pin_e debugSetTimer;
	/**
	 * offset 291
	 */
	brain_pin_e debugTimerCallback;
	/**
	 * offset 292
	 */
	brain_pin_e debugMapAveraging;
	/**
	 * offset 293
	 */
	brain_pin_e starterRelayPin;
	/**
	 * Some Subaru and some Mazda use double-solenoid idle air valve
	 * offset 294
	 */
	brain_pin_e secondSolenoidPin;
	/**
	 * This pin is used for debugging - snap a logic analyzer on it and see if it's ever high
	 * offset 295
	 */
	brain_pin_e triggerErrorPin;
	/**
	 * Each rusEfi piece can provide synthetic trigger signal for external ECU. Sometimes these wires are routed back into trigger inputs of the same rusEfi board.
	 * See also directSelfStimulation which is different.
	 * offset 296
	 */
	brain_pin_e triggerSimulatorPins[TRIGGER_SIMULATOR_PIN_COUNT];
	/**
	 * offset 298
	 */
	uint8_t unused251[2];
	/**
	 * offset 300
	 */
	uint8_t freeSpaceForBrainpins[20];
	/**
	 * offset 320
	 */
	output_pin_e fuelPumpPin;
	/**
	 * Narrow band o2 heater, not used for CJ125. See wboHeaterPin
	 * offset 321
	 */
	output_pin_e o2heaterPin;
	/**
	 * offset 322
	 */
	output_pin_e fanPin;
	/**
	 * Check engine light, also malfunction indicator light. Always blinks once on boot.
	 * offset 323
	 */
	output_pin_e malfunctionIndicatorPin;
	/**
	 * offset 324
	 */
	output_pin_e alternatorControlPin;
	/**
	 * This implementation produces one pulse per engine cycle. See also dizzySparkOutputPin.
	 * offset 325
	 */
	output_pin_e tachOutputPin;
	/**
	 * offset 326
	 */
	output_pin_e mainRelayPin;
	/**
	 * offset 327
	 */
	output_pin_e boostControlPin;
	/**
	 * offset 328
	 */
	output_pin_e vvtControlPin;
	/**
	 * offset 329
	 */
	output_pin_e acRelayPin;
	/**
	 * offset 330
	 */
	uint8_t unused174[2];
	/**
	 * offset 332
	 */
	output_pin_e injectionPins[INJECTION_PIN_COUNT];
	/**
	 * offset 344
	 */
	output_pin_e ignitionPins[IGNITION_PIN_COUNT];
	/**
	 * offset 356
	 */
	uint8_t freeSpaceForOutputpins[20];
	/**
	 * offset 376
	 */
	pin_output_mode_e tle8888_csPinMode;
	/**
	 * offset 377
	 */
	pin_output_mode_e fuelPumpPinMode;
	/**
	 * offset 378
	 */
	pin_output_mode_e injectionPinMode;
	/**
	 * offset 379
	 */
	pin_output_mode_e ignitionPinMode;
	/**
	 * offset 380
	 */
	pin_output_mode_e electronicThrottlePin1Mode;
	/**
	 * offset 381
	 */
	pin_output_mode_e o2heaterPinModeTodO;
	/**
	 * offset 382
	 */
	pin_output_mode_e alternatorControlPinMode;
	/**
	 * offset 383
	 */
	pin_output_mode_e malfunctionIndicatorPinMode;
	/**
	 * offset 384
	 */
	pin_output_mode_e fanPinMode;
	/**
	 * offset 385
	 */
	pin_output_mode_e tachOutputPinMode;
	/**
	 * offset 386
	 */
	pin_output_mode_e mainRelayPinMode;
	/**
	 * offset 387
	 */
	pin_output_mode_e starterRelayPinMode;
	/**
	 * offset 388
	 */
	pin_output_mode_e cj125CsPinMode;
	/**
	 * offset 389
	 */
	pin_output_mode_e dizzySparkOutputPinMode;
	/**
	 * offset 390
	 */
	pin_output_mode_e cj125ModePinMode;
	/**
	 * offset 391
	 */
	pin_output_mode_e stepperEnablePinMode;
	/**
	 * offset 392
	 */
	pin_output_mode_e boostControlPinMode;
	/**
	 * offset 393
	 */
	pin_output_mode_e vvtControlPinMode;
	/**
	 * offset 394
	 */
	pin_output_mode_e triggerErrorPinMode;
	/**
	 * offset 395
	 */
	pin_output_mode_e stepperDirectionPinMode;
	/**
	 * offset 396
	 */
	pin_output_mode_e acRelayPinMode;
	/**
	 * offset 397
	 */
	pin_output_mode_e triggerSimulatorPinModes[TRIGGER_SIMULATOR_PIN_COUNT];
	/**
	 * offset 399
	 */
	uint8_t unused129;
	/**
	 * offset 400
	 */
	uint8_t freeSpaceForOutputpinMode[20];
	/**
	 * offset 420
	 */
	brain_input_pin_e triggerInputPins[TRIGGER_INPUT_PIN_COUNT];
	/**
	 * offset 423
	 */
	brain_input_pin_e frequencyReportingMapInputPin;
	/**
	 * Camshaft input could be used either just for engine phase detection if your trigger shape does not include cam sensor as 'primary' channel, or it could be used for Variable Valve timing on one of the camshafts.
	 * TODO #660
	 * offset 424
	 */
	brain_input_pin_e camInputs[CAM_INPUTS_COUNT];
	/**
	 * offset 428
	 */
	brain_input_pin_e vehicleSpeedSensorInputPin;
	/**
	 * offset 429
	 */
	uint8_t unused137[3];
	/**
	 * offset 432
	 */
	uint8_t freeSpaceForInputpins[8];
	/**
	 * offset 440
	 */
	pin_input_mode_e clutchDownPinMode;
	/**
	 * offset 441
	 */
	pin_input_mode_e throttlePedalUpPinMode;
	/**
	 * offset 442
	 */
	pin_input_mode_e clutchUpPinMode;
	/**
	 * offset 443
	 */
	pin_input_mode_e brakePedalPinMode;
	/**
	 * offset 444
	 */
	uint8_t freeSpaceForInputpinMode[8];
	/**
	 * Some vehicles have a switch to indicate that clutch pedal is all the way up
	 * offset 452
	 */
	switch_input_pin_e clutchUpPin;
	/**
	 * some cars have a switch to indicate that clutch pedal is all the way down
	 * offset 453
	 */
	switch_input_pin_e clutchDownPin;
	/**
	 * Throttle Pedal not pressed switch
	 * offset 454
	 */
	switch_input_pin_e unused82;
	/**
	 * offset 455
	 */
	switch_input_pin_e launchActivatePin;
	/**
	 * offset 456
	 */
	switch_input_pin_e antiLagActivatePin;
	/**
	 * Brake pedal switch
	 * offset 457
	 */
	switch_input_pin_e brakePedalPin;
	/**
	 * offset 458
	 */
	uint8_t unused130[2];
	/**
	 * offset 460
	 */
	uint8_t freeSpaceForSwitchInputpins[20];
	/**
	 * offset 480
	 */
	pin_mode_e spi1SckMode;
	/**
	 * offset 481
	 */
	pin_mode_e spi1MosiMode;
	/**
	 * offset 482
	 */
	pin_mode_e spi1MisoMode;
	/**
	 * offset 483
	 */
	pin_mode_e spi2SckMode;
	/**
	 * offset 484
	 */
	pin_mode_e spi2MosiMode;
	/**
	 * offset 485
	 */
	pin_mode_e spi2MisoMode;
	/**
	 * offset 486
	 */
	pin_mode_e spi3SckMode;
	/**
	 * offset 487
	 */
	pin_mode_e spi3MosiMode;
	/**
	 * offset 488
	 */
	pin_mode_e spi3MisoMode;
	/**
	 * offset 489
	 */
	uint8_t unused131[3];
	/**
	 * offset 492
	 */
	uint8_t freeSpaceForPinmode[16];
	/**
	 * offset 508
	 */
	spi_device_e tle8888spiDevice;
	/**
	 * offset 509
	 */
	spi_device_e cj125SpiDevice;
	/**
	 * offset 510
	 */
	uint8_t unused141[2];
	/**
	 * offset 512
	 */
	uint8_t freeSpaceForSpiDevice[40];
	/**
	 * offset 552
	 */
	can_device_mode_e canDeviceMode;
	/**
	 * offset 556
	 */
	uart_device_e consoleUartDevice;
	/**
	 * offset 557
	 */
	uint8_t unused139[3];
	/**
	 * offset 560
	 */
	etb_io etbIo[ETB_COUNT];
	/**
	 * offset 568
	 */
	gp_pwmio_s gppwmIo;
	/**
	 * offset 580
	 */
	uint8_t freeSpaceForPins[60];
	/**
	 * offset 640
	 */
	vvtLoadAxis_e vvtLoadAxis;
	/**
	 * offset 644
	 */
	gpPwm1Load_e gpPwm1Load;
	/**
	 * offset 648
	 */
	gpPwm2Load_e gpPwm2Load;
	/**
	 * offset 652
	 */
	gpPwm3Load_e gpPwm3Load;
	/**
	 * offset 656
	 */
	gpPwm4Load_e gpPwm4Load;
	/**
	 * Should trigger emulator push data right into trigger handling logic, eliminating the need for physical jumper wires?
	 * See also triggerSimulatorPins
	 * PS: Funny name, right? :)
	offset 660 bit 0 */
	bool directSelfStimulation : 1;
	/**
	offset 660 bit 1 */
	bool useBiQuadAnalogFiltering : 1;
	/**
	offset 660 bit 2 */
	bool cj125isUaDivided : 1;
	/**
	offset 660 bit 3 */
	bool cj125isLsu49 : 1;
	/**
	offset 660 bit 4 */
	bool etb_use_two_wires : 1;
	/**
	offset 660 bit 5 */
	bool isDoubleSolenoidIdle : 1;
	/**
	offset 660 bit 6 */
	bool showSdCardWarning : 1;
	/**
	 * looks like 3v range should be enough, divider not needed
	offset 660 bit 7 */
	bool cj125isUrDivided : 1;
	/**
	offset 660 bit 8 */
	bool useTLE8888_hall_mode : 1;
	/**
	offset 660 bit 9 */
	bool useTLE8888_cranking_hack : 1;
	/**
	offset 660 bit 10 */
	bool useInstantRpmForIdle : 1;
	/**
	 * If your fuel regulator does not have vacuum line
	offset 660 bit 11 */
	bool absoluteFuelPressure : 1;
	/**
	offset 660 bit 12 */
	bool launchControlEnabled : 1;
	/**
	offset 660 bit 13 */
	bool rollingLaunchEnabled : 1;
	/**
	offset 660 bit 14 */
	bool antiLagEnabled : 1;
	/**
	offset 660 bit 15 */
	bool useRunningMathForCranking : 1;
	/**
	offset 660 bit 16 */
	bool isCJ125Enabled : 1;
	/**
	 * Use rise or fall signal front
	offset 660 bit 17 */
	bool vvtCamSensorUseRise : 1;
	/**
	 * Useful for individual intakes
	offset 660 bit 18 */
	bool measureMapOnlyInOneCylinder : 1;
	/**
	offset 660 bit 19 */
	bool stepperForceParkingEveryRestart : 1;
	/**
	 * Smarter cranking logic.
	offset 660 bit 20 */
	bool isFasterEngineSpinUpEnabled : 1;
	/**
	 * This setting disables fuel injection while the engine is in overrun, this is useful as a fuel saving measure and to prevent back firing.
	offset 660 bit 21 */
	bool coastingFuelCutEnabled : 1;
	/**
	 * This setting allows the ECU to open the IAC during overrun conditions to help reduce engine breaking, this can be helpful for large engines in light weight cars.
	offset 660 bit 22 */
	bool useIacTableForCoasting : 1;
	/**
	offset 660 bit 23 */
	bool useNoiselessTriggerDecoder : 1;
	/**
	offset 660 bit 24 */
	bool useIdleTimingPidControl : 1;
	/**
	offset 660 bit 25 */
	bool useTPSBasedVeTable : 1;
	/**
	offset 660 bit 26 */
	bool pauseEtbControl : 1;
	/**
	offset 660 bit 27 */
	bool alignEngineSnifferAtTDC : 1;
	/**
	 * This setting allows the ETB to act as the idle air control valve and move to regulate the airflow at idle.
	offset 660 bit 28 */
	bool useETBforIdleControl : 1;
	/**
	offset 660 bit 29 */
	bool idleIncrementalPidCic : 1;
	/**
	offset 660 bit 30 */
	bool enableAemXSeries : 1;
	/**
	 *  +This will cause the alternator to be operated in a basic on or off mode, this is the simplest alternator control.
	offset 660 bit 31 */
	bool onOffAlternatorLogic : 1;
	/**
	 * offset 664
	 */
	uint8_t unused243[4];
	/**
	offset 668 bit 0 */
	bool useSerialPort : 1;
	/**
	offset 668 bit 1 */
	bool is_enabled_spi_1 : 1;
	/**
	offset 668 bit 2 */
	bool is_enabled_spi_2 : 1;
	/**
	offset 668 bit 3 */
	bool is_enabled_spi_3 : 1;
	/**
	offset 668 bit 4 */
	bool is_enabled_spi_4 : 1;
	/**
	offset 668 bit 5 */
	bool isFastAdcEnabled : 1;
	/**
	offset 668 bit 6 */
	bool isEngineControlEnabled : 1;
	/**
	offset 668 bit 7 */
	bool canReadEnabled : 1;
	/**
	offset 668 bit 8 */
	bool canWriteEnabled : 1;
	/**
	 * This setting should only be used if you have a stepper motor idle valve and a stepper motor driver installed.
	offset 668 bit 9 */
	bool useStepperIdle : 1;
	/**
	offset 668 bit 10 */
	bool useTpicAdvancedMode : 1;
	/**
	 * This option could be used if your second trigger channel is broken
	offset 668 bit 11 */
	bool useOnlyFirstChannel : 1;
	/**
	offset 668 bit 12 */
	bool useLinearCltSensor : 1;
	/**
	offset 668 bit 13 */
	bool useLinearIatSensor : 1;
	/**
	offset 668 bit 14 */
	bool hasFrequencyReportingMapSensor : 1;
	/**
	offset 668 bit 15 */
	bool vvtType : 1;
	/**
	 * Enable fuel injection - This is default off for new projects as a safety feature, set to "true" to enable fuel injection and further injector settings.
	offset 668 bit 16 */
	bool isInjectionEnabled : 1;
	/**
	 * Enable ignition - This is default off for new projects as a safety feature, set to "true" to enable ignition and further ignition settings.
	offset 668 bit 17 */
	bool isIgnitionEnabled : 1;
	/**
	 * When enabled if TPS is held above 95% no fuel is injected while cranking to clear excess fuel from the cylinders.
	offset 668 bit 18 */
	bool isCylinderCleanupEnabled : 1;
	/**
	offset 668 bit 19 */
	bool secondTriggerChannelEnabled : 1;
	/**
	offset 668 bit 20 */
	bool vvtDisplayInverted : 1;
	/**
	 * Enables lambda sensor closed loop feedback for fuelling.
	offset 668 bit 21 */
	bool fuelClosedLoopCorrectionEnabled : 1;
	/**
	 * If set to true, will use the specified duration for cranking dwell. If set to false, will use the specified dwell angle. Unless you have a really good reason to, leave this set to true to use duration mode.
	offset 668 bit 22 */
	bool useConstantDwellDuringCranking : 1;
	/**
	 * This options enables data for 'engine sniffer' tab in console, which comes at some CPU price
	offset 668 bit 23 */
	bool isEngineChartEnabled : 1;
	/**
	 * Sometimes we have a performance issue while printing error
	offset 668 bit 24 */
	bool silentTriggerError : 1;
	/**
	offset 668 bit 25 */
	bool tachPulseDurationAsDutyCycle : 1;
	/**
	 * This enables smart alternator control and activates the extra alternator settings.
	offset 668 bit 26 */
	bool isAlternatorControlEnabled : 1;
	/**
	 * This setting flips the signal from the primary engine speed sensor.
	offset 668 bit 27 */
	bool invertPrimaryTriggerSignal : 1;
	/**
	 * This setting flips the signal from the secondary engine speed sensor.
	offset 668 bit 28 */
	bool invertSecondaryTriggerSignal : 1;
	/**
	offset 668 bit 29 */
	bool isBoostControlEnabled : 1;
	/**
	offset 668 bit 30 */
	bool isVvtControlEnabled : 1;
	/**
	offset 668 bit 31 */
	bool setEtbErrorRpm : 1;
	/**
	 * offset 672
	 */
	uint8_t unused200[4];
	/**
	offset 676 bit 0 */
	bool enableLaunchBoost : 1;
	/**
	offset 676 bit 1 */
	bool isMapAveragingEnabled : 1;
	/**
	 * This setting overrides the normal multiplication values that have been set for the idle air control valve during cranking. If this setting is enabled the "IAC multiplier" table in the Cranking settings tab needs to be adjusted appropriately or potentially no IAC opening will occur.
	offset 676 bit 2 */
	bool overrideCrankingIacSetting : 1;
	/**
	 * This activates a separate ignition timing table for idle conditions, this can help idle stability by using ignition retard and advance either side of the desired idle speed. Extra retard at low idle speeds will prevent stalling and extra advance at high idle speeds can help reduce engine power and slow the idle speed.
	offset 676 bit 3 */
	bool useSeparateAdvanceForIdle : 1;
	/**
	offset 676 bit 4 */
	bool isTunerStudioEnabled : 1;
	/**
	offset 676 bit 5 */
	bool isWaveAnalyzerEnabled : 1;
	/**
	 * This activates a separate fuel table for Idle, this allows fine tuning of the idle fuelling.
	offset 676 bit 6 */
	bool useSeparateVeForIdle : 1;
	/**
	 * Usually if we have no trigger events that means engine is stopped
	 * Unless we are troubleshooting and spinning the engine by hand - this case a longer
	 * delay is needed
	offset 676 bit 7 */
	bool isManualSpinningMode : 1;
	/**
	 * This is needed if your coils are individually wired and you wish to use batch injection.
	 * enable two_wire_batch_injection
	offset 676 bit 8 */
	bool twoWireBatchInjection : 1;
	/**
	 * VR sensors are only precise on rising front
	 * enable trigger_only_front
	offset 676 bit 9 */
	bool useOnlyRisingEdgeForTrigger : 1;
	/**
	 * This is needed if your coils are individually wired (COP) and you wish to use batch ignition (wasted spark).
	offset 676 bit 10 */
	bool twoWireBatchIgnition : 1;
	/**
	offset 676 bit 11 */
	bool useFixedBaroCorrFromMap : 1;
	/**
	 * This activates a separate advance table for cranking conditions, this allows cranking advance to be RPM dependant.
	offset 676 bit 12 */
	bool useSeparateAdvanceForCranking : 1;
	/**
	 * This enables the various ignition corrections during cranking (IAT, CLT, FSIO and PID idle).
	offset 676 bit 13 */
	bool useAdvanceCorrectionsForCranking : 1;
	/**
	 * This flag allows to use TPS for ignition lookup while in Speed Density Fuel Mode
	offset 676 bit 14 */
	bool useTPSAdvanceTable : 1;
	/**
	offset 676 bit 15 */
	bool etbCalibrationOnStart : 1;
	/**
	 * This flag allows to use a special 'PID Multiplier' table (0.0-1.0) to compensate for nonlinear nature of IAC-RPM controller
	offset 676 bit 16 */
	bool useIacPidMultTable : 1;
	/**
	offset 676 bit 17 */
	bool launchDisableBySpeed : 1;
	/**
	 * Interpolates the Ignition Retard from 0 to 100% within the RPM Range
	offset 676 bit 18 */
	bool launchSmoothRetard : 1;
	/**
	offset 676 bit 19 */
	bool enableLaunchRetard : 1;
	/**
	offset 676 bit 20 */
	bool cutFuelOnHardLimit : 1;
	/**
	offset 676 bit 21 */
	bool cutSparkOnHardLimit : 1;
	/**
	offset 676 bit 22 */
	bool launchFuelCutEnable : 1;
	/**
	 * This is the Cut Mode normally used
	offset 676 bit 23 */
	bool launchSparkCutEnable : 1;
	/**
	offset 676 bit 24 */
	bool multisparkEnable : 1;
	/**
	offset 676 bit 25 */
	bool unused44 : 1;
	/**
	offset 676 bit 26 */
	bool unused45 : 1;
	/**
	offset 676 bit 27 */
	bool unused5 : 1;
	/**
	offset 676 bit 28 */
	bool unused205 : 1;
	/**
	offset 676 bit 29 */
	bool unused202 : 1;
	/**
	offset 676 bit 30 */
	bool unused203 : 1;
	/**
	offset 676 bit 31 */
	bool unused204 : 1;
	/**
	 * offset 680
	 */
	uint8_t freeSpaceForBits[40];
	/**
	 * offset 720
	 */
	pid_s boostPid;
	/**
	 * offset 740
	 */
	pid_s vvtPid;
	/**
	 * offset 760
	 */
	pid_s alternatorControl;
	/**
	 * offset 780
	 */
	pid_s etb;
	/**
	 * See cltIdleRpmBins
	 * offset 800
	 */
	pid_s idleRpmPid;
	/**
	 * offset 820
	 */
	pid_s fuelClosedLoopPid;
	/**
	 * See useIdleTimingPidControl
	 * offset 840
	 */
	pid_s idleTimingPid;
	/**
	 * offset 860
	 */
	pid_s idleRpmPid2;
	/**
	 * offset 880
	 */
	uint8_t freeSpaceForPid[80];
	/**
	 * Engine sniffer would be disabled above this rpm
	 * set engineSnifferRpmThreshold X
	 * offset 960
	 */
	int engineSnifferRpmThreshold;
	/**
	 * A secondary Rev limit engaged by the driver to help launch the vehicle faster
	 * offset 964
	 */
	int launchRpm;
	/**
	 * Disable sensor sniffer above this rpm
	 * offset 968
	 */
	int sensorSnifferRpmThreshold;
	/**
	 * set rpm_hard_limit X
	 * offset 972
	 */
	int rpmHardLimit;
	/**
	 * CANbus thread period, ms
	 * offset 976
	 */
	int canSleepPeriodMs;
	/**
	 * offset 980
	 */
	int byFirmwareVersion;
	/**
	 * offset 984
	 */
	int overrideCrankingIgnition;
	/**
	 * offset 988
	 */
	int sensorChartFrequency;
	/**
	 * Same RPM is used for two ways of producing simulated RPM. See also triggerSimulatorPins (with wires)
	 * See also directSelfStimulation (no wires, bypassing input hardware)
	 * rpm X
	 * offset 992
	 */
	int triggerSimulatorFrequency;
	/**
	 * offset 996
	 */
	int idleThreadPeriodMs;
	/**
	 * offset 1000
	 */
	int consoleLoopPeriodMs;
	/**
	 * offset 1004
	 */
	int generalPeriodicThreadPeriodMs;
	/**
	 * offset 1008
	 */
	int unrealisticRpmThreashold;
	/**
	 * offset 1012
	 */
	int launchTpsTreshold;
	/**
	 * Disabled above this speed
	 * offset 1016
	 */
	int launchSpeedTreshold;
	/**
	 * Disabled below this rpm
	 * offset 1020
	 */
	int antiLagRpmTreshold;
	/**
	 * Range from Launch Rpm for Timing Retard to activate
	 * offset 1024
	 */
	int launchTimingRpmRange;
	/**
	 * Extra Fuel Added
	 * offset 1028
	 */
	int launchFuelAdded;
	/**
	 * Duty Cycle for the Boost Solenoid
	 * offset 1032
	 */
	int launchBoostDuty;
	/**
	 * RPM Range for Hard Cut
	 * offset 1036
	 */
	int hardCutRpmRange;
	/**
	 * Time in Seconds
	 * offset 1040
	 */
	int antilagTimeout;
	/**
	 * Time in Seconds
	 * offset 1044
	 */
	int launchActivateDelay;
	/**
	 * offset 1048
	 */
	int boostPwmFrequency;
	/**
	 * offset 1052
	 */
	int vvtPwmFrequency;
	/**
	 * offset 1056
	 */
	int idleStepperTotalSteps;
	/**
	 * At what trigger index should some MAP-related math be executed? This is a performance trick to reduce load on synchronization trigger callback.
	 * offset 1060
	 */
	int mapAveragingSchedulingAtIndex;
	/**
	 * At what trigger index should some ignition-related math be executed? This is a performance trick to reduce load on synchronization trigger callback.
	 * offset 1064
	 */
	int ignMathCalculateAtIndex;
	/**
	 * offset 1068
	 */
	int mapMinBufferLength;
	/**
	 * offset 1072
	 */
	int maxVvtDeviation;
	/**
	 * Trigger cycle index at which we start tach pulse (performance consideration)
	 * offset 1076
	 */
	int tachPulseTriggerIndex;
	/**
	 * This is the IAC position during cranking, some engines start better if given more air during cranking to improve cylinder filling.
	 * offset 1080
	 */
	int crankingIACposition;
	/**
	 * offset 1084
	 */
	int nbVvtIndex;
	/**
	 * offset 1088
	 */
	int alternatorPwmFrequency;
	/**
	 * This is the number of engine cycles that the TPS position change can occur over, a longer duration will make the enrichment more active but too long may affect steady state driving, a good default is 30-60 cycles. 
	 * offset 1092
	 */
	int tpsAccelLength;
	/**
	 * offset 1096
	 */
	int engineLoadAccelLength;
	/**
	 * offset 1100
	 */
	int unused201;
	/**
	 * offset 1104
	 */
	uint8_t freeSpaceForInt[64];
	/**
	 * Closed throttle. todo: extract these two fields into a structure
	 * See also tps1_1AdcChannel
	 * set tps_min X
	 * offset 1168
	 */
	int16_t tpsMin;
	/**
	 * Full throttle. tpsMax value as 10 bit ADC value. Not Voltage!
	 * See also tps1_1AdcChannel
	 * set tps_max X
	 * offset 1170
	 */
	int16_t tpsMax;
	/**
	 * TPS error detection, what TPS % value is unrealistically low
	 * offset 1172
	 */
	int16_t tpsErrorDetectionTooLow;
	/**
	 * TPS error detection, what TPS % value is unrealistically high
	 * offset 1174
	 */
	int16_t tpsErrorDetectionTooHigh;
	/**
	 * SD card logging period, in milliseconds
	 * offset 1176
	 */
	int16_t sdCardPeriodMs;
	/**
	 * offset 1178
	 */
	int16_t idlePidDeactivationTpsThreshold;
	/**
	 * offset 1180
	 */
	int16_t stepperParkingExtraSteps;
	/**
	 * Relative to the target idle RPM
	 * offset 1182
	 */
	int16_t idlePidRpmUpperLimit;
	/**
	 * This sets the temperature above which no priming pulse is used, The value at -40 is reduced until there is no more priming injection at this temperature.
	 * offset 1184
	 */
	int16_t primeInjFalloffTemperature;
	/**
	 * offset 1186
	 */
	int16_t acCutoffLowRpm;
	/**
	 * offset 1188
	 */
	int16_t acCutoffHighRpm;
	/**
	 * on ECU start turn fuel pump on to build fuel pressure
	 * offset 1190
	 */
	int16_t startUpFuelPumpDuration;
	/**
	 * If RPM is close enough let's leave IAC alone, and maybe engage timing PID correction
	 * offset 1192
	 */
	int16_t idlePidRpmDeadZone;
	/**
	 * iTerm max value
	 * offset 1194
	 */
	int16_t idlerpmpid_iTermMax;
	/**
	 * This is the duration in cycles that the IAC will take to reach its normal idle position, it can be used to hold the idle higher for a few seconds after cranking to improve startup.
	 * offset 1196
	 */
	int16_t afterCrankingIACtaperDuration;
	/**
	 * Closed throttle#2. todo: extract these two fields into a structure
	 * See also tps2_1AdcChannel
	 * set tps2_min X
	 * offset 1198
	 */
	int16_t tps2Min;
	/**
	 * Full throttle#2. tpsMax value as 10 bit ADC value. Not Voltage!
	 * See also tps1_1AdcChannel
	 * set tps2_max X
	 * offset 1200
	 */
	int16_t tps2Max;
	/**
	 * Extra IAC, in percent between 0 and 100, tapered between zero and idle deactivation TPS value
	 * offset 1202
	 */
	int16_t iacByTpsTaper;
	/**
	 * iTerm min value
	 * offset 1204
	 */
	int16_t idlerpmpid_iTermMin;
	/**
	 * offset 1206
	 */
	int16_t acIdleRpmBump;
	/**
	 * set warningPeriod X
	 * offset 1208
	 */
	int16_t warningPeriod;
	/**
	 * When the current RPM is closer than this value to the target, closed-loop idle timing control is enabled.
	 * offset 1210
	 */
	int16_t idleTimingPidWorkZone;
	/**
	 * If the RPM closer to target than this value, disable timing correction to prevent oscillation
	 * offset 1212
	 */
	int16_t idleTimingPidDeadZone;
	/**
	 * Taper out idle timing control over this range as the engine leaves idle conditions
	 * offset 1214
	 */
	int16_t idlePidFalloffDeltaRpm;
	/**
	 * offset 1216
	 */
	int16_t fuelClosedLoopCltThreshold;
	/**
	 * offset 1218
	 */
	int16_t fuelClosedLoopTpsThreshold;
	/**
	 * offset 1220
	 */
	int16_t fuelClosedLoopRpmThreshold;
	/**
	 * offset 1222
	 */
	int16_t etbFreq;
	/**
	 * This sets the RPM limit above which the fuel cut is deactivated, activating this maintains fuel flow at high RPM to help cool pistons
	 * offset 1224
	 */
	int16_t coastingFuelCutRpmHigh;
	/**
	 * This sets the RPM limit below which the fuel cut is deactivated, this prevents jerking or issues transitioning to idle
	 * offset 1226
	 */
	int16_t coastingFuelCutRpmLow;
	/**
	 * percent between 0 and 100 below which the fuel cut is deactivated, this helps low speed drivability.
	 * offset 1228
	 */
	int16_t coastingFuelCutTps;
	/**
	 * Fuel cutoff is deactivated below this coolant threshold.
	 * offset 1230
	 */
	int16_t coastingFuelCutClt;
	/**
	 * Increases PID reaction for RPM<target by adding extra percent to PID-error
	 * offset 1232
	 */
	int16_t pidExtraForLowRpm;
	/**
	 * MAP value above which fuel injection is re-enabled.
	 * offset 1234
	 */
	int16_t coastingFuelCutMap;
	/**
	 * iTerm min value
	 * offset 1236
	 */
	int16_t etb_iTermMin;
	/**
	 * iTerm max value
	 * offset 1238
	 */
	int16_t etb_iTermMax;
	/**
	 * A delay in cycles between fuel-enrich. portions
	 * offset 1240
	 */
	int16_t tpsAccelFractionPeriod;
	/**
	 * offset 1242
	 */
	int16_t unused143;
	/**
	 * offset 1244
	 */
	uint16_t multisparkSparkDuration;
	/**
	 * offset 1246
	 */
	uint16_t multisparkDwell;
	/**
	 * offset 1248
	 */
	uint16_t multisparkMaxRpm;
	/**
	 * offset 1250
	 */
	uint8_t freeSpaceForInt16[10];
	/**
	 * offset 1260
	 */
	uint32_t tunerStudioSerialSpeed;
	/**
	 * offset 1264
	 */
	uint32_t engineChartSize;
	/**
	 * offset 1268
	 */
	uint32_t uartConsoleSerialSpeed;
	/**
	 * offset 1272
	 */
	uint8_t freeSpaceForUint32[20];
	/**
	 * this is about deciding when the injector starts it's squirt
	 * See also injectionPhase map
	 * todo: do we need even need this since we have the map anyway?
	 * offset 1292
	 */
	angle_t extraInjectionOffset;
	/**
	 * Ignition advance angle used during engine cranking, 5-10 degrees will work as a base setting for most engines.
	 * set cranking_timing_angle X
	 * offset 1296
	 */
	angle_t crankingTimingAngle;
	/**
	 * this value could be used to offset the whole ignition timing table by a constant
	 * offset 1300
	 */
	angle_t ignitionOffset;
	/**
	 * This value is the ignition timing used when in 'fixed timing' mode, i.e. constant timing
	 * This mode is useful when adjusting distributor location.
	 * offset 1304
	 */
	angle_t fixedModeTiming;
	/**
	 * Angle between Top Dead Center (TDC) and the first trigger event.
	 * Knowing this angle allows us to control timing and other angles in reference to TDC.
	 * set global_trigger_offset_angle X
	 * offset 1308
	 */
	angle_t globalTriggerAngleOffset;
	/**
	 * offset 1312
	 */
	uint8_t freeSpaceForangle_t[16];
	/**
	 * Cylinder diameter, in mm.
	 * offset 1328
	 */
	float cylinderBore;
	/**
	 * offset 1332
	 */
	float primingSquirtDurationMs;
	/**
	 * Used if useConstantDwellDuringCranking is TRUE
	 * offset 1336
	 */
	float ignitionDwellForCrankingMs;
	/**
	 * While cranking (which causes battery voltage to drop) we can calculate dwell time in shaft
	 * degrees, not in absolute time as in running mode.
	 * set cranking_charge_angle X
	 * offset 1340
	 */
	float crankingChargeAngle;
	/**
	 * We calculate knock band based of cylinderBore
	 *  Use this to override - kHz knock band override
	 * offset 1344
	 */
	float knockBandCustom;
	/**
	 * Coefficient of input voltage dividers on your PCB
	 * offset 1348
	 */
	float analogInputDividerCoefficient;
	/**
	 * This is the ratio of the resistors for the battery voltage, measure the voltage at the battery and then adjust this number until the gauge matches the reading.
	 * offset 1352
	 */
	float vbattDividerCoeff;
	/**
	 * Cooling fan turn-on temperature threshold, in Celsius
	 * offset 1356
	 */
	float fanOnTemperature;
	/**
	 * Cooling fan turn-off temperature threshold, in Celsius
	 * offset 1360
	 */
	float fanOffTemperature;
	/**
	 * This coefficient translates vehicle speed input frequency (in Hz) into vehicle speed, km/h
	 * offset 1364
	 */
	float vehicleSpeedCoef;
	/**
	 * offset 1368
	 */
	float fuelClosedLoopAfrLowThreshold;
	/**
	 * offset 1372
	 */
	float launchTimingRetard;
	/**
	 * set global_fuel_correction X
	 * offset 1376
	 */
	float globalFuelCorrection;
	/**
	 * offset 1380
	 */
	float adcVcc;
	/**
	 * maximum total number of degrees to subtract from ignition advance
	 * when knocking
	 * offset 1384
	 */
	float maxKnockSubDeg;
	/**
	 * value between 0 and 100 used in Manual mode
	 * offset 1388
	 */
	float manIdlePosition;
	/**
	 * offset 1392
	 */
	float mapFrequency0Kpa;
	/**
	 * offset 1396
	 */
	float mapFrequency100Kpa;
	/**
	 * offset 1400
	 */
	float fuelLevelEmptyTankVoltage;
	/**
	 * offset 1404
	 */
	float fuelLevelFullTankVoltage;
	/**
	 * offset 1408
	 */
	float tachPulseDuractionMs;
	/**
	 * offset 1412
	 */
	float etbItermLimit;
	/**
	 * offset 1416
	 */
	float idleStepperReactionTime;
	/**
	 * offset 1420
	 */
	float knockVThreshold;
	/**
	 * offset 1424
	 */
	float fuelRailPressure;
	/**
	 * offset 1428
	 */
	float alternator_derivativeFilterLoss;
	/**
	 * offset 1432
	 */
	float alternator_antiwindupFreq;
	/**
	 * offset 1436
	 */
	float minVvtTemperature;
	/**
	 * offset 1440
	 */
	float knockDetectionWindowStart;
	/**
	 * offset 1444
	 */
	float knockDetectionWindowEnd;
	/**
	 * TODO: finish this #413
	 * offset 1448
	 */
	float noAccelAfterHardLimitPeriodSecs;
	/**
	 * Length of time the deposited wall fuel takes to dissipate after the start of acceleration. 
	 * offset 1452
	 */
	float wwaeTau;
	/**
	 * This is the target battery voltage the alternator PID control will attempt to maintain
	 * offset 1456
	 */
	float targetVBatt;
	/**
	 * Turns off alternator output above specified TPS, enabling this reduced parasitic drag on the engine at full load.
	 * offset 1460
	 */
	float alternatorOffAboveTps;
	/**
	 * Prime pulse for cold engine, duration in ms
	 * Linear interpolation between -40F/-40C and fallout temperature.
	 * offset 1464
	 */
	float startOfCrankingPrimingPulse;
	/**
	 * Maximum change delta of TPS percentage over the 'length'. Actual TPS change has to be above this value in order for TPS/TPS acceleration to kick in.
	 * offset 1468
	 */
	float tpsAccelEnrichmentThreshold;
	/**
	 * Angle between cam sensor and VVT zero position
	 * set vvt_offset X
	 * offset 1472
	 */
	float vvtOffset;
	/**
	 * offset 1476
	 */
	float engineLoadDecelEnleanmentThreshold;
	/**
	 * offset 1480
	 */
	float engineLoadDecelEnleanmentMultiplier;
	/**
	 * offset 1484
	 */
	float engineLoadAccelEnrichmentThreshold;
	/**
	 * offset 1488
	 */
	float engineLoadAccelEnrichmentMultiplier;
	/**
	 * offset 1492
	 */
	float tpsDecelEnleanmentThreshold;
	/**
	 * offset 1496
	 */
	float tpsDecelEnleanmentMultiplier;
	/**
	 * ExpAverage alpha coefficient
	 * offset 1500
	 */
	float slowAdcAlpha;
	/**
	 * Fixed timing, useful for TDC testing
	 * offset 1504
	 */
	float fixedTiming;
	/**
	 * MAP voltage for low point
	 * offset 1508
	 */
	float mapLowValueVoltage;
	/**
	 * MAP voltage for low point
	 * offset 1512
	 */
	float mapHighValueVoltage;
	/**
	 * EGO value correction
	 * offset 1516
	 */
	float egoValueShift;
	/**
	 * kPa value at which we need to cut fuel and spark, 0 if not enabled
	 * offset 1520
	 */
	float boostCutPressure;
	/**
	 * offset 1524
	 */
	float tChargeMinRpmMinTps;
	/**
	 * offset 1528
	 */
	float tChargeMinRpmMaxTps;
	/**
	 * offset 1532
	 */
	float tChargeMaxRpmMinTps;
	/**
	 * offset 1536
	 */
	float tChargeMaxRpmMaxTps;
	/**
	 * offset 1540
	 */
	float fuelClosedLoopAfrHighThreshold;
	/**
	 * offset 1544
	 */
	float autoTuneCltThreshold;
	/**
	 * offset 1548
	 */
	float autoTuneTpsRocThreshold;
	/**
	 * offset 1552
	 */
	float autoTuneTpsQuietPeriod;
	/**
	 * offset 1556
	 */
	float postCrankingTargetClt;
	/**
	 * Fuel multiplier taper, see also postCrankingDurationSec
	 * offset 1560
	 */
	float postCrankingFactor;
	/**
	 * See also postCrankingFactor
	 * offset 1564
	 */
	float postCrankingDurationSec;
	/**
	 * offset 1568
	 */
	float idlePidActivationTime;
	/**
	 * offset 1572
	 */
	float tChargeAirCoefMin;
	/**
	 * offset 1576
	 */
	float tChargeAirCoefMax;
	/**
	 * offset 1580
	 */
	float tChargeAirFlowMax;
	/**
	 * offset 1584
	 */
	float tChargeAirIncrLimit;
	/**
	 * offset 1588
	 */
	float tChargeAirDecrLimit;
	/**
	 * kPa value which is too low to be true
	 * offset 1592
	 */
	float mapErrorDetectionTooLow;
	/**
	 * kPa value which is too high to be true
	 * offset 1596
	 */
	float mapErrorDetectionTooHigh;
	/**
	 * 0 = No fuel settling on port walls 1 = All the fuel settling on port walls setting this to 0 disables the wall wetting enrichment. 
	 * offset 1600
	 */
	float wwaeBeta;
	/**
	 * offset 1604
	 */
	float throttlePedalUpVoltage;
	/**
	 * Pedal in the floor
	 * offset 1608
	 */
	float throttlePedalWOTVoltage;
	/**
	 * offset 1612
	 */
	float etbDeadband;
	/**
	 *  ETB idle authority
	 * offset 1616
	 */
	float etbIdleThrottleRange;
	/**
	 * A fraction divisor: 1 or less = entire portion at once, or split into diminishing fractions
	 * offset 1620
	 */
	float tpsAccelFractionDivisor;
	/**
	 * offset 1624
	 */
	uint8_t freeSpaceForFloats[20];
	/**
	 * offset 1644
	 */
	uint8_t tachPulsePerRev;
	/**
	 * Trigger comparator center point voltage
	 * offset 1645
	 */
	uint8_t triggerCompCenterVolt;
	/**
	 * Trigger comparator hysteresis voltage (Min)
	 * offset 1646
	 */
	uint8_t triggerCompHystMin;
	/**
	 * Trigger comparator hysteresis voltage (Max)
	 * offset 1647
	 */
	uint8_t triggerCompHystMax;
	/**
	 * VR-sensor saturation RPM
	 * offset 1648
	 */
	uint8_t triggerCompSensorSatRpm;
	/**
	 * offset 1649
	 */
	uint8_t multisparkMaxSparkingAngle;
	/**
	 * offset 1650
	 */
	uint8_t multisparkMaxExtraSparkCount;
	/**
	 * offset 1651
	 */
	uint8_t unused260;
	/**
	 * offset 1652
	 */
	gp_pwm_s gppwm;
	/**
	 * per-cylinder timing correction
	 * offset 1744
	 */
	cfg_float_t_1f timing_offset_cylinder[IGNITION_PIN_COUNT];
	/**
	 * @see hasBaroSensor
	 * offset 1792
	 */
	air_pressure_sensor_config_s baroSensor;
	/**
	 * todo: merge with channel settings, use full-scale Thermistor here!
	 * offset 1808
	 */
	ThermistorConf clt;
	/**
	 * offset 1840
	 */
	ThermistorConf iat;
	/**
	 * offset 1872
	 */
	oil_pressure_config_s oilPressure;
	/**
	 * @see hasMapSensor
	 * @see isMapAveragingEnabled
	 * offset 1892
	 */
	MAP_sensor_config_s map;
	/**
	 * offset 1908
	 */
	injector_s injector;
	/**
	 * offset 1976
	 */
	MAP_sampling_config_s mapsampling;
	/**
	 * offset 2104
	 */
	baro_corr_table_t baroCorrTable;
	/**
	 * offset 2168
	 */
	float baroCorrPressureBins[BARO_CORR_SIZE];
	/**
	 * offset 2184
	 */
	float baroCorrRpmBins[BARO_CORR_SIZE];
	/**
	 * CLT-based timing correction
	 * offset 2200
	 */
	float cltTimingBins[CLT_TIMING_CURVE_SIZE];
	/**
	 * offset 2232
	 */
	float cltTimingExtra[CLT_TIMING_CURVE_SIZE];
	/**
	 * On single-coil or wasted spark setups you have to lower dwell at high RPM
	 * offset 2264
	 */
	float sparkDwellRpmBins[DWELL_CURVE_SIZE];
	/**
	 * offset 2296
	 */
	float sparkDwellValues[DWELL_CURVE_SIZE];
	/**
	 * CLT-based target RPM for automatic idle controller
	 * offset 2328
	 */
	float cltIdleRpmBins[CLT_CURVE_SIZE];
	/**
	 * See idleRpmPid
	 * offset 2392
	 */
	float cltIdleRpm[CLT_CURVE_SIZE];
	/**
	 * Optional timing advance table for Cranking (see useSeparateAdvanceForCranking)
	 * offset 2456
	 */
	float crankingAdvanceBins[CRANKING_ADVANCE_CURVE_SIZE];
	/**
	 * Optional timing advance table for Cranking (see useSeparateAdvanceForCranking)
	 * offset 2472
	 */
	float crankingAdvance[CRANKING_ADVANCE_CURVE_SIZE];
	/**
	 * Narrow Band WBO Approximation
	 * offset 2488
	 */
	float narrowToWideOxygenBins[NARROW_BAND_WIDE_BAND_CONVERSION_SIZE];
	/**
	 * offset 2520
	 */
	float narrowToWideOxygen[NARROW_BAND_WIDE_BAND_CONVERSION_SIZE];
	/**
	 * Knock sensor output knock detection threshold depending on current RPM
	 * offset 2552
	 */
	float knockNoise[ENGINE_NOISE_CURVE_SIZE];
	/**
	 * offset 2584
	 */
	float knockNoiseRpmBins[ENGINE_NOISE_CURVE_SIZE];
	/**
	 * Cranking fuel correction coefficient based on TPS
	 * offset 2616
	 */
	float crankingTpsCoef[CRANKING_CURVE_SIZE];
	/**
	 * offset 2648
	 */
	float crankingTpsBins[CRANKING_CURVE_SIZE];
	/**
	 * CLT-based idle position for coasting (used in Auto-PID Idle mode)
	 * offset 2680
	 */
	float iacCoastingBins[CLT_CURVE_SIZE];
	/**
	 *  CLT-based idle position for coasting (used in Auto-PID Idle mode)
	 * offset 2744
	 */
	float iacCoasting[CLT_CURVE_SIZE];
	/**
	 * offset 2808
	 */
	float mapAccelTaperBins[MAP_ACCEL_TAPER];
	/**
	 * offset 2840
	 */
	float mapAccelTaperMult[MAP_ACCEL_TAPER];
	/**
	 * target TPS value, 0 to 100%
	 * TODO: use int8 data date once we template interpolation method
	 * offset 2872
	 */
	float etbBiasBins[ETB_BIAS_CURVE_LENGTH];
	/**
	 * PWM bias, 0 to 100%
	 * offset 2896
	 */
	float etbBiasValues[ETB_BIAS_CURVE_LENGTH];
	/**
	 * offset 2920
	 */
	int mainUnuse[285];
	/**
	 * offset 4060
	 */
	iac_pid_mult_t iacPidMultTable;
	/**
	 * offset 4124
	 */
	uint8_t iacPidMultLoadBins[IAC_PID_MULT_SIZE];
	/**
	 * offset 4132
	 */
	uint8_t iacPidMultRpmBins[IAC_PID_MULT_SIZE];
	/**
	 * offset 4140
	 */
	uint8_t mainUnusedEnd[304];
	/** total size 4444*/
};

typedef struct engine_configuration_s engine_configuration_s;

// start of persistent_config_s
struct persistent_config_s {
	/**
	 * offset 0
	 */
	engine_configuration_s engineConfiguration;
	/**
	 * offset 4444
	 */
	int freeSpaceForCurves[967];
	/**
	 * offset 8312
	 */
	le_formula_t timingMultiplier;
	/**
	 * offset 8512
	 */
	le_formula_t timingAdditive;
	/**
	 * CLT-based cranking position multiplier for simple manual idle controller
	 * offset 8712
	 */
	float cltCrankingCorrBins[CLT_CRANKING_CURVE_SIZE];
	/**
	 * CLT-based cranking position multiplier for simple manual idle controller
	 * offset 8744
	 */
	float cltCrankingCorr[CLT_CRANKING_CURVE_SIZE];
	/**
	 * Optional timing advance table for Idle (see useSeparateAdvanceForIdle)
	 * offset 8776
	 */
	float idleAdvanceBins[IDLE_ADVANCE_CURVE_SIZE];
	/**
	 * Optional timing advance table for Idle (see useSeparateAdvanceForIdle)
	 * offset 8808
	 */
	float idleAdvance[IDLE_ADVANCE_CURVE_SIZE];
	/**
	 * Optional VE table for Idle (see useSeparateVEForIdle)
	 * offset 8840
	 */
	float idleVeBins[IDLE_VE_CURVE_SIZE];
	/**
	 *  Optional VE table for Idle (see useSeparateVEForIdle)
	 * offset 8872
	 */
	float idleVe[IDLE_VE_CURVE_SIZE];
	/**
	 * offset 8904
	 */
	float cltFuelCorrBins[CLT_CURVE_SIZE];
	/**
	 * offset 8968
	 */
	float cltFuelCorr[CLT_CURVE_SIZE];
	/**
	 * offset 9032
	 */
	float iatFuelCorrBins[IAT_CURVE_SIZE];
	/**
	 * offset 9096
	 */
	float iatFuelCorr[IAT_CURVE_SIZE];
	/**
	 * offset 9160
	 */
	float crankingFuelCoef[CRANKING_CURVE_SIZE];
	/**
	 * offset 9192
	 */
	float crankingFuelBins[CRANKING_CURVE_SIZE];
	/**
	 * offset 9224
	 */
	float crankingCycleCoef[CRANKING_CURVE_SIZE];
	/**
	 * offset 9256
	 */
	float crankingCycleBins[CRANKING_CURVE_SIZE];
	/**
	 * CLT-based idle position multiplier for simple manual idle controller
	 * offset 9288
	 */
	float cltIdleCorrBins[CLT_CURVE_SIZE];
	/**
	 *  CLT-based idle position multiplier for simple manual idle controller
	 * offset 9352
	 */
	float cltIdleCorr[CLT_CURVE_SIZE];
	/**
	 * kg/hour value.
	 * By the way 2.081989116 kg/h = 1 ft3/m
	 * offset 9416
	 */
	float mafDecoding[MAF_DECODING_COUNT];
	/**
	 * offset 9544
	 */
	float mafDecodingBins[MAF_DECODING_COUNT];
	/**
	 * offset 9672
	 */
	float afterstartHoldTimeBins[AFTERSTART_HOLD_CURVE_SIZE];
	/**
	 * offset 9704
	 */
	float afterstartHoldTime[AFTERSTART_HOLD_CURVE_SIZE];
	/**
	 * offset 9736
	 */
	float afterstartDecayTimeBins[AFTERSTART_DECAY_CURVE_SIZE];
	/**
	 * offset 9768
	 */
	float afterstartDecayTime[AFTERSTART_DECAY_CURVE_SIZE];
	/**
	 * offset 9800
	 */
	float afterstartEnrichBins[AFTERSTART_ENRICH_CURVE_SIZE];
	/**
	 * offset 9832
	 */
	float afterstartEnrich[AFTERSTART_ENRICH_CURVE_SIZE];
	/**
	 * offset 9864
	 */
	int freeSpaceForTables[700];
	/**
	 * offset 12664
	 */
	gp_pwm_table_t gpPwmTable1;
	/**
	 * offset 12728
	 */
	uint8_t gpPwm1LoadBins[GP_PWM_LOAD_COUNT];
	/**
	 * offset 12736
	 */
	uint8_t gpPwm1RpmBins[GP_PWM_RPM_COUNT];
	/**
	 * offset 12744
	 */
	gp_pwm_table_t gpPwmTable2;
	/**
	 * offset 12808
	 */
	uint8_t gpPwm2LoadBins[GP_PWM_LOAD_COUNT];
	/**
	 * offset 12816
	 */
	uint8_t gpPwm2RpmBins[GP_PWM_RPM_COUNT];
	/**
	 * offset 12824
	 */
	gp_pwm_table_t gpPwmTable3;
	/**
	 * offset 12888
	 */
	uint8_t gpPwm3LoadBins[GP_PWM_LOAD_COUNT];
	/**
	 * offset 12896
	 */
	uint8_t gpPwm3RpmBins[GP_PWM_RPM_COUNT];
	/**
	 * offset 12904
	 */
	gp_pwm_table_t gpPwmTable4;
	/**
	 * offset 12968
	 */
	uint8_t gpPwm4LoadBins[GP_PWM_LOAD_COUNT];
	/**
	 * offset 12976
	 */
	uint8_t gpPwm4RpmBins[GP_PWM_RPM_COUNT];
	/**
	 * offset 12984
	 */
	fsio_table_8x8_f32t vvtTable1;
	/**
	 * offset 13240
	 */
	float vvtLoadBins[VVT_LOAD_COUNT];
	/**
	 * offset 13272
	 */
	float vvtRpmBins[VVT_RPM_COUNT];
	/**
	 * offset 13304
	 */
	boost_table_t boostTableOpenLoop;
	/**
	 * offset 13368
	 */
	uint8_t boostMapBins[BOOST_LOAD_COUNT];
	/**
	 * offset 13376
	 */
	uint8_t boostRpmBins[BOOST_RPM_COUNT];
	/**
	 * offset 13384
	 */
	boost_table_t boostTableClosedLoop;
	/**
	 * offset 13448
	 */
	uint8_t boostTpsBins[BOOST_LOAD_COUNT];
	/**
	 * offset 13456
	 */
	pedal_to_tps_t pedalToTpsTable;
	/**
	 * offset 13520
	 */
	uint8_t pedalToTpsPedalBins[PEDAL_TO_TPS_SIZE];
	/**
	 * offset 13528
	 */
	uint8_t pedalToTpsRpmBins[PEDAL_TO_TPS_SIZE];
	/**
	 * offset 13536
	 */
	angle_table_t ignitionIatCorrTable;
	/**
	 * offset 14560
	 */
	float ignitionIatCorrLoadBins[IGN_LOAD_COUNT];
	/**
	 * offset 14624
	 */
	float ignitionIatCorrRpmBins[IGN_RPM_COUNT];
	/**
	 * offset 14688
	 */
	angle_table_t injectionPhase;
	/**
	 * offset 15712
	 */
	float injPhaseLoadBins[FUEL_LOAD_COUNT];
	/**
	 * offset 15776
	 */
	float injPhaseRpmBins[FUEL_RPM_COUNT];
	/**
	 * offset 15840
	 */
	fuel_table_t fuelTable;
	/**
	 * offset 16864
	 */
	float fuelLoadBins[FUEL_LOAD_COUNT];
	/**
	 * RPM is float and not integer in order to use unified methods for interpolation
	 * offset 16928
	 */
	float fuelRpmBins[FUEL_RPM_COUNT];
	/**
	 * offset 16992
	 */
	fuel_table_t fuel2Table;
	/**
	 * offset 18016
	 */
	float fuel2LoadBins[FUEL_LOAD_COUNT];
	/**
	 * offset 18080
	 */
	float fuel2RpmBins[FUEL_RPM_COUNT];
	/**
	 * offset 18144
	 */
	ignition_table_t ignitionTable;
	/**
	 * offset 19168
	 */
	float ignitionLoadBins[IGN_LOAD_COUNT];
	/**
	 * offset 19232
	 */
	float ignitionRpmBins[IGN_RPM_COUNT];
	/**
	 * offset 19296
	 */
	afr_table_t afrTable;
	/**
	 * offset 19552
	 */
	float afrLoadBins[FUEL_LOAD_COUNT];
	/**
	 * offset 19616
	 */
	float afrRpmBins[FUEL_RPM_COUNT];
	/**
	 * offset 19680
	 */
	tps_tps_table_t tpsTpsAccelTable;
	/**
	 * offset 19936
	 */
	float tpsTpsAccelFromRpmBins[TPS_TPS_ACCEL_TABLE];
	/**
	 * RPM is float and not integer in order to use unified methods for interpolation
	 * offset 19968
	 */
	float tpsTpsAccelToRpmBins[TPS_TPS_ACCEL_TABLE];
	/** total size 20000*/
};

typedef struct persistent_config_s persistent_config_s;

#endif
// end
// this section was generated automatically by rusEfi tool ConfigDefinition.jar based on integration\rusefi_config.txt Tue Mar 10 23:17:54 CET 2020
