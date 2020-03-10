/*
 * launch_control.h
 *
 *  Created on: 3. feb. 2020
 *      Author: Ola Ruud
 */
#pragma once

#include "engine.h"
#include "periodic_task.h"


void setDefaultLaunchParameters(DECLARE_CONFIG_PARAMETER_SIGNATURE);
void applyLaunchControlLimiting(bool *limitedSpark, bool *limitedFuel DECLARE_ENGINE_PARAMETER_SUFFIX);
void initLaunchControl(Logging *sharedLogger DECLARE_ENGINE_PARAMETER_SUFFIX);
void setLaunchBoost(void);
void applyLaunchControlAdvance(DECLARE_ENGINE_PARAMETER_SUFFIX);
