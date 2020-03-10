/*
 * launch_control.cpp
 *
 *  Created on: 10. sep. 2019
 *      Author: Ola Ruud
 */
#include "global.h"
//#if EFI_LAUNCH_CONTROL
#include "boost_control.h"
#include "vehicle_speed.h"
#include "launch_control.h"
#include "tunerstudio_configuration.h"
#include "boost_control.h"
#include "io_pins.h"
#include "engine_configuration.h"
#include "engine_controller.h"
#include "periodic_task.h"
#include "pin_repository.h"
#include "allsensors.h"
#include "engine_math.h"
#include "efi_gpio.h"
#include "advance_map.h"
#include "engine_state.h"
#include "advance_map.h"
#if defined(HAS_OS_ACCESS)
#error "Unexpected OS ACCESS HERE"
#endif


static Logging *logger;




#if EFI_TUNER_STUDIO
extern TunerStudioOutputChannels tsOutputChannels;
#endif /* EFI_TUNER_STUDIO */

EXTERN_ENGINE;

class LaunchControl: public PeriodicTimerController {
	efitick_t launchTimer;

int getPeriodMs() override {
return 50;
	}
void PeriodicTask() override {


	int rpm = GET_RPM_VALUE;
	int speed = getVehicleSpeed();
	int tps = getTPS(PASS_ENGINE_PARAMETER_SIGNATURE);
	int speedtreeshold = engineConfiguration->launchSpeedTreshold;
		int tpstreshold = engineConfiguration->launchTpsTreshold;
	bool activateSwitchCondition = false;
	float timeDelay = engineConfiguration->launchActivateDelay;
	int cutRpmRange = engineConfiguration->hardCutRpmRange;
			int launchAdvanceRpmRange = engineConfiguration->launchTimingRpmRange;
			int launchRpm = engineConfiguration->launchRpm;

	if (!engineConfiguration->launchControlEnabled)
				return;

	if (engineConfiguration->launchActivationMode == SWITCH_INPUT_LAUNCH) {
					if (CONFIG(launchActivatePin) != GPIO_UNASSIGNED) {
						engine->launchActivatePinState = efiReadPin(CONFIG(launchActivatePin));
					}
					activateSwitchCondition = engine->launchActivatePinState;

				   }
		else if (engineConfiguration->launchActivationMode == CLUTCH_INPUT_LAUNCH) {
				if (CONFIG(clutchDownPin) != GPIO_UNASSIGNED) {
					engine->clutchDownState = efiReadPin(CONFIG(clutchDownPin));
		           	}
				activateSwitchCondition = engine->clutchDownState;

		}

	  activateSwitchCondition = activateSwitchCondition||(engineConfiguration->launchActivationMode == ALWAYS_ACTIVE_LAUNCH);

		bool rpmCondition = (launchRpm < rpm);
		bool tpsCondition = (tpstreshold < tps);

	 bool speedCondition = (engineConfiguration->launchDisableBySpeed && (speedtreeshold > speed)) || ! engineConfiguration->launchDisableBySpeed;
	bool combinedConditions = (speedCondition && activateSwitchCondition  && rpmCondition  && tpsCondition);


	if (!combinedConditions) {
	    // conditions not met, reset timer
	    launchTimer = getTimeNowNt();
	    engine->isLaunchCondition = false;
		engine->setLaunchBoostDuty = false;
	   	engine->applyLaunchControlRetard = false;
	    engine->applyLaunchExtraFuel = false;
	} else {
	    // If conditions are met...
	    if ((getTimeNowNt() - launchTimer > MS2NT(timeDelay*1000)) && (combinedConditions)) {
	    	 engine->isLaunchCondition = true;           // ...enable launch!
	    	 engine->applyLaunchExtraFuel = true;
	    }
	    if (engineConfiguration->enableLaunchBoost) {
	   	 engine->setLaunchBoostDuty = true;              // ...enable boost!
	    }
	    if (engineConfiguration->enableLaunchRetard) {
	    	 engine->applyLaunchControlRetard = true;    // ...enable retard!
	    }

	}


	 	if (engineConfiguration->debugMode == DBG_LAUNCH) {
	 	#if EFI_TUNER_STUDIO
	 				tsOutputChannels.debugIntField1 =  rpmCondition;
	 				tsOutputChannels.debugIntField2 = tpsCondition;
	 				tsOutputChannels.debugIntField3 = speedCondition;
	 				tsOutputChannels.debugIntField4 = activateSwitchCondition;
	 				tsOutputChannels.debugIntField5 = engine->clutchDownState;
	 				tsOutputChannels.debugFloatField1 = engine->launchActivatePinState;
	 				tsOutputChannels.debugFloatField2  = engine->isLaunchCondition;
	 				tsOutputChannels.debugFloatField3  = combinedConditions;
	 	#endif /* EFI_TUNER_STUDIO */
	 			}
	 }

};

static LaunchControl Launch;

void setDefaultLaunchParameters(DECLARE_CONFIG_PARAMETER_SIGNATURE) {
	engineConfiguration->launchRpm = 4000;        // Rpm to trigger Launch condition
	engineConfiguration->launchTimingRetard = 10;  // retard in absolute degrees ATDC
	engineConfiguration->launchTimingRpmRange = 500;   // Rpm above Launch triggered for full retard
	engineConfiguration->launchSparkCutEnable = true;
	engineConfiguration->launchFuelCutEnable = false;
	engineConfiguration->hardCutRpmRange = 500;       //Rpm above Launch triggered +(if retard enabled) launchTimingRpmRange to hard cut
	engineConfiguration->launchSpeedTreshold = 10;   //maximum speed allowed before disable launch
	engineConfiguration->launchFuelAdded = 10;       // Extra fuel in % when launch are triggered
	engineConfiguration->launchBoostDuty = 70;       // boost valve duty cycle at launch
	engineConfiguration->launchActivateDelay = 3;    // Delay in seconds for launch to kick in
	engineConfiguration->enableLaunchRetard = true;
	engineConfiguration->enableLaunchBoost= true;
	engineConfiguration->launchSmoothRetard = true; //interpolates the advance linear from launchrpm to fully retarded at launchtimingrpmrange
	engineConfiguration->antilagTimeout = 3;
	engineConfiguration->antiLagRpmTreshold = 3000;
}


void applyLaunchControlLimiting(bool *limitedSpark, bool *limitedFuel DECLARE_ENGINE_PARAMETER_SUFFIX) {
	int cutRpmRange = engineConfiguration->hardCutRpmRange;
	int launchAdvanceRpmRange = engineConfiguration->launchTimingRpmRange;
	int launchRpm = engineConfiguration->launchRpm;
	int rpm = GET_RPM_VALUE;

	if (!engineConfiguration->enableLaunchRetard) {
	if ((launchRpm + cutRpmRange) > rpm) {
		*limitedSpark = ((engine->isLaunchCondition) && (engineConfiguration->launchSparkCutEnable)) ;
		*limitedFuel = ((engine->isLaunchCondition) && (engineConfiguration->launchFuelCutEnable));
		engine->rpmHardCut = true;
		}
	} else if (engineConfiguration->enableLaunchRetard) {
		if ((launchRpm + launchAdvanceRpmRange + cutRpmRange) > rpm) {
				*limitedSpark = ((engine->isLaunchCondition) && (engineConfiguration->launchSparkCutEnable)) ;
				*limitedFuel = ((engine->isLaunchCondition) && (engineConfiguration->launchFuelCutEnable));
				engine->rpmHardCut = true;
		}
	}
}



void initLaunchControl(Logging *sharedLogger DECLARE_ENGINE_PARAMETER_SUFFIX) {
	logger = sharedLogger;
	Launch.Start();
}
//#endif
