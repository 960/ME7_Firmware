/**
 * @file	tunerstudio.cpp
 * @brief	Binary protocol implementation
 *
 * This implementation would not happen without the documentation
 * provided by Jon Zeeff (jon@zeeff.com)
 *
 *
 * @brief Integration with EFI Analytics Tuner Studio software
 *
 * Tuner Studio has a really simple protocol, a minimal implementation
 * capable of displaying current engine state on the gauges would
 * require only two commands: queryCommand and ochGetCommand
 *
 * queryCommand:
 * 		Communication initialization command. TunerStudio sends a single byte H
 * 		ECU response:
 * 			One of the known ECU id strings.
 *
 * ochGetCommand:
 * 		Request for output channels state.TunerStudio sends a single byte O
 * 		ECU response:
 * 			A snapshot of output channels as described in [OutputChannels] section of the .ini file
 * 			The length of this block is 'ochBlockSize' property of the .ini file
 *
 * These two commands are enough to get working gauges. In order to start configuring the ECU using
 * tuner studio, three more commands should be implemented:
 *
 *
 * @date Oct 22, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 *
 * This file is part of rusEfi - see http://rusefi.com
 *
 * rusEfi is free software; you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * rusEfi is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * This file is part of rusEfi - see http://rusefi.com
 *
 * rusEfi is free software; you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * rusEfi is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "global.h"
#include "os_access.h"
#include "allsensors.h"
#include "tunerstudio.h"
#include "main_trigger_callback.h"
#include "flash_main.h"
#include "tunerstudio_io.h"
#include "tunerstudio_outputs.h"
#include "malfunction_central.h"
#include "console_io.h"
#include "crc.h"
#include "tunerstudio_io.h"
#include "tooth_logger.h"
#include "electronic_throttle.h"
#include <string.h>
#include "engine_configuration.h"
#include "bench_test.h"
#include "fram.h"
#include "status_loop.h"
#include "perf_trace.h"

#if EFI_TUNER_STUDIO



EXTERN_ENGINE;

extern persistent_config_container_s persistentState;

/**
 * note the use-case where text console port is switched into
 * binary port
 */

#if !defined(EFI_NO_CONFIG_WORKING_COPY)
/**
 * this is a local copy of the configuration. Any changes to this copy
 * have no effect until this copy is explicitly propagated to the main working copy
 */
persistent_config_s configWorkingCopy;

#endif /* EFI_NO_CONFIG_WORKING_COPY */

static efitimems_t previousWriteReportMs = 0;

static ts_channel_s tsChannel;

// this thread wants a bit extra stack
static THD_WORKING_AREA(tunerstudioThreadStack, CONNECTIVITY_THREAD_STACK);

static void resetTs(void) {
	memset(&tsState, 0, sizeof(tsState));
}

static void printErrorCounters(void) {
}

void printTsStats(void) {
	printErrorCounters();
}


char *getWorkingPageAddr() {
#ifndef EFI_NO_CONFIG_WORKING_COPY
	return (char*) &configWorkingCopy.engineConfiguration;
#else
	return (char*) engineConfiguration;
#endif /* EFI_NO_CONFIG_WORKING_COPY */
}

static constexpr size_t getTunerStudioPageSize() {
	return TOTAL_CONFIG_SIZE;
}

void sendOkResponse(ts_channel_s *tsChannel, ts_response_format_e mode) {
	sr5SendResponse(tsChannel, mode, NULL, 0);
}

static void sendErrorCode(ts_channel_s *tsChannel, uint8_t code) {
	sr5WriteCrcPacket(tsChannel, code, NULL, 0);
}

static void handlePageSelectCommand(ts_channel_s *tsChannel, ts_response_format_e mode, uint16_t pageId) {
	tsState.pageCommandCounter++;

	sendOkResponse(tsChannel, mode);
}

static void onlineApplyWorkingCopyBytes(uint32_t offset, int count) {
	if (offset >= sizeof(engine_configuration_s)) {
		int maxSize = sizeof(persistent_config_s) - offset;
		if (count > maxSize) {
			warning(CUSTOM_TS_OVERFLOW, "TS overflow %d %d", offset, count);
			return;
		}
#if !defined(EFI_NO_CONFIG_WORKING_COPY)
		memcpy(((char*) &persistentState.persistentConfiguration) + offset, ((char*) &configWorkingCopy) + offset,
				count);
#endif /* EFI_NO_CONFIG_WORKING_COPY */
	}
}

static const void * getStructAddr(int structId) {
	switch (structId) {
	case LDS_ENGINE_STATE_INDEX:
		return static_cast<engine_state2_s*>(&engine->engineState);
	case LDS_FUEL_TRIM_STATE_INDEX:
		return static_cast<wall_fuel_state*>(&engine->injectionEvents.elements[0].wallFuel);
	case LDS_TRIGGER_CENTRAL_STATE_INDEX:
		return static_cast<trigger_central_s*>(&engine->triggerCentral);
	case LDS_TRIGGER_STATE_STATE_INDEX:
		return static_cast<trigger_state_s*>(&engine->triggerCentral.triggerState);
#if EFI_ELECTRONIC_THROTTLE_BODY
	case LDS_ETB_PID_STATE_INDEX:
		return static_cast<EtbController*>(engine->etbControllers[0])->getPidState();
#endif /* EFI_ELECTRONIC_THROTTLE_BODY */

#ifndef EFI_IDLE_CONTROL
	case LDS_IDLE_PID_STATE_INDEX:
		return static_cast<pid_state_s*>(getIdlePid());
#endif /* EFI_IDLE_CONTROL */

	default:
		return NULL;
	}
}

static void handleGetStructContent(ts_channel_s *tsChannel, int structId, int size) {
	tsState.readPageCommandsCounter++;

	const void *addr = getStructAddr(structId);
	if (addr == nullptr) {
		// todo: add warning code - unexpected structId
		return;
	}
	sr5SendResponse(tsChannel, TS_CRC, (const uint8_t *)addr, size);
}

static bool validateOffsetCount(size_t offset, size_t count, ts_channel_s *tsChannel) {
	if (offset + count > getTunerStudioPageSize()) {


		sendErrorCode(tsChannel, TS_RESPONSE_OUT_OF_RANGE);
		return true;
	}

	return false;
}

static void handleWriteChunkCommand(ts_channel_s *tsChannel, ts_response_format_e mode, uint16_t offset, uint16_t count,
		void *content) {
	tsState.writeChunkCommandCounter++;

	if (validateOffsetCount(offset, count, tsChannel)) {
		return;
	}

	uint8_t * addr = (uint8_t *) (getWorkingPageAddr() + offset);
	memcpy(addr, content, count);
	onlineApplyWorkingCopyBytes(offset, count);

	sendOkResponse(tsChannel, mode);
}

static void handleCrc32Check(ts_channel_s *tsChannel, ts_response_format_e mode, uint16_t pageId) {
	UNUSED(pageId);
	tsState.crc32CheckCommandCounter++;
	uint16_t count = getTunerStudioPageSize();
	uint32_t crc = SWAP_UINT32(crc32((void * ) getWorkingPageAddr(), count));
	sr5SendResponse(tsChannel, mode, (const uint8_t *) &crc, 4);
}

static void handleWriteValueCommand(ts_channel_s *tsChannel, ts_response_format_e mode, uint16_t page, uint16_t offset,
		uint8_t value) {
	UNUSED(tsChannel);
	UNUSED(mode);
	UNUSED(page);

	tsState.writeValueCommandCounter++;

	if (validateOffsetCount(offset, 1, tsChannel)) {
		return;
	}

	efitimems_t nowMs = currentTimeMillis();
	if (nowMs - previousWriteReportMs > 5) {
		previousWriteReportMs = nowMs;

	}

	getWorkingPageAddr()[offset] = value;

	onlineApplyWorkingCopyBytes(offset, 1);

}

static void handlePageReadCommand(ts_channel_s *tsChannel, ts_response_format_e mode, uint16_t pageId, uint16_t offset,
		uint16_t count) {
	tsState.readPageCommandsCounter++;
	if (validateOffsetCount(offset, count, tsChannel)) {
		return;
	}

	const uint8_t *addr = (const uint8_t *) (getWorkingPageAddr() + offset);
	sr5SendResponse(tsChannel, mode, addr, count);

}

void requestBurn(void) {
#if EFI_INTERNAL_FLASH
#if EFI_SPI_FRAM
	writeToFlashNow();
#else
	setNeedToWriteConfiguration();
#endif
#endif
	incrementGlobalConfigurationVersion(PASS_ENGINE_PARAMETER_SIGNATURE);
}

static void sendResponseCode(ts_response_format_e mode, ts_channel_s *tsChannel, const uint8_t responseCode) {
	if (mode == TS_CRC) {
		sr5WriteCrcPacket(tsChannel, responseCode, NULL, 0);
	}
}


static void handleBurnCommand(ts_channel_s *tsChannel, ts_response_format_e mode) {
	tsState.burnCommandCounter++;
#if !defined(EFI_NO_CONFIG_WORKING_COPY)
	memcpy(&persistentState.persistentConfiguration, &configWorkingCopy, sizeof(persistent_config_s));
#endif /* EFI_NO_CONFIG_WORKING_COPY */
	requestBurn();
	sendResponseCode(mode, tsChannel, TS_RESPONSE_BURN_OK);
}

static bool isKnownCommand(char command) {
	return command == TS_HELLO_COMMAND || command == TS_READ_COMMAND || command == TS_OUTPUT_COMMAND
			|| command == TS_PAGE_COMMAND || command == TS_BURN_COMMAND || command == TS_SINGLE_WRITE_COMMAND
			|| command == TS_CHUNK_WRITE_COMMAND
			|| command == TS_EXECUTE
			|| command == TS_IO_TEST_COMMAND
			|| command == TS_GET_STRUCT
			|| command == TS_SET_LOGGER_SWITCH
			|| command == TS_GET_LOGGER_GET_BUFFER
			|| command == TS_GET_COMPOSITE_BUFFER_DONE_DIFFERENTLY
			|| command == TS_GET_TEXT
			|| command == TS_CRC_CHECK_COMMAND
			|| command == TS_GET_FIRMWARE_VERSION
			|| command == TS_PERF_TRACE_BEGIN
			|| command == TS_PERF_TRACE_GET_BUFFER
			|| command == TS_GET_CONFIG_ERROR;
}

// this function runs indefinitely
void runBinaryProtocolLoop(ts_channel_s *tsChannel) {
	int wasReady = false;

	while (true) {
		int isReady = sr5IsReady(tsChannel);
		if (!isReady) {
			chThdSleepMilliseconds(10);
			wasReady = false;
			continue;
		}

		if (!wasReady) {
			wasReady = true;
		}

		tsState.totalCounter++;
		uint8_t firstByte;
		int received = sr5ReadData(tsChannel, &firstByte, 1);

		if (received != 1) {
			continue;
		}
		onDataArrived();

		if (handlePlainCommand(tsChannel, firstByte))
			continue;

		uint8_t secondByte;
		received = sr5ReadData(tsChannel, &secondByte, 1);
		if (received != 1) {
			continue;
		}

		uint16_t incomingPacketSize = firstByte << 8 | secondByte;

		if (incomingPacketSize == 0 || incomingPacketSize > (sizeof(tsChannel->crcReadBuffer) - CRC_WRAPPING_SIZE)) {
	
			sendErrorCode(tsChannel, TS_RESPONSE_UNDERRUN);
			continue;
		}

		received = sr5ReadData(tsChannel, (uint8_t* )tsChannel->crcReadBuffer, 1);
		if (received != 1) {
			continue;
		}

		char command = tsChannel->crcReadBuffer[0];
		if (!isKnownCommand(command)) {
		
			sendErrorCode(tsChannel, TS_RESPONSE_UNRECOGNIZED_COMMAND);
			continue;
		}

		received = sr5ReadData(tsChannel, (uint8_t * ) (tsChannel->crcReadBuffer + 1),
				incomingPacketSize + CRC_VALUE_SIZE - 1);
		int expectedSize = incomingPacketSize + CRC_VALUE_SIZE - 1;
		if (received != expectedSize) {
						
			sendErrorCode(tsChannel, TS_RESPONSE_UNDERRUN);
		}

		uint32_t expectedCrc = *(uint32_t*) (tsChannel->crcReadBuffer + incomingPacketSize);

		expectedCrc = SWAP_UINT32(expectedCrc);

		uint32_t actualCrc = crc32(tsChannel->crcReadBuffer, incomingPacketSize);
		if (actualCrc != expectedCrc) {
			sendErrorCode(tsChannel, TS_RESPONSE_CRC_FAILURE);
			continue;
		}

		int success = tunerStudioHandleCrcCommand(tsChannel, tsChannel->crcReadBuffer, incomingPacketSize);
		if (!success)
			break;

	}
}

static THD_FUNCTION(tsThreadEntryPoint, arg) {
	(void) arg;
	chRegSetThreadName("tunerstudio thread");

	startTsPort(&tsChannel);

	runBinaryProtocolLoop(&tsChannel);
}

void syncTunerStudioCopy(void) {
#if !defined(EFI_NO_CONFIG_WORKING_COPY)
	memcpy(&configWorkingCopy, &persistentState.persistentConfiguration, sizeof(persistent_config_s));
#endif /* EFI_NO_CONFIG_WORKING_COPY */
}

tunerstudio_counters_s tsState;
TunerStudioOutputChannels tsOutputChannels;

void tunerStudioError() {
	printErrorCounters();
	tsState.errorCounter++;
}

void handleQueryCommand(ts_channel_s *tsChannel, ts_response_format_e mode) {
	tsState.queryCommandCounter++;
	sr5SendResponse(tsChannel, mode, (const uint8_t *) TS_SIGNATURE, strlen(TS_SIGNATURE) + 1);
}

static void handleOutputChannelsCommand(ts_channel_s *tsChannel, ts_response_format_e mode, uint16_t offset, uint16_t count) {
	if (offset + count > sizeof(TunerStudioOutputChannels)) {
		
		sendErrorCode(tsChannel, TS_RESPONSE_OUT_OF_RANGE);
		return;
	}

	tsState.outputChannelsCommandCounter++;
	prepareTunerStudioOutputs();
	// this method is invoked too often to print any debug information
	sr5SendResponse(tsChannel, mode, ((const uint8_t *) &tsOutputChannels) + offset, count);
}

static void handleGetVersion(ts_channel_s *tsChannel, ts_response_format_e mode) {
	static char versionBuffer[32];
	chsnprintf(versionBuffer, sizeof(versionBuffer), "Ruud Bilelektro", getRusEfiVersion(), 123);
	sr5SendResponse(tsChannel, mode, (const uint8_t *) versionBuffer, strlen(versionBuffer) + 1);
}

static void handleExecuteCommand(ts_channel_s *tsChannel, char *data, int incomingPacketSize) {
	sr5WriteCrcPacket(tsChannel, TS_RESPONSE_COMMAND_OK, NULL, 0);
	data[incomingPacketSize] = 0;

}

bool handlePlainCommand(ts_channel_s *tsChannel, uint8_t command) {
	// Bail fast if guaranteed not to be a plain command
	if (command == 0)
	{
		return false;
	}
	else if (command == TS_HELLO_COMMAND) {
		handleQueryCommand(tsChannel, TS_PLAIN);
		return true;
	} else if (command == TS_COMMAND_F) {
		sr5WriteData(tsChannel, (const uint8_t *) TS_PROTOCOL, strlen(TS_PROTOCOL));
		return true;
	} else {
		return false;
	}
}

static int transmitted = 0;


int tunerStudioHandleCrcCommand(ts_channel_s *tsChannel, char *data, int incomingPacketSize) {
	ScopePerf perf(PE::TunerStudioHandleCrcCommand);

	char command = data[0];
	data++;

	const uint16_t* data16 = reinterpret_cast<uint16_t*>(data);

	uint16_t offset = data16[0];
	uint16_t count = data16[1];

	switch(command)
	{
	case TS_OUTPUT_COMMAND:
		handleOutputChannelsCommand(tsChannel, TS_CRC, data16[0], data16[1]);
		break;
	case TS_HELLO_COMMAND:
		handleQueryCommand(tsChannel, TS_CRC);
		break;
	case TS_GET_FIRMWARE_VERSION:
		handleGetVersion(tsChannel, TS_CRC);
		break;
	case TS_EXECUTE:
		handleExecuteCommand(tsChannel, data, incomingPacketSize - 1);
		break;
	case TS_PAGE_COMMAND:
		handlePageSelectCommand(tsChannel, TS_CRC, data16[0]);
		break;
	case TS_GET_STRUCT:
		handleGetStructContent(tsChannel, data16[0], data16[1]);
		break;
	case TS_CHUNK_WRITE_COMMAND:
		handleWriteChunkCommand(tsChannel, TS_CRC, data16[1], data16[2], data + sizeof(TunerStudioWriteChunkRequest));
		break;
	case TS_SINGLE_WRITE_COMMAND:
		{
			uint8_t value = data[4];
			handleWriteValueCommand(tsChannel, TS_CRC, data16[0], data16[1], value);
		}
		break;
	case TS_CRC_CHECK_COMMAND:
		handleCrc32Check(tsChannel, TS_CRC, data16[0]);
		break;
	case TS_BURN_COMMAND:
		handleBurnCommand(tsChannel, TS_CRC);
		break;
	case TS_READ_COMMAND:
		handlePageReadCommand(tsChannel, TS_CRC, data16[0], data16[1], data16[2]);
		break;
	case TS_IO_TEST_COMMAND:
		{
			uint16_t subsystem = SWAP_UINT16(data16[0]);
			uint16_t index = SWAP_UINT16(data16[1]);

			if (engineConfiguration->debugMode == DBG_BENCH_TEST) {
				tsOutputChannels.debugIntField1++;
				tsOutputChannels.debugIntField2 = subsystem;
				tsOutputChannels.debugIntField3 = index;
			}


		executeTSCommand(subsystem, index);

			sendOkResponse(tsChannel, TS_CRC);
		}
		break;
#if EFI_TOOTH_LOGGER
	case TS_SET_LOGGER_SWITCH:
		switch(data[0]) {
		case TS_COMPOSITE_ENABLE:
			EnableToothLogger();
			break;
		case TS_COMPOSITE_DISABLE:
			DisableToothLogger();
			break;
		default:
			return false;
		}
		sendOkResponse(tsChannel, TS_CRC);
		break;
		case TS_GET_COMPOSITE_BUFFER_DONE_DIFFERENTLY:

		{
			EnableToothLoggerIfNotEnabled();
			const uint8_t* const buffer = GetToothLoggerBuffer().Buffer;

			const uint8_t* const start = buffer + COMPOSITE_PACKET_SIZE * transmitted;

			int currentEnd = getCompositeRecordCount();

			// set debug_mode 40
			if (engineConfiguration->debugMode == DBG_COMPOSITE_LOG) {
				tsOutputChannels.debugIntField1 = currentEnd;
				tsOutputChannels.debugIntField2 = transmitted;

			}

			if (currentEnd > transmitted) {
				// more normal case - tail after head
				sr5SendResponse(tsChannel, TS_CRC, start, COMPOSITE_PACKET_SIZE * (currentEnd - transmitted));
				transmitted = currentEnd;
			} else if (currentEnd == transmitted) {
				sr5SendResponse(tsChannel, TS_CRC, start, 0);
			} else {
				// we are here if tail of buffer has reached the end of buffer and re-started from the start of buffer
				// sending end of the buffer, next transmission would take care of the rest
				sr5SendResponse(tsChannel, TS_CRC, start, COMPOSITE_PACKET_SIZE * (COMPOSITE_PACKET_COUNT - transmitted));
				transmitted = 0;
			}
		}
		break;
	case TS_GET_LOGGER_GET_BUFFER:
		{
			auto toothBuffer = GetToothLoggerBuffer();
			sr5SendResponse(tsChannel, TS_CRC, toothBuffer.Buffer, toothBuffer.Length);
		}

		break;
#endif /* EFI_TOOTH_LOGGER */
	case TS_GET_CONFIG_ERROR: {
		char * configError = getFirmwareError();
		sr5SendResponse(tsChannel, TS_CRC, reinterpret_cast<const uint8_t*>(configError), strlen(configError));
		break;
	}
	default:
		return false;
	}
	return true;
}

void startTunerStudioConnectivity(void) {
	// Assert tune & output channel struct sizes
	static_assert(sizeof(persistent_config_s) == TOTAL_CONFIG_SIZE, "TS datapage size mismatch");
	static_assert(sizeof(TunerStudioOutputChannels) == TS_OUTPUT_SIZE, "TS output channels size mismatch");

	memset(&tsState, 0, sizeof(tsState));
	syncTunerStudioCopy();

	
	chThdCreateStatic(tunerstudioThreadStack, sizeof(tunerstudioThreadStack), NORMALPRIO, (tfunc_t)tsThreadEntryPoint, NULL);
}

#endif
