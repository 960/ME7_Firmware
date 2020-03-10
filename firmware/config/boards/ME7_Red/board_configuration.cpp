/**
 * @file boards/microrusefi/board_configuration.cpp
 *
 *
 * @brief Configuration defaults for the microRusefi board
 *
 * See https://github.com/rusefi/rusefi_documentation/wiki/Hardware_microRusEfi_wiring
 *
 * @author Matthew Kennedy, (c) 2019
 */

#include "global.h"
#include "engine.h"
#include "engine_math.h"
#include "allsensors.h"
#include "fsio_impl.h"
#include "engine_configuration.h"

EXTERN_ENGINE;

void setPinConfigurationOverrides(void) {
}

void setSerialConfigurationOverrides(void) {
	engineConfiguration->useSerialPort = true;

	//UART

	engineConfiguration->binarySerialTxPin = GPIOC_10;
	engineConfiguration->binarySerialRxPin = GPIOC_11;
	engineConfiguration->consoleSerialTxPin = GPIOC_10;
	engineConfiguration->consoleSerialRxPin = GPIOC_11;

}
static void setLedPins() {
	engineConfiguration->communicationLedPin = GPIO_UNASSIGNED; // d23 = blue
	engineConfiguration->runningLedPin = GPIO_UNASSIGNED;		// d22 = green
	engineConfiguration->triggerErrorPin = GPIO_UNASSIGNED;		// d27 = orange
}
static void setInjectorPins() {
	engineConfiguration->injectionPins[0] = GPIOE_4; // #1
	engineConfiguration->injectionPins[1] = GPIOE_2; // #2
	engineConfiguration->injectionPins[2] = GPIOE_3; // #3
	engineConfiguration->injectionPins[3] = GPIOE_1; // #4
	engineConfiguration->injectionPins[4] = GPIOE_6; // #5
	engineConfiguration->injectionPins[5] = GPIOE_5; // #6
	engineConfiguration->injectionPins[6] = GPIOE_0; // #7
	engineConfiguration->injectionPins[7] = GPIOB_9; // #8

	// Disable remainder
	for (int i = 8; i < INJECTION_PIN_COUNT; i++) {
		engineConfiguration->injectionPins[i] = GPIO_UNASSIGNED;
	}

	engineConfiguration->injectionPinMode = OM_OPENDRAIN_INVERTED;
}

static void setIgnitionPins() {
	engineConfiguration->ignitionPins[0] = GPIOD_3; // #1
	engineConfiguration->ignitionPins[1] = GPIOD_6; // #2
	engineConfiguration->ignitionPins[2] = GPIOD_2; // #3
	engineConfiguration->ignitionPins[3] = GPIOD_4; // #4
	engineConfiguration->ignitionPins[4] = GPIOD_0; // #5
	engineConfiguration->ignitionPins[5] = GPIOD_1; // #6
	// disable remainder
	for (int i = 6; i < IGNITION_PIN_COUNT; i++) {
		engineConfiguration->ignitionPins[i] = GPIO_UNASSIGNED;
	}

	engineConfiguration->ignitionPinMode = OM_INVERTED;
}


static void setupVbatt() {
	engineConfiguration->vbattAdcChannel = EFI_ADC_0;

	// 1k high side/1.5k low side = 1.6667 ratio divider
	engineConfiguration->analogInputDividerCoefficient = 2.5f / 1.5f;

	engineConfiguration->vbattDividerCoeff = (49.0f / 10.0f) * engineConfiguration->analogInputDividerCoefficient;
	engineConfiguration->adcVcc = 3.29f;
}

static void setupTle8888() {
	// Enable spi3
	engineConfiguration->is_enabled_spi_1 = true;
	engineConfiguration->spi3mosiPin = GPIOB_5;
	engineConfiguration->spi3misoPin = GPIOB_4;
	engineConfiguration->spi3sckPin = GPIOB_3;
	engineConfiguration->tle8888_cs = GPIOD_5;
	engineConfiguration->tle8888spiDevice = SPI_DEVICE_1;
}

static void setupEtb() {


	engineConfiguration->etb_use_two_wires = true;
		CONFIG(etbIo[0].directionPin1) = GPIOD_7;
		CONFIG(etbIo[0].directionPin2) = GPIOG_10;
		CONFIG(etbIo[0].controlPin1) = GPIO_UNASSIGNED;
		engineConfiguration->etbFreq = 500;

	engineConfiguration->etb.pFactor = 15;
	engineConfiguration->etb.iFactor = 3;
	engineConfiguration->etb.dFactor = 0.05;
	engineConfiguration->etbFreq = 500;
}

static void setupDefaultSensorInputs() {
	engineConfiguration->triggerInputPins[0] = GPIOC_6;
	// Direct hall-only cam input
	engineConfiguration->camInputs[0] = GPIOA_5;

	engineConfiguration->isFastAdcEnabled = true;
    engineConfiguration->map.sensor.type = MT_CUSTOM;
	engineConfiguration->clt.config.bias_resistor = 2700;
	engineConfiguration->iat.config.bias_resistor = 2700;
	engineConfiguration->throttlePedalPositionAdcChannel = EFI_ADC_4;
	engineConfiguration->tps1_1AdcChannel = EFI_ADC_8;
	engineConfiguration->map.sensor.hwChannel = EFI_ADC_12;
	engineConfiguration->clt.adcChannel = EFI_ADC_13;
	engineConfiguration->iat.adcChannel = EFI_ADC_15;
	engineConfiguration->afr.hwChannel = EFI_ADC_NONE;
	engineConfiguration->cj125ur = EFI_ADC_1;
	engineConfiguration->cj125ua = EFI_ADC_2;
	engineConfiguration->mafAdcChannel = EFI_ADC_6;
}
static void setEngineDefaults() {
	setOperationMode(engineConfiguration, FOUR_STROKE_CRANK_SENSOR);
	engineConfiguration->trigger.type = TT_60_2_VW;
	engineConfiguration->useOnlyRisingEdgeForTrigger = true;
	setAlgorithm(LM_SPEED_DENSITY PASS_CONFIG_PARAMETER_SUFFIX);
	engineConfiguration->specs.cylindersCount = 4;
	engineConfiguration->specs.firingOrder = FO_1_3_4_2;
	engineConfiguration->injector.flow = 288;
	engineConfiguration->specs.displacement = 1.788;
	engineConfiguration->globalTriggerAngleOffset = 114;
	engineConfiguration->ignitionMode = IM_INDIVIDUAL_COILS;
	engineConfiguration->crankingInjectionMode = IM_SEQUENTIAL;
	engineConfiguration->injectionMode = IM_SEQUENTIAL;
	engineConfiguration->ignitionPinMode = OM_DEFAULT;
	engineConfiguration->injectionPinMode = OM_OPENDRAIN_INVERTED;
	engineConfiguration->isCylinderCleanupEnabled = true;
	engineConfiguration->rpmHardLimit = 8000;
	engineConfiguration->cranking.baseFuel = 5;

	setInjectorPins();
	setIgnitionPins();
	setLedPins();
	setupVbatt();
	setupTle8888();
	setupEtb();
	setupDefaultSensorInputs();
	setEngineDefaults();


}
void setengineConfigurationOverrides(void) {
	//CAN Settings

	engineConfiguration->canNbcType = CAN_BUS_NBC_VAG;
	engineConfiguration->canReadEnabled = true;
	engineConfiguration->canWriteEnabled = true;
	engineConfiguration->canTxPin = GPIOB_6;
	engineConfiguration->canRxPin = GPIOB_12;
	engineConfiguration->cj125SpiDevice = SPI_DEVICE_2;
	engineConfiguration->cj125CsPin = GPIOB_11;

	//Digital Inputs/Outputs

	engineConfiguration->fuelPumpPin = GPIOG_4;
	engineConfiguration->mainRelayPin = GPIO_UNASSIGNED;
	engineConfiguration->idle.solenoidPin = GPIO_UNASSIGNED;
	engineConfiguration->fanPin = GPIO_UNASSIGNED;
	engineConfiguration->clutchDownPin = GPIO_UNASSIGNED;
	engineConfiguration->brakePedalPin = GPIOE_10;
	engineConfiguration->triggerInputPins[0] = GPIOC_6;

	engineConfiguration->tps2_1AdcChannel = EFI_ADC_NONE;
	//ETB Settings


#if EFI_FSIO
	 setFsioExt (1, GPIO_UNASSIGNED,"rpm map 2 fsio_table 100 /", 25 PASS_CONFIG_PARAMETER_SUFFIX);
			    setFsioExt (2, GPIO_UNASSIGNED,"rpm map 3 fsio_table 100 /", 25 PASS_CONFIG_PARAMETER_SUFFIX);
			    setFsioExt (3, GPIO_UNASSIGNED,"rpm map 4 fsio_table 100 /", 25 PASS_CONFIG_PARAMETER_SUFFIX);
				setFsioExt (4, GPIO_UNASSIGNED, "0.15 90 coolant 120 min max 90 - 30 / 0.8 * +", 25 PASS_CONFIG_PARAMETER_SUFFIX);
#endif
	
	// NOT USED
	engineConfiguration->triggerInputPins[1] = GPIO_UNASSIGNED;
		engineConfiguration->triggerInputPins[2] = GPIO_UNASSIGNED;
	engineConfiguration->isHip9011Enabled = false;
	engineConfiguration->dizzySparkOutputPin = GPIO_UNASSIGNED;
	engineConfiguration->externalKnockSenseAdc = EFI_ADC_NONE;
	engineConfiguration->displayMode = DM_NONE;
	for (int i = 0; i < DIGIPOT_COUNT; i++) {
		engineConfiguration->digitalPotentiometerChipSelect[i] = GPIO_UNASSIGNED;
	}
	engineConfiguration->triggerSimulatorPins[1] = GPIO_UNASSIGNED;
	engineConfiguration->triggerSimulatorPins[2] = GPIO_UNASSIGNED;
	engineConfiguration->triggerSimulatorPinModes[1] = OM_DEFAULT;
	engineConfiguration->triggerSimulatorPinModes[2] = OM_DEFAULT;
	engineConfiguration->digitalPotentiometerSpiDevice = SPI_NONE;
	engineConfiguration->max31855spiDevice = SPI_NONE;
	engineConfiguration->warningLedPin = GPIO_UNASSIGNED;
	engineConfiguration->runningLedPin = GPIO_UNASSIGNED;
	engineConfiguration->useStepperIdle = false;
	engineConfiguration->idle.stepperDirectionPin = GPIO_UNASSIGNED;
	engineConfiguration->idle.stepperStepPin = GPIO_UNASSIGNED;
	engineConfiguration->stepperEnablePin = GPIO_UNASSIGNED;
	engineConfiguration->stepperEnablePinMode = OM_DEFAULT;
	engineConfiguration->injectionPins[8] = GPIO_UNASSIGNED;
	engineConfiguration->injectionPins[9] = GPIO_UNASSIGNED;
	engineConfiguration->injectionPins[10] = GPIO_UNASSIGNED;
	engineConfiguration->injectionPins[11] = GPIO_UNASSIGNED;
	engineConfiguration->tachOutputPin = GPIO_UNASSIGNED;
}




void setSdCardConfigurationOverrides(void) {
}

void setAdcChannelOverrides(void) {
}

