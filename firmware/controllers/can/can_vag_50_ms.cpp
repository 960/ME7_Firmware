
/*
 ***************************************************************************************************
 *
 * (C) All rights reserved by RUUD BILELEKTRO, NORWAY
 *
 ***************************************************************************************************
 *
 * File: can_vag_50_ms.cpp
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


void can_VAG_50Ms(void) {

	chThdSleepMilliseconds(50);

	{
	CanTxMessage msg(CAN_VAG_IMMO);
	msg.setShortValue(0x80, 1);
	}

}
