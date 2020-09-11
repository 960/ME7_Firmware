
/*
 ***************************************************************************************************
 *
 * (C) All rights reserved by RUUD BILELEKTRO, NORWAY
 *
 ***************************************************************************************************
 *
 * File: can_vag_10_ms.cpp
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

void can_VAG_10Ms(void) {

	uint8_t myValues[4] = { 0x65, 0x8f, 0xE8, 0x0B };
	float clt = Sensor::get(SensorType::Clt).value_or(0);

for (int i = 0; i < 4; i++) {


	{  	  	  	  	  	  	  	  // All Torque fields = 0 - 99.06 %  (X * 2.54)
		//  Motor 1 ($280)
		CanTxMessage msg(VAG_MOTOR_1);  //10ms
		msg[0] = 0x08;
		msg[1] = (int) (Sensor::get(SensorType::DriverThrottleIntent).Value * 2.5);		// inneres Motor-Moment
		msg.setShortValue(GET_RPM() * 4, 2); //RPM Byte 2-3
		msg[4] = (int) (Sensor::get(SensorType::DriverThrottleIntent).Value	* 2.5);		// inneres Moment ohne externe Eingriffe
		msg[5] = (int) (Sensor::get(SensorType::DriverThrottleIntent).Value * 2.5);  // Throttle Pedal
		msg[6] = 0x00;     // mechanisches Motor-Verlustmoment
		msg[7] = (int) (Sensor::get(SensorType::DriverThrottleIntent).Value * 2.5);     // Drivers Wish Torque
	}

	{						//01100101, 10001111, 11101000, 00001011

		CanTxMessage msg(VAG_MOTOR_2);  //10ms
		msg[0] = myValues[i];
		msg.setShortValue((int) ((clt + 48.373) / 0.75), 1); // Coolant Temp
		msg[2] = 0x00;
		msg[4] = 0x00;// GRA target speed for CAN output (GRA = Cruise control?)
		msg[5] = 0x51; 		// Idle target rpm
		msg[6] = (int) (Sensor::get(SensorType::DriverThrottleIntent).Value * 2.5);   // max possible torque
		msg[7] = (int) (Sensor::get(SensorType::DriverThrottleIntent).Value * 2.5); // indexed engine torque at the latest ignition angle for CAN output
		}
		{

		CanTxMessage msg(VAG_MOTOR_6);   //10ms
		msg[0] = 0x00;
		msg[1] = (int) (Sensor::get(SensorType::DriverThrottleIntent).Value * 2.5);
		msg[2] = (int) (Sensor::get(SensorType::DriverThrottleIntent).Value * 2.5);
		msg[3] = 0x7C;
		msg[4] = 0xA6;
		msg[5] = 0x00;
		msg[6] = 0x00;
		msg[7] = 0x00;
	}
  }
}
