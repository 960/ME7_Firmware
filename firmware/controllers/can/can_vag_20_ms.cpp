
/*
 ***************************************************************************************************
 *
 * (C) All rights reserved by RUUD BILELEKTRO, NORWAY
 *
 ***************************************************************************************************
 *
 * File: can_vag_20_ms.cpp
 */
#include "globalaccess.h"

#include "engine.h"
#include "can_dash.h"
#include "can_msg_tx.h"
#include "can_vag.h"
#include "sensor.h"
#include "allsensors.h"
#include "vehicle_speed.h"
#include "rtc_helper.h"

EXTERN_ENGINE;

void can_VAG_20Ms(void) {

	chThdSleepMilliseconds(20);
	{
	CanTxMessage msg(VAG_MOTOR_7);
	msg[0] = 0x08;
	msg[1] = 0x00;
	msg[2] = 0x7C;
	}
}


