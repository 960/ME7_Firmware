/*
 * @file launch_control.h
 *
 * @date 10. sep. 2019
 * (C) All rights reserved by RUUD BILELEKTRO, NORWAY
 */

#pragma once

#include "engine.h"

void initLaunchControl( DECLARE_ENGINE_PARAMETER_SUFFIX);
void initAntiLag( DECLARE_ENGINE_PARAMETER_SUFFIX);
void setDefaultAntiLagParameters(DECLARE_CONFIG_PARAMETER_SIGNATURE);
void setDefaultLaunchParameters(DECLARE_CONFIG_PARAMETER_SIGNATURE);

