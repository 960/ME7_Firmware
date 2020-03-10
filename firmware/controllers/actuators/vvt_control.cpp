/*
 * vvt_control.cpp
 *
 *  Created on: 25. nov. 2019
 *      Author: Ola
 */

#include "global.h"
#include "vvt_control.h"
#if EFI_VVT_CONTROL
#if EFI_TUNER_STUDIO
#include "tunerstudio_configuration.h"
#endif /* EFI_TUNER_STUDIO */
#include "engine.h"
#include "tps.h"
#include "map.h"
#include "io_pins.h"
#include "engine_configuration.h"
#include "pwm_generator_logic.h"
#include "pid.h"
#include "engine_controller.h"
#include "periodic_task.h"
#include "pin_repository.h"
#include "pwm_generator.h"
#include "pid_auto_tune.h"
#include "local_version_holder.h"
#include "thermistors.h"
#define NO_PIN_PERIOD 500

#if defined(HAS_OS_ACCESS)
#error "Unexpected OS ACCESS HERE"
#endif

EXTERN_ENGINE;


static pid_s *vvtPidS = &persistentState.persistentConfiguration.engineConfiguration.vvtPid;
static Pid vvtControlPid(vvtPidS);
static bool shouldResetPid = false;


static vvt_Map3D_t vvtMap("vvtmap", 1);
static bool isEnabled = engineConfiguration->isVvtControlEnabled;


static SimplePwm vvtPwmControl("vvt");
static void pidReset(void) {
	vvtControlPid.reset();
}

static Logging *logger;

class VVTControl: public PeriodicTimerController {
public:
OutputPin vvtPin;
percent_t vvtDuty = 0;
float targetAngle = 0.0;
int getPeriodMs()
	 override {
		return GET_PERIOD_LIMITED(&engineConfiguration->vvtPid);

				return engineConfiguration->vvtControlPin == GPIO_UNASSIGNED ? NO_PIN_PERIOD : GET_PERIOD_LIMITED(&engineConfiguration->vvtPid);


	}
	void PeriodicTask() override {
		if (shouldResetPid) {
					pidReset();
					shouldResetPid = false;
				}

#if EFI_TUNER_STUDIO
		vvtControlPid.postState(&tsOutputChannels);
#endif /* EFI_TUNER_STUDIO */

	float rpm = GET_RPM_VALUE;
	float mapValue = getMap(PASS_ENGINE_PARAMETER_SIGNATURE);
	float tps = getTPS(PASS_ENGINE_PARAMETER_SIGNATURE);
	float coolant = getCoolantTemperature();



	//   bool enabledAtEngineRunning = rpm < engineConfiguration->cranking.rpm;
	 //   	 if (!enabledAtEngineRunning) {
	  //  		 vvtControlPid.reset();
	   // 	   				return;
	   // 	     }


		      float temp = engineConfiguration->minVvtTemperature;
               if (temp > coolant) {
        		   return;
        		 }

		      if (engineConfiguration->vvtType == false)  {


		      if(engineConfiguration->vvtLoadAxis == VVT_LOAD_TPS) {

		    	  targetAngle = vvtMap.getValue(rpm / RPM_1_BYTE_PACKING_MULT, tps);
		      }

		      else if(engineConfiguration->vvtLoadAxis == VVT_LOAD_MAP) {

		   		  targetAngle = vvtMap.getValue(rpm / RPM_1_BYTE_PACKING_MULT, mapValue);

		   		      }


		      else if(engineConfiguration->vvtLoadAxis == VVT_LOAD_CLT) {

		    	 targetAngle = vvtMap.getValue(rpm / RPM_1_BYTE_PACKING_MULT, coolant);
		      }
		    		   		    	 if(targetAngle > engineConfiguration->maxVvtDeviation) {
		    		   		    		 targetAngle = engineConfiguration->maxVvtDeviation;
		    		   		    	  }

		    		   		       float camAngle = engine->triggerCentral.vvtPosition;
		    		   		       vvtDuty = vvtControlPid.getOutput(targetAngle, camAngle);
		    		   		    	vvtPwmControl.setSimplePwmDutyCycle(PERCENT_TO_DUTY(vvtDuty));


		    		   		    		 if (vvtDuty > 95)
		    		   		    			vvtDuty = 95;
		    		   		    		 if (vvtDuty < 5)
		    		   		    			vvtDuty = 5;

#if EFI_TUNER_STUDIO

	tsOutputChannels.vvtDuty = vvtDuty;
#endif /* EFI_TUNER_STUDIO */
		    		   		      }



	}
};

static VVTControl VVTController;


void setVvtPFactor(float value) {
	engineConfiguration->vvtPid.pFactor = value;
	vvtControlPid.reset();

}

void setVvtIFactor(float value) {
	engineConfiguration->vvtPid.iFactor = value;
	vvtControlPid.reset();
}


void setVvtDFactor(float value) {
	engineConfiguration->vvtPid.dFactor = value;
	vvtControlPid.reset();
}
void setDefaultVvtParameters(DECLARE_CONFIG_PARAMETER_SIGNATURE) {


	engineConfiguration->isVvtControlEnabled = true;
	engineConfiguration->vvtPwmFrequency = 88;
	engineConfiguration->vvtPid.offset = 0;
	engineConfiguration->vvtPid.pFactor = 1;
	engineConfiguration->vvtPid.periodMs = 100;
	engineConfiguration->vvtPid.maxValue = 99;


	setLinearCurve(config->vvtRpmBins, 0, 8000 / RPM_1_BYTE_PACKING_MULT, 1);
	setLinearCurve(config->vvtLoadBins, 0, 100, 1);
	for (int loadIndex = 0;loadIndex<VVT_LOAD_COUNT;loadIndex++) {
			for (int rpmIndex = 0;rpmIndex<VVT_RPM_COUNT;rpmIndex++) {
				config->vvtTable1[loadIndex][rpmIndex] = config->vvtLoadBins[loadIndex];
			}
		}
}

static void turnVvtPidOn() {
	if (CONFIG(vvtControlPin) == GPIO_UNASSIGNED){
		return;
	} else {
	startSimplePwmExt(&vvtPwmControl, "vvt", &engine->executor,
			CONFIG(vvtControlPin), &enginePins.vvtPin,
			engineConfiguration->vvtPwmFrequency, 0.1,
			(pwm_gen_callback*) applyPinState);
	}
}

void startVvtPin(void) {
	turnVvtPidOn();

}
void stopVvtPin(void) {
	brain_pin_markUnused(activeConfiguration.vvtControlPin);
}
void onConfigurationChangeVvtCallback(engine_configuration_s *previousConfiguration) {
	shouldResetPid = !vvtControlPid.isSame(&previousConfiguration->vvtPid);
}

void initVvtCtrl(Logging *sharedLogger DECLARE_ENGINE_PARAMETER_SUFFIX){
	logger = sharedLogger;

	vvtMap.init(config->vvtTable1, config->vvtLoadBins, config->vvtRpmBins);
	vvtControlPid.reset();
	startVvtPin();
	VVTController.Start();

}
#endif
