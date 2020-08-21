/**
 * @file efifeatures.h
 *
 * @brief In this header we can configure which firmware modules are used.
 *
 * STM32F7 config is inherited from STM32F4. This file contains only differences between F4 and F7.
 * This is more consistent way to maintain these config 'branches' and add new features.
 *
 * @date Aug 29, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 */
 
#include "../stm32f4ems/efifeatures.h"

#pragma once
#define EFI_BOOST_CONTROL TRUE

// Warning! This is a test config!

#undef EFI_SPI_FRAM
#define EFI_SPI_FRAM FALSE

#define EFI_ANTILAG FALSE

#undef EFI_LAUNCH_CONTROL
#define EFI_LAUNCH_CONTROL TRUE

#undef EFI_CJ125
#define EFI_CJ125 TRUE

#undef EFI_USE_CCM
#define EFI_USE_CCM TRUE


#undef EFI_MC33816
#define EFI_MC33816 FALSE


#undef EFI_CDM_INTEGRATION
#define EFI_CDM_INTEGRATION FALSE

#undef EFI_PWM_TESTER
#define EFI_PWM_TESTER FALSE

#undef EFI_MC3381
#define EFI_MC33816 FALSE


#undef EFI_ICU_INPUTS
#define EFI_ICU_INPUTS FALSE

#undef HAL_USE_ICU
#define HAL_USE_ICU FALSE

#undef HAL_TRIGGER_USE_PAL
#define HAL_TRIGGER_USE_PAL TRUE

#undef EFI_VEHICLE_SPEED
#define EFI_VEHICLE_SPEED FALSE

#undef EFI_BLUETOOTH_SETUP
#define EFI_BLUETOOTH_SETUP FALSE


#ifndef BOARD_TLE6240_COUNT
#define BOARD_TLE6240_COUNT         0
#endif

#ifndef BOARD_MC33972_COUNT
#define BOARD_MC33972_COUNT			0
#endif

#ifndef BOARD_TLE8888_COUNT
#define BOARD_TLE8888_COUNT 	1
#endif

#undef EFI_CAN_SUPPORT
#define EFI_CAN_SUPPORT TRUE

#undef EFI_AUX_SERIAL
#define EFI_AUX_SERIAL FALSE

#undef EFI_HD44780_LCD
#define EFI_HD44780_LCD FALSE

#undef EFI_LCD
#define EFI_LCD FALSE

/**
 * Do we need file logging (like SD card) logic?
 */
#undef EFI_FILE_LOGGING
#define EFI_FILE_LOGGING FALSE

#undef EFI_USB_SERIAL
#define EFI_USB_SERIAL TRUE

#undef EFI_UART_GPS
#define EFI_UART_GPS FALSE
#define EFI_CONSOLE_NO_THREAD TRUE
// todo: start using consoleUartDevice? Not sure
#undef EFI_CONSOLE_SERIAL_DEVICE

#undef EFI_CONSOLE_UART_DEVICE

// todo: our "DMA-half" ChibiOS patch not implemented for USARTv2/STM32F7
#undef TS_UART_DMA_MODE
#define TS_UART_DMA_MODE FALSE

#undef PRIMARY_UART_DMA_MODE
#define PRIMARY_UART_DMA_MODE FALSE

#undef TS_UART_DEVICE
//#define TS_UART_DEVICE (&UARTD3)

#undef TS_SERIAL_DEVICE
#define TS_SERIAL_DEVICE (&SD3)

#define AUX_SERIAL_DEVICE (&SD6)

// todo: add DMA-mode for Console?
#if (TS_UART_DMA_MODE || TS_UART_MODE)
#undef EFI_CONSOLE_SERIAL_DEVICE
#endif

// todo: start using consoleSerialTxPin? Not sure
#undef EFI_CONSOLE_TX_BRAIN_PIN
#define EFI_CONSOLE_TX_BRAIN_PIN GPIOD_8

// todo: start using consoleSerialRxPin? Not sure
#undef EFI_CONSOLE_RX_BRAIN_PIN
#define EFI_CONSOLE_RX_BRAIN_PIN GPIOD_9

// todo: temporary ignore errors, this is a test config
#define EFI_PRINT_ERRORS_AS_WARNINGS TRUE
