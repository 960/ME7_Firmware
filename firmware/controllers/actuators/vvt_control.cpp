/*
 * vvt_control.cpp
 *
 *  Created on: 25. nov. 2019
 *     (C) All rights reserved by RUUD BILELEKTRO, NORWAY
 */

#include "global.h"
#include "vvt_control.h"
#if EFI_VVT_CONTROL

#include "tunerstudio_outputs.h"
#include "pin_repository.h"
#include "sensor.h"

#include "global.h"
#include "os_access.h"

#include "trigger_central.h"
#include "trigger_decoder.h"
#include "main_trigger_callback.h"
#include "engine_configuration.h"
#include "listener_array.h"
#include "pwm_generator_logic.h"
#include "tooth_logger.h"


#include "engine_math.h"
#include "local_version_holder.h"
#include "trigger_simulator.h"

#include "rpm_calculator.h"
#include "tooth_logger.h"
#include "perf_trace.h"
#define NO_PIN_PERIOD 500



EXTERN_ENGINE;


static pid_s *vvtPidS = &persistentState.persistentConfiguration.engineConfiguration.vvt.vvtPid;
static Pid vvtControlPid(vvtPidS);
static bool shouldResetPid = false;


static vvt_Map3D_t vvtMap("vvtmap");
static bool isEnabled = engineConfiguration->isVvtControlEnabled;


static SimplePwm vvtPwmControl("vvt");
static void pidReset(void) {
	vvtControlPid.reset();
}


class VVTControl: public PeriodicTimerController {
public:

OutputPin vvtPin;
percent_t vvtDuty = 0;
float targetAngle = 0.0;
int getPeriodMs()
	 override {
		return engineConfiguration->vvt.vvtControlPin == GPIO_UNASSIGNED ? NO_PIN_PERIOD : GET_PERIOD_LIMITED(&engineConfiguration->vvt.vvtPid);
		}
	void PeriodicTask() override {
		if (shouldResetPid) {
			pidReset();
			shouldResetPid = false;
		}

#if EFI_TUNER_STUDIO
		vvtControlPid.postState(&tsOutputChannels);
#endif /* EFI_TUNER_STUDIO */

	float rpm = GET_RPM();
	auto mapValue = Sensor::get(SensorType::Map);
	auto tps = Sensor::get(SensorType::DriverThrottleIntent);
	auto coolant = Sensor::get(SensorType::Clt);

	 if (GET_RPM() == 0) {
	    vvtControlPid.reset();
	    return;
	 }

	float camAngle = engine->triggerCentral.vvtPosition;
	float temp = engineConfiguration->vvt.minVvtTemperature;

	if (temp > coolant.Value) {
		return;
	}
	if (engineConfiguration->vvt.vvtType == CLOSED_LOOP_VVT) {
		if (engineConfiguration->vvt.vvtLoadAxis == VVT_LOAD_TPS) {
		targetAngle = vvtMap.getValue(rpm / RPM_1_BYTE_PACKING_MULT, tps.Value);
		}
	else if (engineConfiguration->vvt.vvtLoadAxis == VVT_LOAD_MAP) {
		targetAngle = vvtMap.getValue(rpm / RPM_1_BYTE_PACKING_MULT, mapValue.Value);
		}
	else if (engineConfiguration->vvt.vvtLoadAxis == VVT_LOAD_CLT) {
		targetAngle = vvtMap.getValue(rpm / RPM_1_BYTE_PACKING_MULT, coolant.Value);
		}

		if (targetAngle > CONFIG(vvt.maxVvtDeviation)) {
		targetAngle = CONFIG(vvt.maxVvtDeviation);
		}

		if  (targetAngle < -CONFIG(vvt.maxVvtDeviation)) {
		targetAngle = -CONFIG(vvt.maxVvtDeviation);
		}

		vvtDuty = vvtControlPid.getOutput(targetAngle, camAngle);

		vvtPwmControl.setSimplePwmDutyCycle(PERCENT_TO_DUTY(vvtDuty));

		if (engineConfiguration->debugMode == DBG_VVT) {
#if EFI_TUNER_STUDIO
		tsOutputChannels.debugFloatField3 = vvtDuty;
		tsOutputChannels.debugFloatField4 = camAngle;
#endif /* EFI_TUNER_STUDIO */
		}
		}

	}
};

static VVTControl VVTController;

void setVvtPFactor(float value) {
	engineConfiguration->vvt.vvtPid.pFactor = value;
	vvtControlPid.reset();
}
void setVvtIFactor(float value) {
	engineConfiguration->vvt.vvtPid.iFactor = value;
	vvtControlPid.reset();
}
void setVvtDFactor(float value) {
	engineConfiguration->vvt.vvtPid.dFactor = value;
	vvtControlPid.reset();
}
void setDefaultVvtParameters(DECLARE_CONFIG_PARAMETER_SIGNATURE) {
	engineConfiguration->isVvtControlEnabled = true;
	engineConfiguration->vvt.vvtPwmFrequency = 88;
	engineConfiguration->vvt.vvtPid.offset = 0;
	engineConfiguration->vvt.vvtPid.pFactor = 1;
	engineConfiguration->vvt.vvtPid.periodMs = 100;
	engineConfiguration->vvt.vvtPid.maxValue = 99;
	setLinearCurve(config->vvtRpmBins, 0, 8000 / RPM_1_BYTE_PACKING_MULT, 1);
	setLinearCurve(config->vvtLoadBins, 0, 100, 1);
	for (int loadIndex = 0;loadIndex<VVT_LOAD_COUNT;loadIndex++) {
		for (int rpmIndex = 0;rpmIndex<VVT_RPM_COUNT;rpmIndex++) {
		config->vvtTable[loadIndex][rpmIndex] = config->vvtLoadBins[loadIndex];
		}
	}
}
static void turnVvtPidOn() {
	if (CONFIG(vvt.vvtControlPin) == GPIO_UNASSIGNED){
	return;
	} else {
	startSimplePwmHard(&vvtPwmControl, "vvt", &engine->executor,
	CONFIG(vvt.vvtControlPin), &enginePins.vvtPin,
	engineConfiguration->vvt.vvtPwmFrequency, 0.1);
	}
}
void startVvtPin(void) {
	turnVvtPidOn();
}
void stopVvtPin(void) {
	brain_pin_markUnused(activeConfiguration.vvt.vvtControlPin);
}
void onConfigurationChangeVvtCallback(engine_configuration_s *previousConfiguration) {
	shouldResetPid = !vvtControlPid.isSame(&previousConfiguration->vvt.vvtPid);
}
void initVvtCtrl( DECLARE_ENGINE_PARAMETER_SUFFIX){

	vvtMap.init(config->vvtTable, config->vvtLoadBins, config->vvtRpmBins);
	vvtControlPid.reset();
	startVvtPin();
	VVTController.Start();
}
#endif
