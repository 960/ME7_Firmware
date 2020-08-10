/*
 * @file launch_control.cpp
 *
 *  @date 10. sep. 2019
 *      (C) All rights reserved by RUUD BILELEKTRO, NORWAY
 */

#include "engine.h"

#if EFI_LAUNCH_CONTROL

#if EFI_TUNER_STUDIO
#include "tunerstudio_outputs.h"
#endif /* EFI_TUNER_STUDIO */

#include "boost_control.h"
#include "vehicle_speed.h"
#include "launch_control.h"
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


static bool getActivateSwitchCondition(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	switch (engineConfiguration->launch.launchActivationMode) {
	case SWITCH_INPUT_LAUNCH:
		if (CONFIG(launchActivatePin) != GPIO_UNASSIGNED) {
			engine->launchActivatePinState = efiReadPin(CONFIG(launchActivatePin));
		}
		return engine->launchActivatePinState;

	case CLUTCH_INPUT_LAUNCH:
		if (CONFIG(clutchDownPin) != GPIO_UNASSIGNED) {
			engine->clutchDownState = efiReadPin(CONFIG(clutchDownPin));
		}
		return engine->clutchDownState;
	default:
		// ALWAYS_ACTIVE_LAUNCH
		return true;
	}
}

class LaunchControl : public PeriodicTimerController{
    efitick_t launchTimer;
    int getPeriodMs() override{
        return 50;
    }
    void PeriodicTask() override{
    	if (!CONFIG(launchControlEnabled)) {
    				return;
    			}

        int rpm = GET_RPM_VALUE;
        auto tps = Sensor::get(SensorType::DriverThrottleIntent);
        int tpstreshold = (CONFIG(launch.launchTpsTreshold));
        float timeDelay = (CONFIG(launch.launchActivateDelay));
        int launchRpm = (CONFIG(launch.launchRpm));
        engine->activateSwitchCondition = getActivateSwitchCondition(PASS_ENGINE_PARAMETER_SIGNATURE);

        if (CONFIG(invertLaunchSwitch)) {
        	 engine->activateSwitchCondition = !getActivateSwitchCondition(PASS_ENGINE_PARAMETER_SIGNATURE);
        }


        engine->rpmCondition = (rpm > launchRpm);
        engine->tpsCondition = (tps.Value > tpstreshold);
        bool speedCondition = (getVehicleSpeed() < CONFIG(launch.launchSpeedTreshold)) || !engineConfiguration->launchDisableBySpeed;
        bool combinedConditions = speedCondition && engine->activateSwitchCondition && engine->rpmCondition && engine->tpsCondition && (CONFIG(launchControlEnabled));
        if (engineConfiguration->debugMode == DBG_LAUNCH){

#if EFI_TUNER_STUDIO
            tsOutputChannels.debugIntField1 = engine->rpmCondition;
            tsOutputChannels.debugIntField2 = engine->tpsCondition;
            tsOutputChannels.debugIntField3 = speedCondition;
            tsOutputChannels.debugIntField4 = engine->activateSwitchCondition;
            tsOutputChannels.debugIntField5 = combinedConditions;
            tsOutputChannels.debugFloatField1 = engine->launchActivatePinState;
            tsOutputChannels.debugFloatField2 = engine->isLaunchCondition;
            tsOutputChannels.debugFloatField3 = engine->clutchDownState;
            tsOutputChannels.debugFloatField4 = engineConfiguration->launchControlEnabled;
#endif /* EFI_TUNER_STUDIO */
        }
        engine->isLaunchCondition = false;

   if (CONFIG(launchControlEnabled)) {
        if(combinedConditions){
            if(!engineConfiguration->useLaunchActivateDelay){
                engine->isLaunchCondition = true;
            } else if(getTimeNowNt() - launchTimer > MS2NT(timeDelay * 1000)){
                engine->isLaunchCondition = true; // ...enable launch!
            }
        } else {
            launchTimer = getTimeNowNt();
        }

        if (engineConfiguration->debugMode == DBG_LAUNCH){
#if EFI_TUNER_STUDIO
            tsOutputChannels.debugFloatField2 = engine->isLaunchCondition * 10;
            tsOutputChannels.debugFloatField3 = combinedConditions * 10;
#endif /* EFI_TUNER_STUDIO */
        }
    }
   }
};



static LaunchControl Launch;

void setDefaultLaunchParameters(DECLARE_CONFIG_PARAMETER_SIGNATURE) {
	engineConfiguration->launch.launchRpm = 4000;    // Rpm to trigger Launch condition
	engineConfiguration->launch.launchTimingRetard = 10; // retard in absolute degrees ATDC
	engineConfiguration->launch.launchAdvanceRpmRange = 500; // Rpm above Launch triggered for full retard
	engineConfiguration->enableLaunchIgnCut = true;
	engineConfiguration->enableLaunchFuelCut = false;
	engineConfiguration->launch.hardCutRpmRange = 500; //Rpm above Launch triggered +(if retard enabled) launchAdvanceRpmRange to hard cut
	engineConfiguration->launch.launchSpeedTreshold = 10; //maximum speed allowed before disable launch
	engineConfiguration->launch.launchFuelAdded = 10; // Extra fuel in % when launch are triggered
	engineConfiguration->launch.launchBoostDuty = 70; // boost valve duty cycle at launch
	engineConfiguration->launch.launchActivateDelay = 3; // Delay in seconds for launch to kick in
	engineConfiguration->enableLaunchRetard = true;
	engineConfiguration->enableLaunchBoost = true;
	engineConfiguration->launchSmoothRetard = true; //interpolates the advance linear from launchrpm to fully retarded at launchAdvanceRpmRange
}


void initLaunchControl( DECLARE_ENGINE_PARAMETER_SUFFIX) {
	
	Launch.Start();
}

#endif /* EFI_LAUNCH_CONTROL */


