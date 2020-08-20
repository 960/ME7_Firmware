/**
 * @file    hardware.cpp
 * @brief   Hardware package entry point
 *
 * @date May 27, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#include "global.h"


#if EFI_PROD_CODE
#include "os_access.h"
#include "trigger_input.h"
#include "fram.h"
#include "adc_inputs.h"
#include "can_hw.h"
#include "hardware.h"
#include "rtc_helper.h"
#include "os_util.h"
#include "bench_test.h"
#include "vehicle_speed.h"
#include "software_knock.h"
#include "pin_repository.h"

#include "smart_gpio.h"




#include "serial_hw.h"
#include "board.h"
#include "mpu_util.h"
//#include "usb_msd.h"

#include "AdcConfiguration.h"
#include "idle_thread.h"


#include "histogram.h"

#include "cj125.h"


#include "trigger_central.h"

#include "engine_configuration.h"

#include "perf_trace.h"
#include "boost_control.h"
#include "vvt_control.h"
#if EFI_MC33816
#include "mc33816.h"
#endif /* EFI_MC33816 */


#if EFI_INTERNAL_FLASH
#include "flash_main.h"
#endif

#if EFI_CAN_SUPPORT
#include "can_vss.h"
#endif

EXTERN_ENGINE;

static mutex_t spiMtx;

#if HAL_USE_SPI
extern bool isSpiInitialized[5];

/**
 * #311 we want to test RTC before engine start so that we do not test it while engine is running
 */
bool rtcWorks = true;

/**
 * Only one consumer can use SPI bus at a given time
 */
void lockSpi(spi_device_e device) {
	UNUSED(device);
	efiAssertVoid(CUSTOM_STACK_SPI, getCurrentRemainingStack() > 128, "lockSpi");
	// todo: different locks for different SPI devices!
	chMtxLock(&spiMtx);
}

void unlockSpi(spi_device_e device) {
	chMtxUnlock(&spiMtx);
}

static void initSpiModules(engine_configuration_s *engineConfiguration) {
	UNUSED(engineConfiguration);


	setPinConfigurationOverrides(PASS_ENGINE_PARAMETER_SUFFIX);

	if (engine->is_enabled_spi_1) {
		 turnOnSpi(SPI_DEVICE_1);
	}
	if (engine->is_enabled_spi_2) {
		turnOnSpi(SPI_DEVICE_2);
	}
	if (engine->is_enabled_spi_3) {
		turnOnSpi(SPI_DEVICE_3);
	}
	if (engine->is_enabled_spi_4) {
		turnOnSpi(SPI_DEVICE_4);
	}
}

/**
 * @return NULL if SPI device not specified
 */
SPIDriver * getSpiDevice(spi_device_e spiDevice) {
	if (spiDevice == SPI_NONE) {
		return NULL;
	}
#if STM32_SPI_USE_SPI1
	if (spiDevice == SPI_DEVICE_1) {
		return &SPID1;
	}
#endif
#if STM32_SPI_USE_SPI2
	if (spiDevice == SPI_DEVICE_2) {
		return &SPID2;
	}
#endif
#if STM32_SPI_USE_SPI3
	if (spiDevice == SPI_DEVICE_3) {
		return &SPID3;
	}
#endif
#if STM32_SPI_USE_SPI4
	if (spiDevice == SPI_DEVICE_4) {
		return &SPID4;
	}
#endif
	warning(CUSTOM_ERR_UNEXPECTED_SPI, "Unexpected SPI device: %d", spiDevice);
	return NULL;
}
#endif





#if EFI_PROD_CODE

#define TPS_IS_SLOW -1

static int fastMapSampleIndex;

static int tpsSampleIndex;

#if HAL_USE_ADC
extern AdcDevice fastAdc;

/**
 * This method is not in the adc* lower-level file because it is more business logic then hardware.
 */
void adc_callback_fast(ADCDriver *adcp, adcsample_t *buffer, size_t n) {
	(void) buffer;
	(void) n;

	ScopePerf perf(PE::AdcCallbackFast);

	/**
	 * Note, only in the ADC_COMPLETE state because the ADC driver fires an
	 * intermediate callback when the buffer is half full.
	 * */
	if (adcp->state == ADC_COMPLETE) {
		ScopePerf perf(PE::AdcCallbackFastComplete);

		fastAdc.invalidateSamplesCache();

		/**
		 * this callback is executed 10 000 times a second, it needs to be as fast as possible
		 */
		efiAssertVoid(CUSTOM_ERR_6676, getCurrentRemainingStack() > 128, "lowstck#9b");



//		if (tpsSampleIndex != TPS_IS_SLOW) {
//			tpsFastAdc = fastAdc.samples[tpsSampleIndex];
//		}
	}
}
#endif /* HAL_USE_ADC */

static void calcFastAdcIndexes(void) {
#if HAL_USE_ADC
	fastMapSampleIndex = fastAdc.internalAdcIndexByHardwareIndex[engineConfiguration->map.sensor.hwChannel];

	if (engineConfiguration->tps1_1AdcChannel != EFI_ADC_NONE) {
		tpsSampleIndex = fastAdc.internalAdcIndexByHardwareIndex[engineConfiguration->tps1_1AdcChannel];
	} else {
		tpsSampleIndex = TPS_IS_SLOW;
	}

#endif/* HAL_USE_ADC */
}

static void adcConfigListener(Engine *engine) {
	UNUSED(engine);
	// todo: something is not right here - looks like should be a callback for each configuration change?
	calcFastAdcIndexes();
}

void turnOnHardware() {

	turnOnTriggerInputPins();

}

void stopSpi(spi_device_e device) {
#if HAL_USE_SPI
	if (!isSpiInitialized[device]) {
		return; // not turned on
	}
	isSpiInitialized[device] = false;
	brain_pin_markUnused(getSckPin(device));
	brain_pin_markUnused(getMisoPin(device));
	brain_pin_markUnused(getMosiPin(device));
#endif /* HAL_USE_SPI */
}

/**
 * this method is NOT currently invoked on ECU start
 * todo: maybe start invoking this method on ECU start so that peripheral start-up initialization and restart are unified?
 */
void applyNewHardwareSettings(void) {
    // all 'stop' methods need to go before we begin starting pins


	stopTriggerInputPins();

	
	enginePins.stopInjectionPins();
    enginePins.stopIgnitionPins();
#if EFI_CAN_SUPPORT
	stopCanPins();
#endif /* EFI_CAN_SUPPORT */

#if EFI_AUX_SERIAL
	stopAuxSerialPins();
#endif /* EFI_AUX_SERIAL */


#if EFI_IDLE_CONTROL
	bool isIdleRestartNeeded = isIdleHardwareRestartNeeded();
	if (isIdleRestartNeeded) {
		stopIdleHardware();
	}
#endif

#if EFI_VEHICLE_SPEED
	stopVSSPins();
#endif /* EFI_VEHICLE_SPEED */


//	if (isConfigurationChanged(is_enabled_spi_1)) {
//		stopSpi(SPI_DEVICE_1);
//	}

//	if (isConfigurationChanged(is_enabled_spi_2)) {
//		stopSpi(SPI_DEVICE_2);
//	}

//	if (isConfigurationChanged(is_enabled_spi_3)) {
//		stopSpi(SPI_DEVICE_3);
//	}

//	if (isConfigurationChanged(is_enabled_spi_4)) {
//		stopSpi(SPI_DEVICE_4);
//	}


#if EFI_BOOST_CONTROL
	stopBoostPin();
#endif

#if EFI_VVT_CONTROL
	stopVvtPin();
#endif

	if (isPinOrModeChanged(clutchUpPin, clutchUpPinMode)) {
		brain_pin_markUnused(activeConfiguration.clutchUpPin);
	}

	if (isPinOrModeChanged(startStopButtonPin, startStopButtonMode)) {
		brain_pin_markUnused(activeConfiguration.startStopButtonPin);
	}

	enginePins.unregisterPins();


	startTriggerInputPins();

	engine->setHardCodedPins(PASS_ENGINE_PARAMETER_SIGNATURE);

	enginePins.startInjectionPins();
	enginePins.startIgnitionPins();
#if EFI_CAN_SUPPORT
	startCanPins();
#endif /* EFI_CAN_SUPPORT */




#if EFI_IDLE_CONTROL
	if (isIdleRestartNeeded) {
		 initIdleHardware();
	}
#endif

#if EFI_VEHICLE_SPEED
	startVSSPins();
#endif /* EFI_VEHICLE_SPEED */

#if EFI_BOOST_CONTROL
	startBoostPin();
#endif

#if EFI_VVT_CONTROL
	startVvtPin();
#endif
#if EFI_CJ125
	cjStartCalibration();
#endif
	adcConfigListener(engine);
}

void setBor(int borValue) {

	BOR_Set((BOR_Level_t)borValue);

}


void initHardware() {
	efiAssertVoid(CUSTOM_IH_STACK, getCurrentRemainingStack() > EXPECTED_REMAINING_STACK, "init h");

	engine_configuration_s *engineConfiguration = engine->engineConfigurationPtr;
	efiAssertVoid(CUSTOM_EC_NULL, engineConfiguration!=NULL, "engineConfiguration");
	


	// todo: enable protection. it's disabled because it takes
	// 10 extra seconds to re-flash the chip
	//flashProtect();

	chMtxObjectInit(&spiMtx);


	/**
	 * We need the LED_ERROR pin even before we read configuration
	 */
	initPrimaryPins();

	if (hasFirmwareError()) {
		return;
	}

#if EFI_SPI_FRAM
	initEeprom();
	identify();
#endif

	initFlash();
	/**
	 * this call reads configuration from flash memory or sets default configuration
	 * if flash state does not look right.
	 *
	 * interesting fact that we have another read from flash before we get here
	 */

		readFromFlash();


	// it's important to initialize this pretty early in the game before any scheduling usages
	initSingleTimerExecutorHardware();

	if (hasFirmwareError()) {
		return;
	}


#if HAL_USE_ADC
	initAdcInputs();
	// wait for first set of ADC values so that we do not produce invalid sensor data
	waitForSlowAdc(1);
#endif /* HAL_USE_ADC */

	initRtc();

#if HAL_USE_SPI
	initSpiModules(engineConfiguration);
#endif /* HAL_USE_SPI */

#if BOARD_EXT_GPIOCHIPS > 0
	// initSmartGpio depends on 'initSpiModules'
	initSmartGpio(PASS_ENGINE_PARAMETER_SIGNATURE);
#endif

	if (CONFIG(startStopButtonPin) != GPIO_UNASSIGNED) {
		efiSetPadMode("start/stop", CONFIG(startStopButtonPin),
				getInputMode(CONFIG(startStopButtonMode)));
	}


	// output pins potentially depend on 'initSmartGpio'
	initOutputPins(PASS_ENGINE_PARAMETER_SIGNATURE);

#if EFI_MC33816
	initMc33816();
#endif /* EFI_MC33816 */

#if EFI_CAN_SUPPORT
	initCan();
#endif /* EFI_CAN_SUPPORT */

//	init_adc_mcp3208(&adcState, &SPID2);
//	requestAdcValue(&adcState, 0);


	// todo: figure out better startup logic
	initTriggerCentral();


	turnOnHardware();



#if ADC_SNIFFER
	initAdcDriver();
#endif


#if EFI_AUX_SERIAL
	initAuxSerial();
#endif /* EFI_AUX_SERIAL */

//	USBMassStorageDriver UMSD1;

//	while (true) {
//		for (int addr = 0x20; addr < 0x28; addr++) {
//			sendI2Cbyte(addr, 0);
//			int err = i2cGetErrors(&I2CD1);
//			print("I2C: err=%x from %d\r\n", err, addr);
//			chThdSleepMilliseconds(5);
//			sendI2Cbyte(addr, 255);
//			chThdSleepMilliseconds(5);
//		}
//	}

#if EFI_VEHICLE_SPEED
	initVehicleSpeed();
#endif

#if EFI_CAN_SUPPORT
	initCanVssSupport();
#endif
#if EFI_SOFTWARE_KNOCK
	initSoftwareKnock();
#endif /* EFI_SOFTWARE_KNOCK */

	calcFastAdcIndexes();


}

#endif /* EFI_PROD_CODE */

#endif  /* EFI_PROD_CODE || EFI_SIMULATOR */

#if HAL_USE_SPI
// this is F4 implementation but we will keep it here for now for simplicity
int getSpiPrescaler(spi_speed_e speed, spi_device_e device) {
	switch (speed) {
	case _5MHz:
		return device == SPI_DEVICE_1 ? SPI_BaudRatePrescaler_16 : SPI_BaudRatePrescaler_8;
	case _2_5MHz:
		return device == SPI_DEVICE_1 ? SPI_BaudRatePrescaler_32 : SPI_BaudRatePrescaler_16;
	case _1_25MHz:
		return device == SPI_DEVICE_1 ? SPI_BaudRatePrescaler_64 : SPI_BaudRatePrescaler_32;

	case _150KHz:
		// SPI1 does not support 150KHz, it would be 300KHz for SPI1
		return SPI_BaudRatePrescaler_256;
	default:
		// unexpected
		return 0;
	}
}

#endif /* HAL_USE_SPI */
