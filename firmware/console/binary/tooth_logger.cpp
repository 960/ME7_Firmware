/*
 * @file tooth_logger.cpp
 *
 * @date Jul 7, 2019
 * @author Matthew Kennedy
 */

#include "tooth_logger.h"
#include "engine.h"
#include "global.h"

#if EFI_TOOTH_LOGGER

#include "tunerstudio_io.h"

#include <cstddef>
#include "efitime.h"
#include "efilib.h"
#include "tunerstudio_outputs.h"

EXTERN_ENGINE;

typedef struct __attribute__ ((packed)) {
    uint32_t timestamp;
} tooth_logger_s;

typedef struct __attribute__ ((packed)) {
	uint32_t timestamp;
	bool priLevel : 1;
	bool secLevel : 1;
	bool trigger : 1;
	bool sync : 1;
} composite_logger_s;


uint8_t compositeLogHistory[TOOTH_PACKET_COUNT];

static composite_logger_s buffer[COMPOSITE_PACKET_COUNT] CCM_OPTIONAL;
static tooth_logger_s toothHistory[COMPOSITE_PACKET_COUNT] CCM_OPTIONAL;

static size_t NextIdx = 0;
static volatile bool ToothLoggerEnabled = false;

static uint32_t lastEdgeTimestamp = 0;

static bool trigger1;
static bool trigger2;
static bool edge;
static bool sync;


int getCompositeRecordCount() {
	return NextIdx;
}

bool validEdge = false;
bool valueLogged = false;
bool validCrank = false;
bool validCam = false;
bool validVvt = false;
bool validSync = false;

uint32_t lasttoothtime;
uint32_t lastcranktime;
uint32_t lastcamtime;
uint32_t thistoothtime;



static inline void addToothLogEntry(uint32_t lasttoothtime, bool whichtooth, bool edge) {
	if((engine->toothLogEnabled == true) || (engine->compositeLogEnabled == true))
	  {
	    bool valueLogged = false;
	if (engine->toothLogEnabled == true) {
		toothHistory[NextIdx].timestamp = SWAP_UINT32(lasttoothtime);
		valueLogged = true;
	}

	else if (engine->compositeLogEnabled == true)
	{

	buffer[NextIdx].timestamp = SWAP_UINT32(lasttoothtime);
	if (whichtooth == trigger1) {
	buffer[NextIdx].priLevel = whichtooth;
	}
	if (whichtooth == trigger2) {
	buffer[NextIdx].secLevel = whichtooth;
	}
	buffer[NextIdx].trigger = edge;
	buffer[NextIdx].sync = engine->triggerCentral.triggerState.shaft_is_synchronized;

	uint32_t gap2 = getTimeNowUs() - thistoothtime;
	toothHistory[NextIdx].timestamp = SWAP_UINT32(gap2);
	thistoothtime = getTimeNowUs();
	valueLogged = true;
	}
	if (valueLogged == true) {
		NextIdx++;
	}
	static_assert(sizeof(composite_logger_s) == COMPOSITE_PACKET_SIZE, "composite packet size");

	if (NextIdx >= COMPOSITE_PACKET_COUNT - 1) {
		NextIdx = 0;
	}
}
}


void LogTriggerTooth(trigger_event_e tooth, efitick_t timestamp DECLARE_ENGINE_PARAMETER_SUFFIX) {
	// bail if we aren't enabled
	if (!ToothLoggerEnabled) {
		return;
	}
	switch (tooth) {
	case SHAFT_PRIMARY_FALLING:
		trigger1 = false;
		edge = false;
		validCrank = true;
		break;
	case SHAFT_PRIMARY_RISING:
		trigger1 = true;
		edge = false;
		validCrank = true;
		break;
	default:
		break;
	}

	if (engine->toothLogEnabled) {
			auto gap = getTimeNowUs() - lastcranktime;
			addToothLogEntry(gap, trigger1, false);
			lastcranktime = getTimeNowUs();
	}

	if (engine->compositeLogEnabled) {
		lastEdgeTimestamp = getTimeNowUs();
		addToothLogEntry(lastEdgeTimestamp, trigger1, edge);

	}

}

void LogCamTooth(trigger_value_e value DECLARE_ENGINE_PARAMETER_SUFFIX) {
	// bail if we aren't enabled
	if (!ToothLoggerEnabled) {
		return;
	}
	switch (value) {
	case TV_FALL:
		trigger2 = false;
		edge = true;
		validCam = true;
		break;
	case TV_RISE:
		trigger2 = true;
		edge = true;
		validCam = true;
		break;
	default:
		break;
	}

	if (engine->compositeLogEnabled) {
		lastEdgeTimestamp = getTimeNowUs();
		addToothLogEntry(lastEdgeTimestamp, trigger2, edge);
		}

}



void LogTriggerTopDeadCenter(efitick_t timestamp DECLARE_ENGINE_PARAMETER_SUFFIX) {
	// bail if we aren't enabled
	if (!ToothLoggerEnabled) {
		return;
	}

}


void EnableToothLogger() {
	// Clear the buffer
	memset(buffer, 0, sizeof(buffer));
	memset(toothHistory, 0, sizeof(toothHistory));
	// Reset the last edge to now - this prevents the first edge logged from being bogus
	lastEdgeTimestamp = getTimeNowUs();
	thistoothtime = getTimeNowUs();
	// Reset write index
	NextIdx = 0;

	// Enable logging of edges as they come
	ToothLoggerEnabled = true;

	// Tell TS that we're ready for it to read out the log
	// nb: this is a lie, as we may not have written anything
	// yet.  However, we can let it continuously read out the buffer
	// as we update it, which looks pretty nice.
	tsOutputChannels.toothLogReady = true;
}

void EnableToothLoggerIfNotEnabled() {
	if (!ToothLoggerEnabled) {
		EnableToothLogger();
	}
}

void DisableToothLogger() {
	ToothLoggerEnabled = false;
	tsOutputChannels.toothLogReady = false;
}
ToothLoggerBuffer GetCompositeLoggerBuffer() {
	return { reinterpret_cast<uint8_t*>(buffer), sizeof(buffer) };
	return { reinterpret_cast<uint8_t*>(toothHistory), sizeof(toothHistory) };
}
ToothLoggerBuffer GetToothLoggerBuffer() {
	return { reinterpret_cast<uint8_t*>(toothHistory), sizeof(toothHistory) };
}

#endif /* EFI_TOOTH_LOGGER */
