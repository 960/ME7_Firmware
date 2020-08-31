/**
 * @file	tunerstudio_io.h
 *
 * @date Mar 8, 2015
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once
#include "global.h"

#if EFI_PROD_CODE
#include "usbconsole.h"
#include "pin_repository.h"
#endif

#define PROTOCOL  "001"

#define PROTOCOL  "001"

#define TS_RESPONSE_OK 0x00
#define TS_RESPONSE_BURN_OK 0x04
#define TS_RESPONSE_COMMAND_OK 0x07

#define TS_RESPONSE_UNDERRUN 0x80
#define TS_RESPONSE_OVERRUN 0x81
#define TS_RESPONSE_CRC_FAILURE 0x82
#define TS_RESPONSE_UNRECOGNIZED_COMMAND 0x83
#define TS_RESPONSE_OUT_OF_RANGE 0x84
#define TS_RESPONSE_BUSY 0x85
#define TS_RESPONSE_FLASH_LOCKED 0x86
#define TS_RESPONSE_SEQUENCE_FAILURE 0x87
#define TS_RESPONSE_FRAMING_ERROR 0x8d

typedef enum {
	TS_PLAIN = 0,
	TS_CRC = 1
} ts_response_format_e;

struct ts_channel_s {
	BaseChannel * channel = nullptr;
	uint8_t writeBuffer[7];	// size(2 bytes) + response(1 byte) + crc32 (4 bytes)
	/**
	 * See 'blockingFactor' in rusefi.ini
	 */
	char crcReadBuffer[BLOCKING_FACTOR + 30];

#if TS_UART_DMA_MODE || PRIMARY_UART_DMA_MODE || TS_UART_MODE
	UARTDriver *uartp = nullptr;
#endif // TS_UART_DMA_MODE
};
#define TS_PROTOCOL "001"
#define TS_OUTPUT_COMMAND 'O'
#define TS_HELLO_COMMAND 'S'
#define TS_CRC_CHECK_COMMAND 'k'
#define TS_GET_COMPOSITE_BUFFER_DONE_DIFFERENTLY '8'

#define TS_COMPOSITE_ENABLE 1
#define TS_COMPOSITE_DISABLE 2
#define TS_GET_FIRMWARE_VERSION 'V'
#define TS_GET_CONFIG_ERROR 'e'
#define TS_SET_LOGGER_SWITCH   'l'
#define TS_GET_LOGGER_GET_BUFFER 'L'
#define TS_OUTPUT_COMMAND 'O' // 0x4F ochGetCommand
#define TS_READ_COMMAND 'R' // 0x52
#define TS_PAGE_COMMAND 'P' // 0x50
#define TS_COMMAND_F 'F' // 0x46

#define TS_GET_CONFIG_ERROR 'e' // returns getFirmwareError()

// High speed logger commands
#define TS_SET_LOGGER_MODE   'l'
#define TS_GET_LOGGER_BUFFER 'L'

// Performance tracing
#define TS_PERF_TRACE_BEGIN 'r'
#define TS_PERF_TRACE_GET_BUFFER 'b'

#define TS_SINGLE_WRITE_COMMAND 'W' // 0x57 pageValueWrite
#define TS_CHUNK_WRITE_COMMAND 'C' // 0x43 pageChunkWrite
#define TS_BURN_COMMAND 'B' // 0x42 burnCommand
#define TS_IO_TEST_COMMAND 'w' // 0x77
// 0x45
#define TS_EXECUTE 'E'
// 0x39
#define TS_GET_STRUCT '9'
// 0x47
#define TS_GET_TEXT 'G'
#define TS_CRC_CHECK_COMMAND 'k' // 0x6B

#define CRC_VALUE_SIZE 4
// todo: double-check this
#define CRC_WRAPPING_SIZE (CRC_VALUE_SIZE + 3)

#if HAL_USE_SERIAL_USB
#define CONSOLE_USB_DEVICE SDU1
#endif /* HAL_USE_SERIAL_USB */

void startTsPort(ts_channel_s *tsChannel);
bool stopTsPort(ts_channel_s *tsChannel);

// that's 1 second
#define BINARY_IO_TIMEOUT TIME_MS2I(1000)

// that's 1 second
#define SR5_READ_TIMEOUT TIME_MS2I(1000)

void sr5WriteData(ts_channel_s *tsChannel, const uint8_t * buffer, int size);
void sr5WriteCrcPacket(ts_channel_s *tsChannel, const uint8_t responseCode, const void *buf, const uint16_t size);
void sr5SendResponse(ts_channel_s *tsChannel, ts_response_format_e mode, const uint8_t * buffer, int size);
void sendOkResponse(ts_channel_s *tsChannel, ts_response_format_e mode);
int sr5ReadData(ts_channel_s *tsChannel, uint8_t * buffer, int size);
int sr5ReadDataTimeout(ts_channel_s *tsChannel, uint8_t * buffer, int size, int timeout);
bool sr5IsReady(ts_channel_s *tsChannel);

