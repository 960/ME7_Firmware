/*
 * @file antilag.cpp
 *
 *  @date 10. sep. 2019
 *      (C) All rights reserved by RUUD BILELEKTRO, NORWAY
 */

#include "engine.h"

//#if EFI_ANTILAG

#if EFI_TUNER_STUDIO
#include "tunerstudio_outputs.h"
#endif /* EFI_TUNER_STUDIO */

#include "boost_control.h"
#include "vehicle_speed.h"
#include "antilag.h"
#include "io_pins.h"
#include "engine_configuration.h"
#include "engine_controller.h"
#include "periodic_task.h"
#include "pin_repository.h"
#include "allsensors.h"
#include "sensor.h"
#include "engine_math.h"
#include "efi_gpio.h"
#include "advance_map.h"
#include "engine_state.h"
#include "advance_map.h"


extern TunerStudioOutputChannels tsOutputChannels;

EXTERN_ENGINE;


static bool getAntilagSwitchCondition(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	switch (engineConfiguration->antiLag.antiLagActivationMode) {
	case SWITCH_INPUT_ANTILAG:
		if (CONFIG(antiLagActivatePin) != GPIO_UNASSIGNED) {
			engine->antiLagPinState = efiReadPin(CONFIG(antiLagActivatePin));
			}
			return engine->antiLagPinState;
	//case ALWAYS_ON_ANTILAG:
	default:
	return true;
	}
}


class AntiLag: public PeriodicTimerController {
	efitick_t antiLagTimerTimer;

	DECLARE_ENGINE_PTR;

	int getPeriodMs() override {
		return 50;
	}

	void PeriodicTask() override {
		if (!CONFIG(antiLagEnabled)) {
			return;
		}
		int rpm = GET_RPM_VALUE;
		auto tps = Sensor::get(SensorType::DriverThrottleIntent);
		auto clt = Sensor::get(SensorType::Clt);

		int antiLagRpm = engineConfiguration->antiLag.antiLagRpmTreshold;
		int antiLagTps = engineConfiguration->antiLag.antiLagTpsTreshold;
		int antiLagCoolant = engineConfiguration->antiLag.antiLagCoolantTreshold;
		float timeOut = engineConfiguration->antiLag.antilagTimeout;

        bool tpsCondition = (antiLagTps < tps.Value); //AL Stays active above this TPS
		bool rpmCondition = (antiLagRpm < rpm); //AL Stays active above this RPM
		bool switchCondition = getAntilagSwitchCondition(PASS_ENGINE_PARAMETER_SIGNATURE);
		bool coolantCondition = (antiLagCoolant < clt.Value);


		bool antiLagConditions = rpmCondition && switchCondition && tpsCondition && coolantCondition && (CONFIG(antiLagEnabled));
		//Antilag Deactivation Timeout routine
		engine->isAntilagCondition = false;
		if ((!antiLagConditions) && (!CONFIG(antiLagEnabled))) {
			antiLagTimerTimer = getTimeNowNt();
			engine->isAntilagCondition = false;
			} else {
			// If conditions are met...
			if ((getTimeNowNt() - antiLagTimerTimer < MS2NT(timeOut * 1000))) {
				engine->isAntilagCondition = true;           // ...enable antilag until timeout!
			}
		}
	}
};

static AntiLag Als;

void setDefaultAntiLagParameters(DECLARE_CONFIG_PARAMETER_SIGNATURE) {
	engineConfiguration->antiLag.antiLagTpsTreshold = 60;
	engineConfiguration->antiLag.antilagTimeout = 3;
	engineConfiguration->antiLag.antiLagRpmTreshold = 3000;
	engineConfiguration->antiLag.antiLagExtraFuel = 10;
}



void initAntiLag( DECLARE_ENGINE_PARAMETER_SUFFIX) {

	Als.Start();
}
//#endif

