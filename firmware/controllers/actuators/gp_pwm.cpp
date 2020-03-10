/*
 * gp_pwm.cpp
 *
 *  Created on: 19. des. 2019
 *      Author: Ola
 */
#define EFI_GP_PWM TRUE
#include "global.h"
#if EFI_GP_PWM

#if EFI_TUNER_STUDIO
#include "tunerstudio_configuration.h"
#endif /* EFI_TUNER_STUDIO */

#include "engine.h"
#include "thermistors.h"
#include "tps.h"
#include "map.h"
#include "io_pins.h"
#include "engine_configuration.h"
#include "gp_pwm.h"
#include "engine_controller.h"
#include "periodic_task.h"
#include "pin_repository.h"
#include "pwm_generator.h"
#include "local_version_holder.h"
#include "pwm_generator_logic.h"
#if defined(HAS_OS_ACCESS)
#error "Unexpected OS ACCESS HERE"
#endif
EXTERN_ENGINE;

#define NO_PIN_PERIOD 500

static Logging *logger;
extern pin_output_mode_e DEFAULT_OUTPUT;

static percent_t duty1;
static percent_t duty2;
static percent_t duty3;
static percent_t duty4;

static gpPwm1_Map3D_t gpPwmMap1("gpPwm1map", 1);
static gpPwm1_Map3D_t gpPwmMap2("gpPwm2map", 1);
static gpPwm1_Map3D_t gpPwmMap3("gpPwm3map", 1);
static gpPwm1_Map3D_t gpPwmMap4("gpPwm4map", 1);

static SimplePwm gpPwm1("gpPwm1");
static SimplePwm gpPwm2("gpPwm2");
static SimplePwm gpPwm3("gpPwm3");
static SimplePwm gpPwm4("gpPwm4");

class GpPWMControl: public PeriodicTimerController {
	int getPeriodMs() override {
		return 50;
	}

	void PeriodicTask() override {

		float rpm = GET_RPM_VALUE;
		float map = getMap(PASS_ENGINE_PARAMETER_SIGNATURE);
		float tps = getTPS(PASS_ENGINE_PARAMETER_SIGNATURE);
		float clt = getCoolantTemperatureM(PASS_ENGINE_PARAMETER_SIGNATURE);

		//Conditions
		float rpm1 = engineConfiguration->gppwm.enableGpPwm1AtRpm;
		float rpm2 = engineConfiguration->gppwm.enableGpPwm2AtRpm;
		float rpm3 = engineConfiguration->gppwm.enableGpPwm3AtRpm;
		float rpm4 = engineConfiguration->gppwm.enableGpPwm4AtRpm;

		float clt1 = engineConfiguration->gppwm.enableGpPwm1AtClt;
		float clt2 = engineConfiguration->gppwm.enableGpPwm2AtClt;
		float clt3 = engineConfiguration->gppwm.enableGpPwm3AtClt;
		float clt4 = engineConfiguration->gppwm.enableGpPwm4AtClt;

		float tps1 = engineConfiguration->gppwm.enableGpPwm1AtTps;
		float tps2 = engineConfiguration->gppwm.enableGpPwm2AtTps;
		float tps3 = engineConfiguration->gppwm.enableGpPwm3AtTps;
		float tps4 = engineConfiguration->gppwm.enableGpPwm4AtTps;

		float map1 = engineConfiguration->gppwm.enableGpPwm1AtMap;
		float map2 = engineConfiguration->gppwm.enableGpPwm2AtMap;
		float map3 = engineConfiguration->gppwm.enableGpPwm3AtMap;
		float map4 = engineConfiguration->gppwm.enableGpPwm4AtMap;



		//Gp Pwm 1
		if (engineConfiguration->gpPwm1Load == GP1_LOAD_MAP) {
			duty1 = gpPwmMap1.getValue(rpm / RPM_1_BYTE_PACKING_MULT, map);

		} else if (engineConfiguration->gpPwm1Load == GP1_LOAD_TPS) {
			duty1 = gpPwmMap1.getValue(rpm / RPM_1_BYTE_PACKING_MULT, tps);

		} else if (engineConfiguration->gpPwm1Load == GP1_LOAD_CLT) {
			duty1 = gpPwmMap1.getValue(rpm / RPM_1_BYTE_PACKING_MULT, clt);
		}
		if ((engineConfiguration->gppwm.conditionGpPwm1Switch) && (CONFIG(gppwmIo.gpPwm1InputPin) != GPIO_UNASSIGNED)) {
			engine->gpPwm1InputPinState = efiReadPin(CONFIG(gppwmIo.gpPwm1InputPin));
			if (!engine->gpPwm1InputPinState) {
			duty1 = 0;
			  }
		   }


		if ((engineConfiguration->gppwm.conditionGpPwm1Rpm)&& (!engineConfiguration->gppwm.gpPwm1BelowOrAboveRpm)  && (rpm1 < rpm)) {
			duty1 = 0;
		}
		if ((engineConfiguration->gppwm.conditionGpPwm1Rpm)&& (engineConfiguration->gppwm.gpPwm1BelowOrAboveRpm)  && (rpm1 > rpm)) {
					duty1 = 0;
				}

		if ((engineConfiguration->gppwm.conditionGpPwm1Clt)&& (!engineConfiguration->gppwm.gpPwm1BelowOrAboveClt)  && (clt1 < clt)) {
					duty1 = 0;
				}
		if ((engineConfiguration->gppwm.conditionGpPwm1Clt)&& (engineConfiguration->gppwm.gpPwm1BelowOrAboveClt)  && (clt1 > clt)) {
					duty1 = 0;
				}

		if ((engineConfiguration->gppwm.conditionGpPwm1Tps)&& (!engineConfiguration->gppwm.gpPwm1BelowOrAboveTps)  && (tps1 < tps)) {
					duty1 = 0;
								}
		if ((engineConfiguration->gppwm.conditionGpPwm1Tps)&& (engineConfiguration->gppwm.gpPwm1BelowOrAboveTps)  && (tps1 > tps)) {
				    duty1 = 0;
				}

		if ((engineConfiguration->gppwm.conditionGpPwm1Map)&& (!engineConfiguration->gppwm.gpPwm1BelowOrAboveMap)  && (map1 < map)) {
					duty1 = 0;
			}
		if ((engineConfiguration->gppwm.conditionGpPwm1Map)&& (engineConfiguration->gppwm.gpPwm1BelowOrAboveMap)  && (map1 > map)) {
					duty1 = 0;
			}

		gpPwm1.setSimplePwmDutyCycle(PERCENT_TO_DUTY(duty1));

		//Gp Pwm 2
		if (engineConfiguration->gpPwm2Load == GP2_LOAD_MAP) {
			duty2 = gpPwmMap2.getValue(rpm / RPM_1_BYTE_PACKING_MULT, map);
		} else if (engineConfiguration->gpPwm2Load == GP2_LOAD_TPS) {
			duty1 = gpPwmMap2.getValue(rpm / RPM_1_BYTE_PACKING_MULT, tps);
		} else if (engineConfiguration->gpPwm2Load == GP2_LOAD_CLT) {
			duty2 = gpPwmMap2.getValue(rpm, clt);
		}

		if ((engineConfiguration->gppwm.conditionGpPwm2Switch) && (CONFIG(gppwmIo.gpPwm2InputPin) != GPIO_UNASSIGNED)) {
					engine->gpPwm2InputPinState = efiReadPin(CONFIG(gppwmIo.gpPwm2InputPin));
					if (!engine->gpPwm2InputPinState) {
					duty2 = 0;
					  }
				   }

		if ((engineConfiguration->gppwm.conditionGpPwm2Rpm)&& (!engineConfiguration->gppwm.gpPwm2BelowOrAboveRpm)  && (rpm2 < rpm)) {
					duty2 = 0;
				}
				if ((engineConfiguration->gppwm.conditionGpPwm2Rpm)&& (engineConfiguration->gppwm.gpPwm2BelowOrAboveRpm)  && (rpm2 > rpm)) {
							duty2 = 0;
						}

				if ((engineConfiguration->gppwm.conditionGpPwm2Clt)&& (!engineConfiguration->gppwm.gpPwm2BelowOrAboveClt)  && (clt2 < clt)) {
							duty2 = 0;
						}
				if ((engineConfiguration->gppwm.conditionGpPwm2Clt)&& (engineConfiguration->gppwm.gpPwm2BelowOrAboveClt)  && (clt2 > clt)) {
							duty2 = 0;
						}

				if ((engineConfiguration->gppwm.conditionGpPwm2Tps)&& (!engineConfiguration->gppwm.gpPwm2BelowOrAboveTps)  && (tps2 < tps)) {
							duty2 = 0;
										}
				if ((engineConfiguration->gppwm.conditionGpPwm2Tps)&& (engineConfiguration->gppwm.gpPwm2BelowOrAboveTps)  && (tps2 > tps)) {
						    duty2 = 0;
						}

				if ((engineConfiguration->gppwm.conditionGpPwm2Map)&& (!engineConfiguration->gppwm.gpPwm2BelowOrAboveMap)  && (map2 < map)) {
							duty2 = 0;
					}
				if ((engineConfiguration->gppwm.conditionGpPwm2Map)&& (engineConfiguration->gppwm.gpPwm2BelowOrAboveMap)  && (map2 > map)) {
							duty2 = 0;
					}
		gpPwm2.setSimplePwmDutyCycle(PERCENT_TO_DUTY(duty2));

		//Gp Pwm 3
		if (engineConfiguration->gpPwm3Load == GP3_LOAD_MAP) {
			duty3 = gpPwmMap3.getValue(rpm / RPM_1_BYTE_PACKING_MULT, map);
		} else if (engineConfiguration->gpPwm3Load == GP3_LOAD_TPS) {
			duty3 = gpPwmMap3.getValue(rpm / RPM_1_BYTE_PACKING_MULT, tps);
		} else if (engineConfiguration->gpPwm3Load == GP3_LOAD_CLT) {
			duty3 = gpPwmMap3.getValue(rpm, clt);
		}

		if ((engineConfiguration->gppwm.conditionGpPwm3Switch) && (CONFIG(gppwmIo.gpPwm3InputPin) != GPIO_UNASSIGNED)) {
					engine->gpPwm3InputPinState = efiReadPin(CONFIG(gppwmIo.gpPwm3InputPin));
					if (!engine->gpPwm3InputPinState) {
					duty3 = 0;
					  }
				   }

		if ((engineConfiguration->gppwm.conditionGpPwm3Rpm)&& (!engineConfiguration->gppwm.gpPwm3BelowOrAboveRpm)  && (rpm3 < rpm)) {
					duty3 = 0;
				}
				if ((engineConfiguration->gppwm.conditionGpPwm3Rpm)&& (engineConfiguration->gppwm.gpPwm3BelowOrAboveRpm)  && (rpm3 > rpm)) {
							duty3 = 0;
						}

				if ((engineConfiguration->gppwm.conditionGpPwm3Clt)&& (!engineConfiguration->gppwm.gpPwm3BelowOrAboveClt)  && (clt3 < clt)) {
							duty3 = 0;
						}
				if ((engineConfiguration->gppwm.conditionGpPwm3Clt)&& (engineConfiguration->gppwm.gpPwm3BelowOrAboveClt)  && (clt3 > clt)) {
							duty3 = 0;
						}

				if ((engineConfiguration->gppwm.conditionGpPwm3Tps)&& (!engineConfiguration->gppwm.gpPwm3BelowOrAboveTps)  && (tps3 < tps)) {
							duty3 = 0;
										}
				if ((engineConfiguration->gppwm.conditionGpPwm3Tps)&& (engineConfiguration->gppwm.gpPwm3BelowOrAboveTps)  && (tps3 > tps)) {
						    duty3 = 0;
						}

				if ((engineConfiguration->gppwm.conditionGpPwm3Map)&& (!engineConfiguration->gppwm.gpPwm3BelowOrAboveMap)  && (map3 < map)) {
							duty3 = 0;
					}
				if ((engineConfiguration->gppwm.conditionGpPwm3Map)&& (engineConfiguration->gppwm.gpPwm3BelowOrAboveMap)  && (map3 > map)) {
							duty3 = 0;
					}
		gpPwm3.setSimplePwmDutyCycle(PERCENT_TO_DUTY(duty3));

		//Gp Pwm 4
		if (engineConfiguration->gpPwm4Load == GP4_LOAD_MAP) {
			duty4 = gpPwmMap4.getValue(rpm / RPM_1_BYTE_PACKING_MULT, map);
		} else if (engineConfiguration->gpPwm4Load == GP4_LOAD_TPS) {
			duty4 = gpPwmMap4.getValue(rpm / RPM_1_BYTE_PACKING_MULT, tps);
		} else if (engineConfiguration->gpPwm4Load == GP4_LOAD_CLT) {
			duty4 = gpPwmMap4.getValue(rpm, clt);
		}

		if ((engineConfiguration->gppwm.conditionGpPwm4Switch) && (CONFIG(gppwmIo.gpPwm4InputPin) != GPIO_UNASSIGNED)) {
					engine->gpPwm4InputPinState = efiReadPin(CONFIG(gppwmIo.gpPwm4InputPin));
					if (!engine->gpPwm4InputPinState) {
					duty4 = 0;
					  }
				   }

		if ((engineConfiguration->gppwm.conditionGpPwm4Rpm)&& (!engineConfiguration->gppwm.gpPwm4BelowOrAboveRpm)  && (rpm4 < rpm)) {
					duty4 = 0;
				}
				if ((engineConfiguration->gppwm.conditionGpPwm4Rpm)&& (engineConfiguration->gppwm.gpPwm4BelowOrAboveRpm)  && (rpm4 > rpm)) {
							duty4 = 0;
						}

				if ((engineConfiguration->gppwm.conditionGpPwm4Clt)&& (!engineConfiguration->gppwm.gpPwm4BelowOrAboveClt)  && (clt4 < clt)) {
							duty4 = 0;
						}
				if ((engineConfiguration->gppwm.conditionGpPwm4Clt)&& (engineConfiguration->gppwm.gpPwm4BelowOrAboveClt)  && (clt4 > clt)) {
							duty4 = 0;
						}

				if ((engineConfiguration->gppwm.conditionGpPwm4Tps)&& (!engineConfiguration->gppwm.gpPwm4BelowOrAboveTps)  && (tps4 < tps)) {
							duty4 = 0;
										}
				if ((engineConfiguration->gppwm.conditionGpPwm4Tps)&& (engineConfiguration->gppwm.gpPwm4BelowOrAboveTps)  && (tps4 > tps)) {
						    duty4 = 0;
						}

				if ((engineConfiguration->gppwm.conditionGpPwm4Map)&& (!engineConfiguration->gppwm.gpPwm4BelowOrAboveMap)  && (map4 < map)) {
							duty4 = 0;
					}
				if ((engineConfiguration->gppwm.conditionGpPwm4Map)&& (engineConfiguration->gppwm.gpPwm4BelowOrAboveMap)  && (map4 > map)) {
							duty4 = 0;
					}
		gpPwm4.setSimplePwmDutyCycle(PERCENT_TO_DUTY(duty4));

#if EFI_TUNER_STUDIO
		tsOutputChannels.gpPwm1Duty = duty1;
	#endif /* EFI_TUNER_STUDIO */
	}
};

static GpPWMControl GpPWMController;

void setDefaultGpPwmParameters(DECLARE_CONFIG_PARAMETER_SIGNATURE) {
	engineConfiguration->gppwmIo.gpPwm1Pin = GPIO_UNASSIGNED;
	engineConfiguration->gppwmIo.gpPwm2Pin = GPIO_UNASSIGNED;
	engineConfiguration->gppwmIo.gpPwm3Pin = GPIO_UNASSIGNED;
	engineConfiguration->gppwmIo.gpPwm4Pin = GPIO_UNASSIGNED;
	engineConfiguration->gppwm.gpPwm1Frequency = 50;
	engineConfiguration->gppwm.gpPwm2Frequency = 50;
	engineConfiguration->gppwm.gpPwm3Frequency = 50;
	engineConfiguration->gppwm.gpPwm4Frequency = 50;

	setLinearCurve(config->gpPwm1LoadBins, 0, 100, 1);
	setLinearCurve(config->gpPwm1RpmBins, 0, 8000 / RPM_1_BYTE_PACKING_MULT, 1);
	setLinearCurve(config->gpPwm2LoadBins, 0, 100, 1);
	setLinearCurve(config->gpPwm2RpmBins, 0, 8000 / RPM_1_BYTE_PACKING_MULT, 1);
	setLinearCurve(config->gpPwm3LoadBins, 0, 100, 1);
	setLinearCurve(config->gpPwm3RpmBins, 0, 8000 / RPM_1_BYTE_PACKING_MULT, 1);
	setLinearCurve(config->gpPwm4LoadBins, 0, 100, 1);
	setLinearCurve(config->gpPwm4RpmBins, 0, 8000 / RPM_1_BYTE_PACKING_MULT, 1);


	for (int loadIndex = 0; loadIndex < GP_PWM_LOAD_COUNT; loadIndex++) {
		for (int rpmIndex = 0; rpmIndex < GP_PWM_RPM_COUNT; rpmIndex++) {
			config->gpPwmTable1[loadIndex][rpmIndex] = config->gpPwm1LoadBins[loadIndex];
		}
	}
	for (int loadIndex = 0; loadIndex < GP_PWM_LOAD_COUNT; loadIndex++) {
		for (int rpmIndex = 0; rpmIndex < GP_PWM_RPM_COUNT; rpmIndex++) {
			config->gpPwmTable2[loadIndex][rpmIndex] = config->gpPwm2LoadBins[loadIndex];
		}
	}
	for (int loadIndex = 0; loadIndex < GP_PWM_LOAD_COUNT; loadIndex++) {
		for (int rpmIndex = 0; rpmIndex < GP_PWM_RPM_COUNT; rpmIndex++) {
			config->gpPwmTable3[loadIndex][rpmIndex] = config->gpPwm3LoadBins[loadIndex];
		}
	}
	for (int loadIndex = 0; loadIndex < GP_PWM_LOAD_COUNT; loadIndex++) {
		for (int rpmIndex = 0; rpmIndex < GP_PWM_RPM_COUNT; rpmIndex++) {
			config->gpPwmTable4[loadIndex][rpmIndex] = config->gpPwm4LoadBins[loadIndex];
		}
	}
}
static void turnGpPwm1On() {
	if (engineConfiguration->gppwmIo.gpPwm1Pin == GPIO_UNASSIGNED) {
		return;
	}
	startSimplePwmExt(&gpPwm1, "gpPwm1", &engine->executor, CONFIG(gppwmIo.gpPwm1Pin),
			&enginePins.gp1Pin, engineConfiguration->gppwm.gpPwm1Frequency, 0.1f,
			(pwm_gen_callback*) applyPinState);
}

static void turnGpPwm2On() {
	if (CONFIG(gppwmIo.gpPwm2Pin) == GPIO_UNASSIGNED) {
		return;
	}
	startSimplePwmExt(&gpPwm2, "gpPwm2", &engine->executor, CONFIG(gppwmIo.gpPwm2Pin),
			&enginePins.gp2Pin, engineConfiguration->gppwm.gpPwm2Frequency, 0.1f,
			(pwm_gen_callback*) applyPinState);
}

static void turnGpPwm3On() {
	if (CONFIG(gppwmIo.gpPwm3Pin) == GPIO_UNASSIGNED) {
		return;
	}
	startSimplePwmExt(&gpPwm3, "gpPwm3", &engine->executor, CONFIG(gppwmIo.gpPwm3Pin),
			&enginePins.gp3Pin, engineConfiguration->gppwm.gpPwm3Frequency, 0.1f,
			(pwm_gen_callback*) applyPinState);
}

static void turnGpPwm4On() {
	if (CONFIG(gppwmIo.gpPwm4Pin) == GPIO_UNASSIGNED) {
		return;
	}
	startSimplePwmExt(&gpPwm4, "gpPwm4", &engine->executor, CONFIG(gppwmIo.gpPwm4Pin),
			&enginePins.gp4Pin, engineConfiguration->gppwm.gpPwm4Frequency, 0.1f,
			(pwm_gen_callback*) applyPinState);
}

void startGpPwmPins(void) {

	turnGpPwm1On();
	turnGpPwm2On();
	turnGpPwm3On();
	turnGpPwm4On();
}
void stopGpPwmPins(void) {
	brain_pin_markUnused(activeConfiguration.gppwmIo.gpPwm1Pin);
	brain_pin_markUnused(activeConfiguration.gppwmIo.gpPwm2Pin);
	brain_pin_markUnused(activeConfiguration.gppwmIo.gpPwm3Pin);
	brain_pin_markUnused(activeConfiguration.gppwmIo.gpPwm4Pin);

}


void initGpPwmCtrl(Logging *sharedLogger DECLARE_ENGINE_PARAMETER_SUFFIX) {
	logger = sharedLogger;
	gpPwmMap1.init(config->gpPwmTable1, config->gpPwm1LoadBins, config->gpPwm1RpmBins);
	gpPwmMap2.init(config->gpPwmTable2, config->gpPwm2LoadBins, config->gpPwm2RpmBins);
	gpPwmMap3.init(config->gpPwmTable3, config->gpPwm3LoadBins, config->gpPwm3RpmBins);
	gpPwmMap4.init(config->gpPwmTable4, config->gpPwm4LoadBins, config->gpPwm4RpmBins);

	stopGpPwmPins();
	startGpPwmPins();
	GpPWMController.Start();

}
#endif
