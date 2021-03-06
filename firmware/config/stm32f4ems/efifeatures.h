/**
 * @file efifeatures.h
 *
 * @brief In this header we can configure which firmware modules are used.
 *
 * @date Aug 29, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once
#define EFI_SPI_FRAM TRUE
#define EFI_SOFTWARE_KNOCK TRUE
#define EFI_GPIO_HARDWARE TRUE
#define EFI_AUXILIARIES TRUE
#define EFI_BOOST_CONTROL TRUE
#define EFI_VVT_CONTROL TRUE

#ifndef EFI_LAUNCH_CONTROL
#define EFI_LAUNCH_CONTROL TRUE
#endif

#define EFI_FSIO FALSE

#ifndef EFI_CDM_INTEGRATION
#define EFI_CDM_INTEGRATION FALSE
#endif

#ifndef EFI_TOOTH_LOGGER
#define EFI_TOOTH_LOGGER TRUE
#endif

#define EFI_TEXT_LOG FALSE

#define EFI_PWM_TESTER FALSE

#define EFI_MC33816 FALSE

#define HAL_USE_USB_MSD FALSE

#define EFI_ENABLE_CRITICAL_ENGINE_STOP FALSE
#define EFI_ENABLE_ENGINE_WARNING FALSE

#define EFI_USE_CCM TRUE

/**
 * if you have a 60-2 trigger, or if you just want better performance, you
 * probably want EFI_ENABLE_ASSERTS to be FALSE. Also you would probably want to FALSE
 * CH_DBG_ENABLE_CHECKS
 * CH_DBG_ENABLE_ASSERTS
 * CH_DBG_ENABLE_TRACE
 * in chconf.h
 *
 */
#if !defined(EFI_ENABLE_ASSERTS)
 #define EFI_ENABLE_ASSERTS FALSE
#endif /* EFI_ENABLE_ASSERTS */

#if !defined(EFI_ENABLE_MOCK_ADC)
 #define EFI_ENABLE_MOCK_ADC TRUE
#endif /* EFI_ENABLE_MOCK_ADC */


//#define EFI_UART_ECHO_TEST_MODE TRUE

/**
 * Build-in logic analyzer support. Logic analyzer viewer is one of the java console panes.
 */
#ifndef EFI_LOGIC_ANALYZER
#define EFI_LOGIC_ANALYZER FALSE
#endif

#ifndef EFI_ICU_INPUTS
#define EFI_ICU_INPUTS TRUE
#endif

#ifndef HAL_TRIGGER_USE_PAL
#define HAL_TRIGGER_USE_PAL FALSE
#endif /* HAL_TRIGGER_USE_PAL */

/**
 * TunerStudio support.
 */
#define EFI_TUNER_STUDIO TRUE

/**
 * Bluetooth UART setup support.
 */
#define EFI_BLUETOOTH_SETUP TRUE

/**
 * TunerStudio debug output
 */
#define EFI_TUNER_STUDIO_VERBOSE FALSE

#define EFI_DEFAILED_LOGGING FALSE

/**
 * Dev console support.
 */
#define EFI_CLI_SUPPORT FALSE

#define EFI_RTC FALSE

#define EFI_ALTERNATOR_CONTROL TRUE


#define EFI_SIGNAL_EXECUTOR_SLEEP FALSE
#define EFI_SIGNAL_EXECUTOR_ONE_TIMER TRUE
#define EFI_SIGNAL_EXECUTOR_HW_TIMER FALSE

#define FUEL_MATH_EXTREME_LOGGING FALSE

#define SPARK_EXTREME_LOGGING FALSE

#define TRIGGER_EXTREME_LOGGING FALSE

#ifndef EFI_INTERNAL_FLASH
#define EFI_INTERNAL_FLASH TRUE
#endif

/**
 * Usually you need shaft position input, but maybe you do not need it?
 */
#ifndef EFI_SHAFT_POSITION_INPUT
#define EFI_SHAFT_POSITION_INPUT TRUE
#endif

/**
 * Maybe we are just sniffing what's going on?
 */
#define EFI_ENGINE_CONTROL TRUE

#ifndef BOARD_TLE6240_COUNT
#define BOARD_TLE6240_COUNT         0
#endif

#ifndef BOARD_MC33972_COUNT
#define BOARD_MC33972_COUNT			0
#endif

#ifndef BOARD_TLE8888_COUNT
#define BOARD_TLE8888_COUNT 	0
#endif

#define EFI_ANALOG_SENSORS TRUE

#ifndef EFI_MAX_31855
#define EFI_MAX_31855 FALSE
#endif

#define EFI_MCP_3208 FALSE

#ifndef EFI_HIP_9011
// disabling for now - DMA conflict with SPI1
#define EFI_HIP_9011 FALSE
#endif

#ifndef EFI_CJ125
#define EFI_CJ125 TRUE
#endif

#if !defined(EFI_MEMS)
 #define EFI_MEMS FALSE
#endif

#ifndef EFI_INTERNAL_ADC
#define EFI_INTERNAL_ADC TRUE
#endif

#define EFI_NARROW_EGO_AVERAGING FALSE

#define EFI_DENSO_ADC FALSE

#ifndef EFI_CAN_SUPPORT
#define EFI_CAN_SUPPORT TRUE
#endif

#ifndef EFI_AUX_SERIAL
#define EFI_AUX_SERIAL FALSE
#endif

#ifndef EFI_HD44780_LCD
#define EFI_HD44780_LCD FALSE
#endif

#ifndef EFI_LCD
#define EFI_LCD FALSE
#endif

#ifndef EFI_IDLE_CONTROL
#define EFI_IDLE_CONTROL TRUE
#endif

#define EFI_IDLE_PID_CIC TRUE

/**
 * Control the main power relay based on measured ignition voltage (Vbatt)
 */
#ifndef EFI_MAIN_RELAY_CONTROL
#define EFI_MAIN_RELAY_CONTROL FALSE
#endif

#ifndef EFI_PWM
#define EFI_PWM TRUE
#endif

#ifndef EFI_VEHICLE_SPEED
#define EFI_VEHICLE_SPEED TRUE
#endif

#define EFI_FUEL_PUMP TRUE

#ifndef EFI_ENGINE_EMULATOR
#define EFI_ENGINE_EMULATOR TRUE
#endif

#ifndef EFI_EMULATE_POSITION_SENSORS
#define EFI_EMULATE_POSITION_SENSORS TRUE
#endif

/**
 * This macros is used to hide hardware-specific pieces of the code from unit tests and simulator, so it only makes
 * sense in folders exposed to the tests projects (simulator and unit tests).
 * This macros is NOT about taking out logging in general.
 * See also EFI_UNIT_TEST
 * See also EFI_SIMULATOR
 * todo: do we want to rename any of these three options?
 */
#define EFI_PROD_CODE TRUE

/**
 * Do we need file log (like SD card) logic?
 */
#ifndef EFI_FILE_LOGGING
#define EFI_FILE_LOGGING FALSE
#endif

#ifndef EFI_USB_SERIAL
#define EFI_USB_SERIAL TRUE
#endif

/**
 * Should PnP engine configurations be included in the binary?
 */
#ifndef EFI_INCLUDE_ENGINE_PRESETS
#define EFI_INCLUDE_ENGINE_PRESETS FALSE
#endif

#ifndef EFI_ENGINE_SNIFFER
#define EFI_ENGINE_SNIFFER FALSE
#endif

#define EFI_HISTOGRAMS FALSE
#define EFI_SENSOR_CHART FALSE

#if defined __GNUC__
#define EFI_PERF_METRICS FALSE
#define DL_OUTPUT_BUFFER 6500
#else
#define EFI_PERF_METRICS FALSE
#define DL_OUTPUT_BUFFER 8000
#endif

/**
 * Do we need GPS logic?
 */
#define EFI_UART_GPS FALSE

#define EFI_SERVO FALSE

#define EFI_ELECTRONIC_THROTTLE_BODY TRUE
//#define EFI_ELECTRONIC_THROTTLE_BODY FALSE

/**
 * Do we need Malfunction Indicator blinking logic?
 */
#define EFI_MALFUNCTION_INDICATOR FALSE
//#define EFI_MALFUNCTION_INDICATOR FALSE

#define CONSOLE_MAX_ACTIONS 180

#define EFI_MAP_AVERAGING FALSE
//#define EFI_MAP_AVERAGING FALSE

// todo: most of this should become configurable

// todo: switch to continues ADC conversion for fast ADC?
#define EFI_INTERNAL_FAST_ADC_GPT	&GPTD6

#define EFI_SPI1_AF 5

#define EFI_SPI2_AF 5

/**
 * This section is for right-side center SPI
 */

#define EFI_SPI3_AF 6

#define EFI_I2C_SCL_BRAIN_PIN GPIOB_6

#define EFI_I2C_SDA_BRAIN_PIN GPIOB_7
#define EFI_I2C_AF 4

/**
 * Patched version of ChibiOS/RT support extra details in the system error messages
 */
#define EFI_CUSTOM_PANIC_METHOD TRUE



/**
 * currently ChibiOS uses only first and second channels of each timer for input capture
 *
 * So, our options are:
 *
 * TIM2_CH1
 *  PA5
 *
 * TIM4_CH1
 *  PB6
 * 	PD12
 *
 * TIM9_CH1
 *  PE5
 */


// todo: start using consoleUartDevice? Not sure
#ifndef EFI_CONSOLE_SERIAL_DEVICE
//#define EFI_CONSOLE_SERIAL_DEVICE (&SD3)
#endif

/**
 * Use 'HAL_USE_UART' DMA-mode driver instead of 'HAL_USE_SERIAL'
 *
 * See also
 *  STM32_SERIAL_USE_USARTx
 *  STM32_UART_USE_USARTx
 * in mcuconf.h
 */
#define TS_UART_DMA_MODE TRUE

#define TS_UART_DEVICE (&UARTD3)
//#define TS_SERIAL_DEVICE (&SD3)

#define AUX_SERIAL_DEVICE (&SD6)

// todo: add DMA-mode for Console?
#if (TS_UART_DMA_MODE || TS_UART_MODE)
#undef EFI_CONSOLE_SERIAL_DEVICE
#endif

// todo: start using consoleSerialTxPin? Not sure
#ifndef EFI_CONSOLE_TX_BRAIN_PIN
#define EFI_CONSOLE_TX_BRAIN_PIN GPIOC_10
#endif
// todo: start using consoleSerialRxPin? Not sure
#ifndef EFI_CONSOLE_RX_BRAIN_PIN
#define EFI_CONSOLE_RX_BRAIN_PIN GPIOC_11
#endif
// todo: this should be detected automatically based on pin selection
#define EFI_CONSOLE_AF 7

// todo: this should be detected automatically based on pin selection
#define TS_SERIAL_AF 7

#ifndef LED_CRITICAL_ERROR_BRAIN_PIN
#define LED_CRITICAL_ERROR_BRAIN_PIN GPIOD_14
#endif

// USART1 -> check defined STM32_SERIAL_USE_USART1
// For GPS we have USART1. We can start with PB7 USART1_RX and PB6 USART1_TX
#define GPS_SERIAL_DEVICE &SD1
#define GPS_SERIAL_SPEED 38400

#ifndef CONFIG_RESET_SWITCH_PORT
// looks like this feature is not extremely popular, we can try living without it now :)
//#define CONFIG_RESET_SWITCH_PORT GPIOD
#endif

#ifndef CONFIG_RESET_SWITCH_PIN
#define CONFIG_RESET_SWITCH_PIN 6
#endif

/**
 * This is the size of the MemoryStream used by chvprintf
 */
#define INTERMEDIATE_LOGGING_BUFFER_SIZE 2000

#define EFI_JOYSTICK FALSE
