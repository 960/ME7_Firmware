

#include "global.h"
#include "engine.h"
#include "engine_math.h"
#include "allsensors.h"
#include "engine_configuration.h"

EXTERN_ENGINE;
#undef SERIAL_SPEED
#define SERIAL_SPEED 115200

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
#if EFI_LEDS
static void setLedPins() {
	engineConfiguration->warningLedPin = GPIO_UNASSIGNED;
		engineConfiguration->runningLedPin = GPIO_UNASSIGNED;
	engineConfiguration->communicationLedPin = GPIO_UNASSIGNED; // d23 = blue
	engineConfiguration->runningLedPin = GPIO_UNASSIGNED;		// d22 = green
	engineConfiguration->triggerErrorPin = GPIOF_12;		// d27 = orange
}
#endif
static void setInjectorPins() {
	engineConfiguration->injectionPins[0] = GPIOE_2; // #1
	engineConfiguration->injectionPins[1] = GPIOB_9; // #2
	engineConfiguration->injectionPins[2] = GPIOE_1; // #3
	engineConfiguration->injectionPins[3] = GPIOE_0; // #4
	engineConfiguration->injectionPins[4] = GPIOE_5; // #5
	engineConfiguration->injectionPins[5] = GPIOE_4; // #6
	engineConfiguration->injectionPins[6] = GPIOB_8; // #7
	engineConfiguration->injectionPins[7] = GPIOB_7; // #8

	// Disable remainder
	for (int i = 8; i < INJECTION_PIN_COUNT; i++) {
		engineConfiguration->injectionPins[i] = GPIO_UNASSIGNED;
	}

	engineConfiguration->injectionPinMode = OM_DEFAULT;
}

static void setIgnitionPins() {
	engineConfiguration->ignitionPins[0] = GPIOD_2; // #1
	engineConfiguration->ignitionPins[1] = GPIOD_1; // #2
	engineConfiguration->ignitionPins[2] = GPIOD_4; // #3
	engineConfiguration->ignitionPins[3] = GPIOD_0; // #4
	engineConfiguration->ignitionPins[4] = GPIOD_3; // #5
	engineConfiguration->ignitionPins[5] = GPIOD_6; // #6
	engineConfiguration->ignitionPins[6] = GPIOC_12; // #7
	engineConfiguration->ignitionPins[7] = GPIOC_9; // #8

	// disable remainder
	for (int i = 8; i < IGNITION_PIN_COUNT; i++) {
		engineConfiguration->ignitionPins[i] = GPIO_UNASSIGNED;
	}

	engineConfiguration->ignitionPinMode = OM_INVERTED;
}

static void setupVbatt() {
	engineConfiguration->vbattDividerCoeff = 8.166666f;
	engineConfiguration->vbattAdcChannel = EFI_ADC_0;

	// 1k high side/1.5k low side = 1.6667 ratio divider
	engineConfiguration->analogInputDividerCoefficient = 2.5f / 1.5f;

	engineConfiguration->adcVcc = 3.29f;
}

static void setupTle8888() {
	// Enable spi3
	engineConfiguration->is_enabled_spi_1 = true;
	engineConfiguration->spi1mosiPin = GPIOB_5;
	engineConfiguration->spi1misoPin = GPIOB_4;
	engineConfiguration->spi1sckPin = GPIOB_3;
	engineConfiguration->tle8888_cs = GPIOD_5;
	engineConfiguration->tle8888spiDevice = SPI_DEVICE_1;
}

static void setupEtb() {

	engineConfiguration->etb_use_two_wires = true;
	engineConfiguration->etbIo[0].directionPin1 = GPIOD_7;
	engineConfiguration->etbIo[0].directionPin2 = GPIOG_9;


	engineConfiguration->etb.pFactor = 12;
	engineConfiguration->etb.iFactor = 15;
	engineConfiguration->etb.dFactor = 0;
	engineConfiguration->etb.offset = 0;
	engineConfiguration->etbFreq = 500;
}

static void setupDefaultSensorInputs() {
	engineConfiguration->triggerInputPins[0] = GPIOC_6;
	// Direct hall-only cam input
	engineConfiguration->camInputs[0] = GPIOA_8;

	engineConfiguration->isFastAdcEnabled = true;
	// Map Sensor
	engineConfiguration->map.sensor.hwChannel = EFI_ADC_12;
    engineConfiguration->map.sensor.type = MT_CUSTOM;
    engineConfiguration->mapLowValueVoltage = 0;
    engineConfiguration->mapHighValueVoltage = 5;

    engineConfiguration->mafAdcChannel = EFI_ADC_7;

    engineConfiguration->clt.adcChannel = EFI_ADC_13;
	engineConfiguration->clt.config.bias_resistor = 2700;

	engineConfiguration->iat.adcChannel = EFI_ADC_8;
	engineConfiguration->iat.config.bias_resistor = 2700;

	engineConfiguration->throttlePedalPositionAdcChannel = EFI_ADC_5;
	engineConfiguration->tps1_1AdcChannel = EFI_ADC_15;


	engineConfiguration->is_enabled_spi_2 = true;
    engineConfiguration->spi2mosiPin = GPIOB_15;
	engineConfiguration->spi2misoPin = GPIOB_14;
	engineConfiguration->spi2sckPin = GPIOB_13;


	engineConfiguration->afr.hwChannel = EFI_ADC_NONE;
	engineConfiguration->isCJ125Enabled = true;
	engineConfiguration->cj125ur = EFI_ADC_1;
	engineConfiguration->cj125ua = EFI_ADC_2;
	engineConfiguration->cj125CsPin = GPIOE_13;
	engineConfiguration->cj125isUaDivided = true;
	engineConfiguration->cj125isUrDivided = true;
	engineConfiguration->cj125SpiDevice = SPI_DEVICE_2;
	engineConfiguration->wboHeaterPin = GPIOE_9;
	engineConfiguration->cj125ModePin = GPIOE_12;
	engineConfiguration->cj125ModePinMode = OM_DEFAULT;
	engineConfiguration->cj125CsPinMode = OM_OPENDRAIN;




}
static void setEngineDefaults() {
	setOperationMode(engineConfiguration, FOUR_STROKE_CRANK_SENSOR);
	engineConfiguration->trigger.type = TT_60_2_VW;
	engineConfiguration->useOnlyRisingEdgeForTrigger = true;
	setAlgorithm(LM_SPEED_DENSITY PASS_CONFIG_PARAMETER_SUFFIX);
	engineConfiguration->specs.cylindersCount = 4;
	engineConfiguration->specs.firingOrder = FO_1_3_4_2;
	engineConfiguration->injector.flow = 280;
	engineConfiguration->specs.displacement = 1.795;
	engineConfiguration->globalTriggerAngleOffset = 114;
	engineConfiguration->ignitionMode = IM_INDIVIDUAL_COILS;
	engineConfiguration->crankingInjectionMode = IM_SEQUENTIAL;
	engineConfiguration->injectionMode = IM_SEQUENTIAL;

	engineConfiguration->isCylinderCleanupEnabled = true;
	engineConfiguration->rpmHardLimit = 8000;
	engineConfiguration->cranking.baseFuel = 5;
	engineConfiguration->tpsMin = convertVoltageTo10bitADC(1.250);
	engineConfiguration->tpsMax = convertVoltageTo10bitADC(4.538);
	engineConfiguration->tpsErrorDetectionTooLow = -10; // -10% open
	engineConfiguration->tpsErrorDetectionTooHigh = 110; // 110% open
    engineConfiguration->oilPressure.v1 = 0.5f;
	engineConfiguration->oilPressure.v2 = 4.5f;
	engineConfiguration->oilPressure.value1 = 0;
	engineConfiguration->oilPressure.value2 = 689.476f;	// 100psi = 689.476kPa


	engineConfiguration->silentTriggerError = true;
	engineConfiguration->primingSquirtDurationMs = 5;


	engineConfiguration->fuelPumpPin = GPIOE_3;


	engineConfiguration->isBoostControlEnabled = true;
			engineConfiguration->boostPwmFrequency = 55;
			engineConfiguration->boostPid.offset = 0;
			engineConfiguration->boostPid.pFactor = 0.5;
			engineConfiguration->boostPid.iFactor = 0.3;
			engineConfiguration->boostPid.periodMs = 100;
			engineConfiguration->boostPid.maxValue = 99;
			engineConfiguration->boostPid.minValue = -99;
			engineConfiguration->boostControlPin = GPIOE_3;
			engineConfiguration->boostControlPinMode = OM_DEFAULT;

#if EFI_FSIO
			setFsioExt (0, GPIO_UNASSIGNED, "rpm map 1 fsio_table 100 /", 100 PASS_CONFIG_PARAMETER_SUFFIX);
			setFsioExt (1, GPIO_UNASSIGNED, "rpm map 2 fsio_table 100 /", 100 PASS_CONFIG_PARAMETER_SUFFIX);
			setFsioExt (2, GPIO_UNASSIGNED, "rpm map 3 fsio_table 100 /", 100 PASS_CONFIG_PARAMETER_SUFFIX);
			setFsioExt (3, GPIO_UNASSIGNED, "rpm map 4 fsio_table 100 /", 100 PASS_CONFIG_PARAMETER_SUFFIX);
			setFsioExt (4, GPIOG_6, "0.15 90 coolant 120 min max 90 - 30 / 0.8 * +", 25 PASS_CONFIG_PARAMETER_SUFFIX);
#endif
}

void setengineConfigurationOverrides(void) {
	//CAN Settings

	engineConfiguration->canNbcType = CAN_BUS_NBC_VAG;
	engineConfiguration->canReadEnabled = true;
	engineConfiguration->canWriteEnabled = true;
	engineConfiguration->canTxPin = GPIOB_6;
	engineConfiguration->canRxPin = GPIOB_12;

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
}
	void setBoardConfigurationOverrides(void) {
#if EFI_LEDS
	setLedPins();
#endif
	setInjectorPins();
	setIgnitionPins();
	setupVbatt();
	setupTle8888();
	setupEtb();
	setupDefaultSensorInputs();
	setEngineDefaults();
	setengineConfigurationOverrides();
	// NOT USED
	engineConfiguration->triggerInputPins[1] = GPIO_UNASSIGNED;
		engineConfiguration->triggerInputPins[2] = GPIO_UNASSIGNED;
		engineConfiguration->fsioAdc[1] =  EFI_ADC_NONE;
		engineConfiguration->fsioAdc[2] =  EFI_ADC_NONE;
		engineConfiguration->fsioAdc[3] =  EFI_ADC_NONE;
		engineConfiguration->fsioAdc[4] =  EFI_ADC_NONE;
	engineConfiguration->dizzySparkOutputPin = GPIO_UNASSIGNED;
	engineConfiguration->externalKnockSenseAdc = EFI_ADC_NONE;



	engineConfiguration->triggerSimulatorPins[1] = GPIO_UNASSIGNED;
	engineConfiguration->triggerSimulatorPins[2] = GPIO_UNASSIGNED;
	engineConfiguration->triggerSimulatorPinModes[1] = OM_DEFAULT;
	engineConfiguration->triggerSimulatorPinModes[2] = OM_DEFAULT;


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
#if BOARD_TLE6240_COUNT > 0
	engineConfiguration->tle6240_cs = GPIO_UNASSIGNED;
#endif
#if EFI_MAX_31855
	engineConfiguration->digitalPotentiometerSpiDevice = SPI_NONE;
		engineConfiguration->max31855spiDevice = SPI_NONE;
				engineConfiguration->mc33972_cs = GPIO_UNASSIGNED;
				engineConfiguration->hip9011CsPin = GPIO_UNASSIGNED;
				engineConfiguration->LIS302DLCsPin = GPIO_UNASSIGNED;
				engineConfiguration->tle6240spiDevice = SPI_NONE;
				engineConfiguration->mc33972spiDevice = SPI_NONE;
				for (int i = 0; i < DIGIPOT_COUNT; i++) {
						engineConfiguration->digitalPotentiometerChipSelect[i] = GPIO_UNASSIGNED;
					}

#endif
#if EFI_LCD
				engineConfiguration->displayMode = DM_NONE;
	engineConfiguration->HD44780_rs = GPIO_UNASSIGNED;
		engineConfiguration->HD44780_e = GPIO_UNASSIGNED;
		engineConfiguration->HD44780_db4 = GPIO_UNASSIGNED;
		engineConfiguration->HD44780_db5 = GPIO_UNASSIGNED;
		engineConfiguration->HD44780_db6 = GPIO_UNASSIGNED;
		engineConfiguration->HD44780_db7 = GPIO_UNASSIGNED;
		engineConfiguration->o2heaterPin = GPIO_UNASSIGNED;
		engineConfiguration->HD44780_db4 = GPIO_UNASSIGNED;

			engineConfiguration->gps_rx_pin = GPIO_UNASSIGNED;
			engineConfiguration->gps_tx_pin = GPIO_UNASSIGNED;
#endif



}
void setSdCardConfigurationOverrides(void) {
}

void setAdcChannelOverrides(void) {
}

