/**
 * @file	engine_math.cpp
 * @brief
 *
 * @date Jul 13, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 *
 * This file is part of rusEfi - see http://rusefi.com
 *
 * rusEfi is free software; you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * rusEfi is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "global.h"
#include "engine_math.h"
#include "engine_configuration.h"
#include "interpolation.h"
#include "allsensors.h"
#include "sensor.h"
#include "event_registry.h"
#include "efi_gpio.h"
#include "fuel_math.h"
#include "advance_map.h"

EXTERN_ENGINE;

floatms_t getEngineCycleDuration(int rpm DECLARE_ENGINE_PARAMETER_SUFFIX) {
	return getCrankshaftRevolutionTimeMs(rpm) * (engine->getOperationMode(PASS_ENGINE_PARAMETER_SIGNATURE) == TWO_STROKE ? 1 : 2);
}

/**
 * @return number of milliseconds in one crank shaft revolution
 */
floatms_t getCrankshaftRevolutionTimeMs(int rpm) {
	if (rpm == 0) {
		return NAN;
	}
	return 360 * getOneDegreeTimeMs(rpm);
}

float getFuelingLoad(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	return ENGINE(engineState.fuelingLoad);
}

float getIgnitionLoad(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	return ENGINE(engineState.ignitionLoad);
}

/**
 * @brief Returns engine load according to selected engine_load_mode
 *
 */
float getEngineLoadT(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	efiAssert(CUSTOM_ERR_ASSERT, engine!=NULL, "engine 2NULL", NAN);
	efiAssert(CUSTOM_ERR_ASSERT, engineConfiguration!=NULL, "engineConfiguration 2NULL", NAN);
	switch (engineConfiguration->fuelAlgorithm) {
	case LM_SPEED_DENSITY:
		return getMap(PASS_ENGINE_PARAMETER_SIGNATURE);
	case LM_ALPHA_N_2:
		return Sensor::get(SensorType::Tps1).value_or(0);
	case LM_REAL_MAF:
		return getRealMaf(PASS_ENGINE_PARAMETER_SIGNATURE);
	default:
		firmwareError(CUSTOM_UNKNOWN_ALGORITHM, "Unexpected engine load parameter: %d", engineConfiguration->fuelAlgorithm);
		return 0;
	}
}

/**
 * see also setConstantDwell
 */
void setSingleCoilDwell(DECLARE_CONFIG_PARAMETER_SIGNATURE) {
	for (int i = 0; i < DWELL_CURVE_SIZE; i++) {
		engineConfiguration->sparkDwellRpmBins[i] = i + 1;
		engineConfiguration->sparkDwellValues[i] = 4;
	}

	engineConfiguration->sparkDwellRpmBins[5] = 10;
	engineConfiguration->sparkDwellValues[5] = 4;

	engineConfiguration->sparkDwellRpmBins[6] = 4500;
	engineConfiguration->sparkDwellValues[6] = 4;

	engineConfiguration->sparkDwellRpmBins[7] = 12500;
	engineConfiguration->sparkDwellValues[7] = 0;
}

static floatms_t getCrankingSparkDwell(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	if (engineConfiguration->enableFixedDwellCranking) {
		return engineConfiguration->ignitionDwellForCrankingMs;
	} else {
		// technically this could be implemented via interpolate2d
		float angle = engineConfiguration->crankingChargeAngle;
		return getOneDegreeTimeMs(GET_RPM()) * angle;
	}
}

/**
 * @return Spark dwell time, in milliseconds. 0 if tables are not ready.
 */
floatms_t getSparkDwell(int rpm DECLARE_ENGINE_PARAMETER_SUFFIX) {

	float dwellMs;
	if (ENGINE(rpmCalculator).isCranking(PASS_ENGINE_PARAMETER_SIGNATURE)) {
		dwellMs = getCrankingSparkDwell(PASS_ENGINE_PARAMETER_SIGNATURE);
	} else {
		efiAssert(CUSTOM_ERR_ASSERT, !cisnan(rpm), "invalid rpm", NAN);

		dwellMs = interpolate2d("dwell", rpm, engineConfiguration->sparkDwellRpmBins, engineConfiguration->sparkDwellValues);
	}

	if (cisnan(dwellMs) || dwellMs <= 0) {
		// this could happen during engine configuration reset
		warning(CUSTOM_ERR_DWELL_DURATION, "invalid dwell: %.2f at rpm=%d", dwellMs, rpm);
		return 0;
	}
	return dwellMs;

}


static const int order_1_2[] = {1, 2};

static const int order_1_2_3[] = {1, 2, 3};
static const int order_1_3_2[] = {1, 3, 2};
// 4 cylinder

static const int order_1_THEN_3_THEN_4_THEN2[] = { 1, 3, 4, 2 };
static const int order_1_THEN_2_THEN_4_THEN3[] = { 1, 2, 4, 3 };
static const int order_1_THEN_3_THEN_2_THEN4[] = { 1, 3, 2, 4 };
static const int order_1_THEN_4_THEN_3_THEN2[] = { 1, 4, 3, 2 };

// 5 cylinder
static const int order_1_2_4_5_3[] = {1, 2, 4, 5, 3};

// 6 cylinder
static const int order_1_THEN_5_THEN_3_THEN_6_THEN_2_THEN_4[] = { 1, 5, 3, 6, 2, 4 };
static const int order_1_THEN_4_THEN_2_THEN_5_THEN_3_THEN_6[] = { 1, 4, 2, 5, 3, 6 };
static const int order_1_THEN_2_THEN_3_THEN_4_THEN_5_THEN_6[] = { 1, 2, 3, 4, 5, 6 };
static const int order_1_6_3_2_5_4[] = {1, 6, 3, 2, 5, 4};

// 8 cylinder
static const int order_1_8_4_3_6_5_7_2[] = { 1, 8, 4, 3, 6, 5, 7, 2 };
static const int order_1_8_7_2_6_5_4_3[] = { 1, 8, 7, 2, 6, 5, 4, 3 };
static const int order_1_5_4_2_6_3_7_8[] = { 1, 5, 4, 2, 6, 3, 7, 8 };
static const int order_1_2_7_8_4_5_6_3[] = { 1, 2, 7, 8, 4, 5, 6, 3 };
static const int order_1_3_7_2_6_5_4_8[] = { 1, 3, 7, 2, 6, 5, 4, 8 };

static int getFiringOrderLength(DECLARE_ENGINE_PARAMETER_SIGNATURE) {

	switch (CONFIG(specs.firingOrder)) {
	case FO_1:
		return 1;
// 2 cylinder
	case FO_1_2:
		return 2;
// 3 cylinder
	case FO_1_2_3:
	case FO_1_3_2:
		return 3;
// 4 cylinder
	case FO_1_3_4_2:
	case FO_1_2_4_3:
	case FO_1_3_2_4:
	case FO_1_4_3_2:
		return 4;
// 5 cylinder
	case FO_1_2_4_5_3:
		return 5;

// 6 cylinder
	case FO_1_5_3_6_2_4:
	case FO_1_4_2_5_3_6:
	case FO_1_2_3_4_5_6:
	case FO_1_6_3_2_5_4:
		return 6;

// 8 cylinder
	case FO_1_8_4_3_6_5_7_2:
	case FO_1_8_7_2_6_5_4_3:
	case FO_1_5_4_2_6_3_7_8:
	case FO_1_2_7_8_4_5_6_3:
	case FO_1_3_7_2_6_5_4_8:
		return 8;
	default:
		warning(CUSTOM_OBD_UNKNOWN_FIRING_ORDER, "getCylinderId not supported for %d", CONFIG(specs.firingOrder));
	}
	return 1;
}


/**
 * @param index from zero to cylindersCount - 1
 * @return cylinderId from one to cylindersCount
 */
int getCylinderId(int index DECLARE_ENGINE_PARAMETER_SUFFIX) {

	const int firingOrderLength = getFiringOrderLength(PASS_ENGINE_PARAMETER_SIGNATURE);

	if (firingOrderLength < 1 || firingOrderLength > INJECTION_PIN_COUNT) {
		warning(CUSTOM_ERR_6687, "fol %d", firingOrderLength);
		return 1;
	}
	if (engineConfiguration->specs.cylindersCount != firingOrderLength) {
		// May 2020 this somehow still happens with functional tests, maybe race condition?
		warning(CUSTOM_OBD_WRONG_FIRING_ORDER, "Wrong cyl count for firing order, expected %d cylinders", firingOrderLength);
		return 1;
	}

	if (index < 0 || index >= firingOrderLength) {
		// May 2020 this somehow still happens with functional tests, maybe race condition?
		warning(CUSTOM_ERR_6686, "firing order index %d", index);
		return 1;
	}

	switch (CONFIG(specs.firingOrder)) {
	case FO_1:
		return 1;
// 2 cylinder
	case FO_1_2:
		return order_1_2[index];
// 3 cylinder
	case FO_1_2_3:
		return order_1_2_3[index];
	case FO_1_3_2:
		return order_1_3_2[index];
// 4 cylinder
	case FO_1_3_4_2:
		return order_1_THEN_3_THEN_4_THEN2[index];
	case FO_1_2_4_3:
		return order_1_THEN_2_THEN_4_THEN3[index];
	case FO_1_3_2_4:
		return order_1_THEN_3_THEN_2_THEN4[index];
	case FO_1_4_3_2:
		return order_1_THEN_4_THEN_3_THEN2[index];
// 5 cylinder
	case FO_1_2_4_5_3:
		return order_1_2_4_5_3[index];

// 6 cylinder
	case FO_1_5_3_6_2_4:
		return order_1_THEN_5_THEN_3_THEN_6_THEN_2_THEN_4[index];
	case FO_1_4_2_5_3_6:
		return order_1_THEN_4_THEN_2_THEN_5_THEN_3_THEN_6[index];
	case FO_1_2_3_4_5_6:
		return order_1_THEN_2_THEN_3_THEN_4_THEN_5_THEN_6[index];
	case FO_1_6_3_2_5_4:
		return order_1_6_3_2_5_4[index];

// 8 cylinder
	case FO_1_8_4_3_6_5_7_2:
		return order_1_8_4_3_6_5_7_2[index];
	case FO_1_8_7_2_6_5_4_3:
		return order_1_8_7_2_6_5_4_3[index];
	case FO_1_5_4_2_6_3_7_8:
		return order_1_5_4_2_6_3_7_8[index];
	case FO_1_2_7_8_4_5_6_3:
		return order_1_2_7_8_4_5_6_3[index];
	case FO_1_3_7_2_6_5_4_8:
		return order_1_3_7_2_6_5_4_8[index];


	default:
		warning(CUSTOM_OBD_UNKNOWN_FIRING_ORDER, "getCylinderId not supported for %d", CONFIG(specs.firingOrder));
	}
	return 1;
}

/**
 * @param cylinderIndex from 0 to cylinderCount, not cylinder number
 */
static int getIgnitionPinForIndex(int cylinderIndex DECLARE_ENGINE_PARAMETER_SUFFIX) {
	switch (getCurrentIgnitionMode(PASS_ENGINE_PARAMETER_SIGNATURE)) {
	case IM_ONE_COIL:
		return 0;
	case IM_WASTED_SPARK: {
		if (CONFIG(specs.cylindersCount) == 1) {
			// we do not want to divide by zero
			return 0;
		}
		return cylinderIndex % (CONFIG(specs.cylindersCount) / 2);
	}
	case IM_INDIVIDUAL_COILS:
		return cylinderIndex;
	case IM_TWO_COILS:
		return cylinderIndex % 2;

	default:
		return 0;
	}
}

void prepareIgnitionPinIndices(ignition_mode_e ignitionMode DECLARE_ENGINE_PARAMETER_SUFFIX) {
	if (ignitionMode != engine->ignitionModeForPinIndices) {

		for (int cylinderIndex = 0; cylinderIndex < CONFIG(specs.cylindersCount); cylinderIndex++) {
			ENGINE(ignitionPin[cylinderIndex]) = getIgnitionPinForIndex(cylinderIndex PASS_ENGINE_PARAMETER_SUFFIX);
		}

		engine->ignitionModeForPinIndices = ignitionMode;
	}
}

/**
 * @return IM_WASTED_SPARK if in SPINNING mode and IM_INDIVIDUAL_COILS setting
 * @return CONFIG(ignitionMode) otherwise
 */
ignition_mode_e getCurrentIgnitionMode(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	ignition_mode_e ignitionMode = CONFIG(ignitionMode);

	// In spin-up cranking mode we don't have full phase sync. info yet, so wasted spark mode is better
	if (ignitionMode == IM_INDIVIDUAL_COILS && ENGINE(rpmCalculator.isSpinningUp(PASS_ENGINE_PARAMETER_SIGNATURE)))
		ignitionMode = IM_WASTED_SPARK;

	return ignitionMode;
}



/**
 * This heavy method is only invoked in case of a configuration change or initialization.
 */
void prepareOutputSignals(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	ENGINE(engineCycle) = getEngineCycle(engine->getOperationMode(PASS_ENGINE_PARAMETER_SIGNATURE));

	angle_t maxTimingCorrMap = -720.0f;
	angle_t maxTimingMap = -720.0f;
	for (int rpmIndex = 0;rpmIndex<IGN_RPM_COUNT;rpmIndex++) {
		for (int l = 0;l<IGN_LOAD_COUNT;l++) {
			maxTimingCorrMap = maxF(maxTimingCorrMap, config->ignitionIatCorrTable[l][rpmIndex]);
			maxTimingMap = maxF(maxTimingMap, config->advanceTable[l][rpmIndex]);
		}
	}


	for (int i = 0; i < CONFIG(specs.cylindersCount); i++) {
		ENGINE(ignitionPositionWithinEngineCycle[i]) = ENGINE(engineCycle) * i / CONFIG(specs.cylindersCount);
	}

	prepareIgnitionPinIndices(CONFIG(ignitionMode) PASS_ENGINE_PARAMETER_SUFFIX);

	TRIGGER_WAVEFORM(prepareShape(&ENGINE(triggerCentral.triggerFormDetails) PASS_ENGINE_PARAMETER_SUFFIX));
}

void setTimingRpmBin(float from, float to DECLARE_CONFIG_PARAMETER_SUFFIX) {
	setRpmBin(config->srpm_table, IGN_RPM_COUNT, from, to);
}

void setTimingLoadBin(float from, float to DECLARE_CONFIG_PARAMETER_SUFFIX) {
	setLinearCurve(config->smap_table, from, to);
}

/**
 * this method sets algorithm and ignition table scale
 */
void setAlgorithm(engine_load_mode_e algo DECLARE_CONFIG_PARAMETER_SUFFIX) {
	engineConfiguration->fuelAlgorithm = algo;
	if (algo == LM_SPEED_DENSITY) {
		setLinearCurve(config->smap_table, 20, 120, 3);
		buildTimingMap(35 PASS_CONFIG_PARAMETER_SUFFIX);
	}
}

void setFlatInjectorLag(float value DECLARE_CONFIG_PARAMETER_SUFFIX) {
	setArrayValues(engineConfiguration->injector.battLagCorr, value);
}


