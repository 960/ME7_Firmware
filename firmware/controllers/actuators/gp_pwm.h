/*
 * gp_pwm.h
 *
 *  Created on: 19. des. 2019
 *      Author: Ola
 */
#pragma once

#include "engine.h"
#include "periodic_task.h"


void stopGpPwmPins(void);
void startGpPwmPins(void);
void initGpPwmCtrl(Logging *sharedLogger);
void setDefaultGpPwmParameters(DECLARE_CONFIG_PARAMETER_SIGNATURE);
void onConfigurationChangeGpPwmCallback(engine_configuration_s *previousConfiguration);











