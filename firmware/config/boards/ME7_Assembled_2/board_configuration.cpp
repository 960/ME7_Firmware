/**
 * @file boards/ME7_Assembled_2/board_configuration.cpp
 *
 *
 * @brief Configuration defaults for the ME7_Assembled_2 board
 *
 * 
 *
 * @author Ola Ruud, (c) 2019
 */

#include "global.h"
#include "engine.h"
#include "engine_math.h"
#include "allsensors.h"
#include "fsio_impl.h"
#include "engine_configuration.h"

EXTERN_ENGINE;




static void setInjectorPins() {
	engineConfiguration->injectionPins[0] = GPIOE_2;
	engineConfiguration->injectionPins[1] = GPIOB_9;
	engineConfiguration->injectionPins[2] = GPIOE_1;
	engineConfiguration->injectionPins[3] = GPIOE_0;
	engineConfiguration->injectionPins[4] = GPIOE_5;
	engineConfiguration->injectionPins[5] = GPIOB_8;
	engineConfiguration->injectionPins[6] = GPIOE_0;
	engineConfiguration->injectionPins[7] = GPIOB_7;
	// Disable remainder
	for (int i = 8; i < INJECTION_PIN_COUNT;i++) {
		engineConfiguration->injectionPins[i] = GPIO_UNASSIGNED;
	}

	engineConfiguration->injectionPinMode = OM_DEFAULT;
}

static void setIgnitionPins() {
	// todo: I wonder if these are not right in light of the network rename and the +12 VP issue?
	engineConfiguration->ignitionPins[0] = GPIOD_2;
	engineConfiguration->ignitionPins[1] = GPIOD_1;
	engineConfiguration->ignitionPins[2] = GPIOD_4;
	engineConfiguration->ignitionPins[3] = GPIOD_0;
	engineConfiguration->ignitionPins[4] = GPIOD_3;
	engineConfiguration->ignitionPins[5] = GPIOD_6;
	engineConfiguration->ignitionPins[6] = GPIOC_12;
	engineConfiguration->ignitionPins[7] = GPIOC_9;


	// disable remainder
	for (int i = 8; i < IGNITION_PIN_COUNT; i++) {
		engineConfiguration->ignitionPins[i] = GPIO_UNASSIGNED;
	}

	engineConfiguration->ignitionPinMode = OM_INVERTED;
}




static void setLedPins() {
#ifdef EFI_COMMUNICATION_PIN
	engineConfiguration->communicationLedPin = GPIO_UNASSIGNED;
#else
	engineConfiguration->communicationLedPin = GPIO_UNASSIGNED; // d23 = blue
#endif /* EFI_COMMUNICATION_PIN */
	engineConfiguration->runningLedPin = GPIO_UNASSIGNED;		// d22 = green
	engineConfiguration->triggerErrorPin = GPIO_UNASSIGNED;		// d27 = orange
}

static void setupVbatt() {
	// 1k high side/1.5k low side = 1.6667 ratio divider
	engineConfiguration->analogInputDividerCoefficient = 2.5f / 1.5f;

	// 39k high side/10k low side multiplied by above analogInputDividerCoefficient = 8.166666f
	engineConfiguration->vbattDividerCoeff = (49.0f / 10.0f) * engineConfiguration->analogInputDividerCoefficient;
	engineConfiguration->vbattAdcChannel = EFI_ADC_11;

	engineConfiguration->adcVcc = 3.29f;
}

static void setupTle8888() {
	// Enable spi3
	engineConfiguration->is_enabled_spi_1 = true;

	// Wire up spi3
	engineConfiguration->spi3mosiPin = GPIOB_5;
	engineConfiguration->spi3misoPin = GPIOB_4;
	engineConfiguration->spi3sckPin = GPIOB_3;

	// Chip select
	engineConfiguration->tle8888_cs = GPIOD_5;

	// SPI device
	engineConfiguration->tle8888spiDevice = SPI_DEVICE_1;
}

static void setupEtb() {
	

	// we only have pwm/dir, no dira/dirb
	engineConfiguration->etb_use_two_wires = true;
	CONFIG(etbIo[0].directionPin1) = GPIOD_7; 
	CONFIG(etbIo[0].directionPin2) = GPIOG_9;
	CONFIG(etbIo[0].controlPin1) = GPIO_UNASSIGNED;
	engineConfiguration->etbFreq = 500;
}

static void setupDefaultSensorInputs() {
	// trigger inputs
	// tle8888 VR conditioner
	engineConfiguration->triggerInputPins[0] = GPIOC_6;
	engineConfiguration->triggerInputPins[1] = GPIO_UNASSIGNED;
	engineConfiguration->triggerInputPins[2] = GPIO_UNASSIGNED;
	// Direct hall-only cam input
	engineConfiguration->camInputs[0] = GPIOA_8;

	// open question if it's great to have TPS in default TPS - the down-side is for
	// vehicles without TPS or for first start without TPS one would have to turn in off
	// to avoid cranking corrections based on wrong TPS data
	// tps = "20 - AN volt 5"
	engineConfiguration->tps1_1AdcChannel = EFI_ADC_15;
	engineConfiguration->tps2_1AdcChannel = EFI_ADC_NONE;


	// EFI_ADC_10: "27 - AN volt 1"
	engineConfiguration->map.sensor.hwChannel = EFI_ADC_12;

	// EFI_ADC_14: "32 - AN volt 6"
	engineConfiguration->afr.hwChannel = EFI_ADC_NONE;

	// clt = "18 - AN temp 1"
	engineConfiguration->clt.adcChannel = EFI_ADC_13;
	engineConfiguration->clt.config.bias_resistor = 2700;

	// iat = "23 - AN temp 2"
	engineConfiguration->iat.adcChannel = EFI_ADC_8;
	engineConfiguration->iat.config.bias_resistor = 2700;
}

void setPinConfigurationOverrides(void) {
}

void setSerialConfigurationOverrides(void) {
	engineConfiguration->useSerialPort = false;
	engineConfiguration->binarySerialTxPin = GPIO_UNASSIGNED;
	engineConfiguration->binarySerialRxPin = GPIO_UNASSIGNED;
	engineConfiguration->consoleSerialTxPin = GPIO_UNASSIGNED;
	engineConfiguration->consoleSerialRxPin = GPIO_UNASSIGNED;
}


/**
 * @brief   Board-specific configuration code overrides.
 *
 * See also setDefaultEngineConfiguration
 *
 * @todo    Add your board-specific code, if any.
 */
void setengineConfigurationOverrides(void) {
	setInjectorPins();
	setIgnitionPins();
	setLedPins();
	setupVbatt();
	setupTle8888();
	setupEtb();

	// MRE has a special main relay control low side pin
	// rusEfi firmware is totally not involved with main relay control on microRusEfi board
	// todo: maybe even set EFI_MAIN_RELAY_CONTROL to FALSE for MRE configuration
	// TLE8888 half bridges (pushpull, lowside, or high-low)  TLE8888_IN11 / TLE8888_OUT21
	// GPIOE_8: "35 - GP Out 1"
	engineConfiguration->fuelPumpPin = GPIO_UNASSIGNED;


	// TLE8888 high current low side: VVT2 IN9 / OUT5
	// GPIOE_10: "3 - Lowside 2"
	engineConfiguration->idle.solenoidPin = GPIO_UNASSIGNED;


	// TLE8888_PIN_22: "34 - GP Out 2"
	engineConfiguration->fanPin = GPIO_UNASSIGNED;

	// "required" hardware is done - set some reasonable defaults
	setupDefaultSensorInputs();

	// Some sensible defaults for other options
	setOperationMode(engineConfiguration, FOUR_STROKE_CRANK_SENSOR);
	engineConfiguration->trigger.type = TT_TOOTHED_WHEEL_60_2;
	engineConfiguration->useOnlyRisingEdgeForTrigger = true;
	setAlgorithm(LM_SPEED_DENSITY PASS_CONFIG_PARAMETER_SUFFIX);

	engineConfiguration->specs.cylindersCount = 4;
	engineConfiguration->specs.firingOrder = FO_1_3_4_2;

	engineConfiguration->ignitionMode = IM_INDIVIDUAL_COILS; // IM_WASTED_SPARK
	engineConfiguration->crankingInjectionMode = IM_SIMULTANEOUS;
	engineConfiguration->injectionMode = IM_SIMULTANEOUS;//IM_BATCH;// IM_SEQUENTIAL;
}

void setAdcChannelOverrides(void) {
}

/**
 * @brief   Board-specific SD card configuration code overrides. Needed by bootloader code.
 * @todo    Add your board-specific code, if any.
 */
void setSdCardConfigurationOverrides(void) {
}
