/**
 * @file	engine_configuration.cpp
 * @brief	Utility method related to the engine configuration data structure.
 *
 * @date Nov 22, 2013
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
 *
 */

#include "global.h"
#include "os_access.h"
#include "engine_configuration.h"

#include "allsensors.h"
#include "interpolation.h"
#include "engine_math.h"
#include "speed_density.h"
#include "advance_map.h"
#include "sensor.h"
#include "can_hw.h"

#include "boost_control.h"
#if EFI_IDLE_CONTROL
#include "idle_thread.h"
#endif /* EFI_IDLE_CONTROL */

#if EFI_ALTERNATOR_CONTROL
#include "alternator_controller.h"
#endif

#if EFI_ELECTRONIC_THROTTLE_BODY
#include "electronic_throttle.h"
#endif
#if EFI_PROD_CODE
#include "init.h"
#include "hardware.h"
#include "board.h"
#endif /* EFI_PROD_CODE */

#if EFI_EMULATE_POSITION_SENSORS
#include "trigger_emulator_algo.h"
#endif /* EFI_EMULATE_POSITION_SENSORS */

#if EFI_LAUNCH_CONTROL
#include "launch_control.h"
#endif


#if EFI_ANTILAG
#include "antilag.h"
#endif

#if EFI_VVT_CONTROL
#include "vvt_control.h"
#endif

#if EFI_TUNER_STUDIO
#include "tunerstudio.h"
#endif

EXTERN_ENGINE;

//#define TS_DEFAULT_SPEED 115200
#define TS_DEFAULT_SPEED 38400

#define xxxxx 0

#if 0
static fuel_table_t alphaNfuel = {
		{/*0  fuelingLoad=0.00*/   /*0 800.0*/xxxxx, /*1 1213.0*/xxxxx, /*2 1626.0*/xxxxx, /*3 2040.0*/xxxxx, /*4 2453.0*/xxxxx, /*5 2866.0*/xxxxx, /*6 3280.0*/xxxxx, /*7 3693.0*/xxxxx, /*8 4106.0*/xxxxx, /*9 4520.0*/xxxxx, /*10 4933.0*/xxxxx, /*11 5346.0*/xxxxx, /*12 5760.0*/xxxxx, /*13 6173.0*/xxxxx, /*14 6586.0*/xxxxx, /*15 7000.0*/xxxxx},
		{/*1  fuelingLoad=6.66*/   /*0 800.0*/xxxxx, /*1 1213.0*/xxxxx, /*2 1626.0*/xxxxx, /*3 2040.0*/xxxxx, /*4 2453.0*/xxxxx, /*5 2866.0*/xxxxx, /*6 3280.0*/xxxxx, /*7 3693.0*/xxxxx, /*8 4106.0*/xxxxx, /*9 4520.0*/xxxxx, /*10 4933.0*/xxxxx, /*11 5346.0*/xxxxx, /*12 5760.0*/xxxxx, /*13 6173.0*/xxxxx, /*14 6586.0*/xxxxx, /*15 7000.0*/xxxxx},
		{/*2  fuelingLoad=13.33*/  /*0 800.0*/xxxxx, /*1 1213.0*/xxxxx, /*2 1626.0*/xxxxx, /*3 2040.0*/xxxxx, /*4 2453.0*/xxxxx, /*5 2866.0*/xxxxx, /*6 3280.0*/xxxxx, /*7 3693.0*/xxxxx, /*8 4106.0*/xxxxx, /*9 4520.0*/xxxxx, /*10 4933.0*/xxxxx, /*11 5346.0*/xxxxx, /*12 5760.0*/xxxxx, /*13 6173.0*/xxxxx, /*14 6586.0*/xxxxx, /*15 7000.0*/xxxxx},
		{/*3  fuelingLoad=20.00*/  /*0 800.0*/xxxxx, /*1 1213.0*/xxxxx, /*2 1626.0*/xxxxx, /*3 2040.0*/xxxxx, /*4 2453.0*/xxxxx, /*5 2866.0*/xxxxx, /*6 3280.0*/xxxxx, /*7 3693.0*/xxxxx, /*8 4106.0*/xxxxx, /*9 4520.0*/xxxxx, /*10 4933.0*/xxxxx, /*11 5346.0*/xxxxx, /*12 5760.0*/xxxxx, /*13 6173.0*/xxxxx, /*14 6586.0*/xxxxx, /*15 7000.0*/xxxxx},
		{/*4  fuelingLoad=26.66*/  /*0 800.0*/xxxxx, /*1 1213.0*/xxxxx, /*2 1626.0*/xxxxx, /*3 2040.0*/xxxxx, /*4 2453.0*/xxxxx, /*5 2866.0*/xxxxx, /*6 3280.0*/xxxxx, /*7 3693.0*/xxxxx, /*8 4106.0*/xxxxx, /*9 4520.0*/xxxxx, /*10 4933.0*/xxxxx, /*11 5346.0*/xxxxx, /*12 5760.0*/xxxxx, /*13 6173.0*/xxxxx, /*14 6586.0*/xxxxx, /*15 7000.0*/xxxxx},
		{/*5  fuelingLoad=33.33*/  /*0 800.0*/xxxxx, /*1 1213.0*/xxxxx, /*2 1626.0*/xxxxx, /*3 2040.0*/xxxxx, /*4 2453.0*/xxxxx, /*5 2866.0*/xxxxx, /*6 3280.0*/xxxxx, /*7 3693.0*/xxxxx, /*8 4106.0*/xxxxx, /*9 4520.0*/xxxxx, /*10 4933.0*/xxxxx, /*11 5346.0*/xxxxx, /*12 5760.0*/xxxxx, /*13 6173.0*/xxxxx, /*14 6586.0*/xxxxx, /*15 7000.0*/xxxxx},
		{/*6  fuelingLoad=40.00*/  /*0 800.0*/xxxxx, /*1 1213.0*/xxxxx, /*2 1626.0*/xxxxx, /*3 2040.0*/xxxxx, /*4 2453.0*/xxxxx, /*5 2866.0*/xxxxx, /*6 3280.0*/xxxxx, /*7 3693.0*/xxxxx, /*8 4106.0*/xxxxx, /*9 4520.0*/xxxxx, /*10 4933.0*/xxxxx, /*11 5346.0*/xxxxx, /*12 5760.0*/xxxxx, /*13 6173.0*/xxxxx, /*14 6586.0*/xxxxx, /*15 7000.0*/xxxxx},
		{/*7  fuelingLoad=46.66*/  /*0 800.0*/xxxxx, /*1 1213.0*/xxxxx, /*2 1626.0*/xxxxx, /*3 2040.0*/xxxxx, /*4 2453.0*/xxxxx, /*5 2866.0*/xxxxx, /*6 3280.0*/xxxxx, /*7 3693.0*/xxxxx, /*8 4106.0*/xxxxx, /*9 4520.0*/xxxxx, /*10 4933.0*/xxxxx, /*11 5346.0*/xxxxx, /*12 5760.0*/xxxxx, /*13 6173.0*/xxxxx, /*14 6586.0*/xxxxx, /*15 7000.0*/xxxxx},
		{/*8  fuelingLoad=53.33*/  /*0 800.0*/xxxxx, /*1 1213.0*/xxxxx, /*2 1626.0*/xxxxx, /*3 2040.0*/xxxxx, /*4 2453.0*/xxxxx, /*5 2866.0*/xxxxx, /*6 3280.0*/xxxxx, /*7 3693.0*/xxxxx, /*8 4106.0*/xxxxx, /*9 4520.0*/xxxxx, /*10 4933.0*/xxxxx, /*11 5346.0*/xxxxx, /*12 5760.0*/xxxxx, /*13 6173.0*/xxxxx, /*14 6586.0*/xxxxx, /*15 7000.0*/xxxxx},
		{/*9  fuelingLoad=60.00*/  /*0 800.0*/xxxxx, /*1 1213.0*/xxxxx, /*2 1626.0*/xxxxx, /*3 2040.0*/xxxxx, /*4 2453.0*/xxxxx, /*5 2866.0*/xxxxx, /*6 3280.0*/xxxxx, /*7 3693.0*/xxxxx, /*8 4106.0*/xxxxx, /*9 4520.0*/xxxxx, /*10 4933.0*/xxxxx, /*11 5346.0*/xxxxx, /*12 5760.0*/xxxxx, /*13 6173.0*/xxxxx, /*14 6586.0*/xxxxx, /*15 7000.0*/xxxxx},
		{/*10 fuelingLoad=66.66*/  /*0 800.0*/xxxxx, /*1 1213.0*/xxxxx, /*2 1626.0*/xxxxx, /*3 2040.0*/xxxxx, /*4 2453.0*/xxxxx, /*5 2866.0*/xxxxx, /*6 3280.0*/xxxxx, /*7 3693.0*/xxxxx, /*8 4106.0*/xxxxx, /*9 4520.0*/xxxxx, /*10 4933.0*/xxxxx, /*11 5346.0*/xxxxx, /*12 5760.0*/xxxxx, /*13 6173.0*/xxxxx, /*14 6586.0*/xxxxx, /*15 7000.0*/xxxxx},
		{/*11 fuelingLoad=73.33*/  /*0 800.0*/xxxxx, /*1 1213.0*/xxxxx, /*2 1626.0*/xxxxx, /*3 2040.0*/xxxxx, /*4 2453.0*/xxxxx, /*5 2866.0*/xxxxx, /*6 3280.0*/xxxxx, /*7 3693.0*/xxxxx, /*8 4106.0*/xxxxx, /*9 4520.0*/xxxxx, /*10 4933.0*/xxxxx, /*11 5346.0*/xxxxx, /*12 5760.0*/xxxxx, /*13 6173.0*/xxxxx, /*14 6586.0*/xxxxx, /*15 7000.0*/xxxxx},
		{/*12 fuelingLoad=80.00*/  /*0 800.0*/xxxxx, /*1 1213.0*/xxxxx, /*2 1626.0*/xxxxx, /*3 2040.0*/xxxxx, /*4 2453.0*/xxxxx, /*5 2866.0*/xxxxx, /*6 3280.0*/xxxxx, /*7 3693.0*/xxxxx, /*8 4106.0*/xxxxx, /*9 4520.0*/xxxxx, /*10 4933.0*/xxxxx, /*11 5346.0*/xxxxx, /*12 5760.0*/xxxxx, /*13 6173.0*/xxxxx, /*14 6586.0*/xxxxx, /*15 7000.0*/xxxxx},
		{/*13 fuelingLoad=86.66*/  /*0 800.0*/xxxxx, /*1 1213.0*/xxxxx, /*2 1626.0*/xxxxx, /*3 2040.0*/xxxxx, /*4 2453.0*/xxxxx, /*5 2866.0*/xxxxx, /*6 3280.0*/xxxxx, /*7 3693.0*/xxxxx, /*8 4106.0*/xxxxx, /*9 4520.0*/xxxxx, /*10 4933.0*/xxxxx, /*11 5346.0*/xxxxx, /*12 5760.0*/xxxxx, /*13 6173.0*/xxxxx, /*14 6586.0*/xxxxx, /*15 7000.0*/xxxxx},
		{/*14 fuelingLoad=93.33*/  /*0 800.0*/xxxxx, /*1 1213.0*/xxxxx, /*2 1626.0*/xxxxx, /*3 2040.0*/xxxxx, /*4 2453.0*/xxxxx, /*5 2866.0*/xxxxx, /*6 3280.0*/xxxxx, /*7 3693.0*/xxxxx, /*8 4106.0*/xxxxx, /*9 4520.0*/xxxxx, /*10 4933.0*/xxxxx, /*11 5346.0*/xxxxx, /*12 5760.0*/xxxxx, /*13 6173.0*/xxxxx, /*14 6586.0*/xxxxx, /*15 7000.0*/xxxxx},
		{/*15 fuelingLoad=100.00*/ /*0 800.0*/xxxxx, /*1 1213.0*/xxxxx, /*2 1626.0*/xxxxx, /*3 2040.0*/xxxxx, /*4 2453.0*/xxxxx, /*5 2866.0*/xxxxx, /*6 3280.0*/xxxxx, /*7 3693.0*/xxxxx, /*8 4106.0*/xxxxx, /*9 4520.0*/xxxxx, /*10 4933.0*/xxxxx, /*11 5346.0*/xxxxx, /*12 5760.0*/xxxxx, /*13 6173.0*/xxxxx, /*14 6586.0*/xxxxx, /*15 7000.0*/xxxxx}
		};
#endif

/**
 * Current engine configuration. On firmware start we assign empty configuration, then
 * we copy actual configuration after reading settings.
 * This is useful to compare old and new configurations in order to apply new settings.
 *
 * todo: place this field next to 'engineConfiguration'?
 */
#ifdef EFI_ACTIVE_CONFIGURATION_IN_FLASH
#include "flash_int.h"
engine_configuration_s & activeConfiguration = reinterpret_cast<persistent_config_container_s*>(getFlashAddrFirstCopy())->persistentConfiguration.engineConfiguration;
// we cannot use this activeConfiguration until we call rememberCurrentConfiguration()
bool isActiveConfigurationVoid = true;
#else
static engine_configuration_s activeConfigurationLocalStorage;
engine_configuration_s & activeConfiguration = activeConfigurationLocalStorage;
#endif /* EFI_ACTIVE_CONFIGURATION_IN_FLASH */

extern engine_configuration_s *engineConfiguration;

void rememberCurrentConfiguration(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
#ifndef EFI_ACTIVE_CONFIGURATION_IN_FLASH
	memcpy(&activeConfiguration, engineConfiguration, sizeof(engine_configuration_s));
#else
	isActiveConfigurationVoid = false;
#endif /* EFI_ACTIVE_CONFIGURATION_IN_FLASH */
}



/**
 * this is the top-level method which should be called in case of any changes to engine configuration
 * online tuning of most values in the maps does not count as configuration change, but 'Burn' command does
 *
 * this method is NOT currently invoked on ECU start - actual user input has to happen!
 */
void incrementGlobalConfigurationVersion(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	ENGINE(globalConfigurationVersion++);

/**
 * All these callbacks could be implemented as listeners, but these days I am saving RAM
 */
#if EFI_PROD_CODE
	applyNewHardwareSettings();
	reconfigureSensors();
#endif /* EFI_PROD_CODE */
	engine->preCalculate(PASS_ENGINE_PARAMETER_SIGNATURE);
#if EFI_ALTERNATOR_CONTROL
	onConfigurationChangeAlternatorCallback(&activeConfiguration);
#endif /* EFI_ALTERNATOR_CONTROL */

#if EFI_BOOST_CONTROL
	onConfigurationChangeBoostCallback(&activeConfiguration);
#endif
#if EFI_ELECTRONIC_THROTTLE_BODY
	onConfigurationChangeElectronicThrottleCallback(&activeConfiguration);
#endif /* EFI_ELECTRONIC_THROTTLE_BODY */

#if EFI_IDLE_CONTROL && ! EFI_UNIT_TEST
	onConfigurationChangeIdleCallback(&activeConfiguration);
#endif /* EFI_IDLE_CONTROL */


	onConfigurationChangeTriggerCallback(PASS_ENGINE_PARAMETER_SIGNATURE);

#if EFI_EMULATE_POSITION_SENSORS
	onConfigurationChangeRpmEmulatorCallback(&activeConfiguration);
#endif /* EFI_EMULATE_POSITION_SENSORS */

#if EFI_FSIO
	onConfigurationChangeFsioCallback(&activeConfiguration PASS_ENGINE_PARAMETER_SUFFIX);
#endif /* EFI_FSIO */
	rememberCurrentConfiguration(PASS_ENGINE_PARAMETER_SIGNATURE);
}

/**
 * @brief Sets the same dwell time across the whole getRpm() range
 * set dwell X
 */
void setConstantDwell(floatms_t dwellMs DECLARE_CONFIG_PARAMETER_SUFFIX) {
	for (int i = 0; i < DWELL_CURVE_SIZE; i++) {
		engineConfiguration->sparkDwellRpmBins[i] = 1000 * i;
	}
	setLinearCurve(engineConfiguration->sparkDwellValues, dwellMs, dwellMs, 0.01);
}

void setAfrMap(afr_table_t table, float value) {
	for (int l = 0; l < FUEL_LOAD_COUNT; l++) {
		for (int rpmIndex = 0; rpmIndex < FUEL_RPM_COUNT; rpmIndex++) {
			table[l][rpmIndex] = (int)(value * AFR_STORAGE_MULT);
		}
	}
}

// todo: make this a template
void setMap(fuel_table_t table, float value) {
	for (int l = 0; l < FUEL_LOAD_COUNT; l++) {
		for (int rpmIndex = 0; rpmIndex < FUEL_RPM_COUNT; rpmIndex++) {
			table[l][rpmIndex] = value;
		}
	}
}

#if 0
static void setWholeVEMap(float value DECLARE_CONFIG_PARAMETER_SUFFIX) {
	setMap(config->veTable, value);
}
#endif


void setWholeIgnitionIatCorr(float value DECLARE_CONFIG_PARAMETER_SUFFIX) {
#if (IGN_LOAD_COUNT == FUEL_LOAD_COUNT) && (IGN_RPM_COUNT == FUEL_RPM_COUNT)
	// todo: make setMap a template
	setMap(config->ignitionIatCorrTable, value);
#else
	UNUSED(value);
#endif
}

void setFuelTablesLoadBin(float minValue, float maxValue DECLARE_CONFIG_PARAMETER_SUFFIX) {
	setLinearCurve(config->injPhaseLoadBins, minValue, maxValue, 1);
	setLinearCurve(config->veLoadBins, minValue, maxValue, 1);
	setLinearCurve(config->afrLoadBins, minValue, maxValue, 1);
}

void setTimingMap(ignition_table_t map, float value) {
	for (int l = 0; l < IGN_LOAD_COUNT; l++) {
		for (int r = 0; r < IGN_RPM_COUNT; r++) {
			map[l][r] = value;
		}
	}
}

void setWholeIatCorrTimingTable(float value DECLARE_CONFIG_PARAMETER_SUFFIX) {
	setTimingMap(config->ignitionIatCorrTable, value);
}

/**
 * See also fixedCrankingTiming
 */
void setWholeTimingTable_d(angle_t value DECLARE_CONFIG_PARAMETER_SUFFIX) {
	setTimingMap(config->ignitionTable, value);
}

static void initTemperatureCurve(float *bins, float *values, int size, float defaultValue) {
	for (int i = 0; i < size; i++) {
		bins[i] = -40 + i * 10;
		values[i] = defaultValue; // this correction is a multiplier
	}
}

void prepareVoidConfiguration(engine_configuration_s *engineConfiguration) {
	efiAssertVoid(OBD_PCM_Processor_Fault, engineConfiguration != NULL, "ec NULL");
	memset(engineConfiguration, 0, sizeof(engine_configuration_s));
	

	// Now that GPIO_UNASSIGNED == 0 we do not really need explicit zero assignments since memset above does that
	// todo: migrate 'EFI_ADC_NONE' to '0' and eliminate the need in this method altogether
	engineConfiguration->debugTriggerSync = GPIO_UNASSIGNED;
	engineConfiguration->triggerErrorPin = GPIO_UNASSIGNED;
	engineConfiguration->pinMainRelay = GPIO_UNASSIGNED;
	engineConfiguration->clt.adcChannel = EFI_ADC_NONE;
	engineConfiguration->iat.adcChannel = EFI_ADC_NONE;
	setPinConfigurationOverrides(PASS_ENGINE_PARAMETER_SUFFIX);
	engine->cj125ModePin = GPIOE_12;
	engineConfiguration->auxTempSensor1.adcChannel = EFI_ADC_NONE;
	engineConfiguration->auxTempSensor2.adcChannel = EFI_ADC_NONE;
	engineConfiguration->baroSensor.hwChannel = EFI_ADC_NONE;
	engineConfiguration->throttlePedalPositionAdcChannel = EFI_ADC_NONE;
	engineConfiguration->oilPressure.hwChannel = EFI_ADC_NONE;
	engineConfiguration->vRefAdcChannel = EFI_ADC_NONE;
	engineConfiguration->vbattAdcChannel = EFI_ADC_NONE;
	engineConfiguration->map.sensor.hwChannel = EFI_ADC_NONE;
	engineConfiguration->mafAdcChannel = EFI_ADC_NONE;
/* this breaks unit tests lovely TODO: fix this?
	engineConfiguration->tps1_1AdcChannel = EFI_ADC_NONE;
*/
	engineConfiguration->tps1_2AdcChannel = EFI_ADC_NONE;
	engineConfiguration->tps2_1AdcChannel = EFI_ADC_NONE;
	engineConfiguration->tps2_2AdcChannel = EFI_ADC_NONE;
	engineConfiguration->auxFastSensor1_adcChannel = EFI_ADC_NONE;
	engineConfiguration->acSwitchAdc = EFI_ADC_NONE;
	engineConfiguration->externalKnockSenseAdc = EFI_ADC_NONE;
	engineConfiguration->fuelLevelSensor = EFI_ADC_NONE;
	engineConfiguration->hipOutputChannel = EFI_ADC_NONE;
	engineConfiguration->afr.hwChannel = EFI_ADC_NONE;
	engineConfiguration->high_fuel_pressure_sensor_1 = EFI_ADC_NONE;
	engineConfiguration->high_fuel_pressure_sensor_2 = EFI_ADC_NONE;
	
	engineConfiguration->clutchDownPinMode = PI_PULLUP;
	engineConfiguration->clutchUpPinMode = PI_PULLUP;
	engineConfiguration->brakePedalPinMode = PI_PULLUP;
}

void setDefaultBasePins(DECLARE_CONFIG_PARAMETER_SIGNATURE) {


#if EFI_PROD_CODE
	// call overrided board-specific serial configuration setup, if needed (for custom boards only)
	// needed also by bootloader code
	setPinConfigurationOverrides();
#endif /* EFI_PROD_CODE */


	engineConfiguration->tunerStudioSerialSpeed = TS_DEFAULT_SPEED;
	engineConfiguration->uartConsoleSerialSpeed = 115200;

#if EFI_PROD_CODE
	// call overrided board-specific serial configuration setup, if needed (for custom boards only)
	setSerialConfigurationOverrides();
#endif /* EFI_PROD_CODE */
}

// needed also by bootloader code
// at the moment bootloader does NOT really need SD card, this is a step towards future bootloader SD card usage
void setDefaultSdCardParameters(DECLARE_CONFIG_PARAMETER_SIGNATURE) {




#if EFI_PROD_CODE
	// call overrided board-specific SD card configuration setup, if needed (for custom boards only)
	setSdCardConfigurationOverrides();
#endif /* EFI_PROD_CODE */
}


// todo: move injector calibration somewhere else?
// todo: add a enum? if we have enough data?


static void setDefaultWarmupIdleCorrection(DECLARE_CONFIG_PARAMETER_SIGNATURE) {
	initTemperatureCurve(CLT_MANUAL_IDLE_CORRECTION, 1.0);

	float baseIdle = 30;

	setCurveValue(CLT_MANUAL_IDLE_CORRECTION, -40, 1.5);
	setCurveValue(CLT_MANUAL_IDLE_CORRECTION, -30, 1.5);
	setCurveValue(CLT_MANUAL_IDLE_CORRECTION, -20, 40.0 / baseIdle);
	setCurveValue(CLT_MANUAL_IDLE_CORRECTION, -10, 40.0 / baseIdle);
	setCurveValue(CLT_MANUAL_IDLE_CORRECTION,   0, 40.0 / baseIdle);
	setCurveValue(CLT_MANUAL_IDLE_CORRECTION,  10, 40.0 / baseIdle);
	setCurveValue(CLT_MANUAL_IDLE_CORRECTION,  20, 40.0 / baseIdle);
	setCurveValue(CLT_MANUAL_IDLE_CORRECTION,  30, 40.0 / baseIdle);
	setCurveValue(CLT_MANUAL_IDLE_CORRECTION,  40, 40.0 / baseIdle);
	setCurveValue(CLT_MANUAL_IDLE_CORRECTION,  50, 37.0 / baseIdle);
	setCurveValue(CLT_MANUAL_IDLE_CORRECTION,  60, 35.0 / baseIdle);
	setCurveValue(CLT_MANUAL_IDLE_CORRECTION,  70, 33.0 / baseIdle);
}

static void setDefaultWarmupFuelEnrichment(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	static const float bins[] =
	{
		-40,
		-30,
		-20,
		-10,
		0,
		10,
		20,
		30,
		40,
		50,
		60,
		70,
		80,
		90,
		100,
		110
	};

	copyArray(config->cltFuelCorrBins, bins);

	static const float values[] =
	{
		1.50,
		1.50,
		1.42,
		1.36,
		1.28,
		1.19,
		1.12,
		1.10,
		1.06,
		1.06,
		1.03,
		1.01,
		1,
		1,
		1,
		1
	};

	copyArray(config->cltFuelCorr, values);
}

static void setDefaultFuelCutParameters(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	engineConfiguration->enableDfco = false;
	engineConfiguration->rpmMinDfco = 1300;
	engineConfiguration->rpmMaxDfco = 1500;
	engineConfiguration->tpsTresholdDfco = 2;
	engineConfiguration->mapTresholdDfco = 30;
	engineConfiguration->cltTresholdDfco = 30;
}

static void setDefaultCrankingSettings(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	CONFIG(useTLE8888_cranking_hack) = true;

	setLinearCurve(config->crankingTpsCoef, /*from*/1, /*to*/1, 1);
	setLinearCurve(config->crankingTpsBins, 0, 100, 1);

	setLinearCurve(config->cltCrankingCorrBins, CLT_CURVE_RANGE_FROM, 100, 1);
	setLinearCurve(config->cltCrankingCorr, 1.0, 1.0, 1);

	setLinearCurve(config->afterstartCoolantBins, AFTERSTART_ENRICH_CURVE_SIZE, 100, 1);
	// Cranking temperature compensation
	static const float crankingCoef[] = {
		2.8,
		2.2,
		1.8,
		1.5,
		1.0,
		1.0,
		1.0,
		1.0
	};
	copyArray(config->crankingFuelCoef, crankingCoef);

	// Deg C
	static const float crankingBins[] = {
		-20,
		-10,
		5,
		30,
		35,
		50,
		65,
		90
	};
	copyArray(config->crankingFuelBins, crankingBins);

	// Cranking cycle compensation

	static const float crankingCycleCoef[] = {
		1.5,
		1.35,
		1.05,
		1.0,
		1.0,
		1.0,
		1.0,
		1.0
	};
	copyArray(config->crankingCycleCoef, crankingCycleCoef);

	static const float crankingCycleBins[] = {
		1,
		2,
		3,
		4,
		5,
		6,
		7,
		8
	};
	copyArray(config->crankingCycleBins, crankingCycleBins);

	// Cranking ignition timing
	static const float advanceValues[] = { 0, 0, 0, 0 };
	copyArray(config->crankingAdvance, advanceValues);

	static const float advanceBins[] = { 0, 200, 400, 1000 };
	copyArray(config->crankingAdvanceBins, advanceBins);

	static const float afterstartEnrich[] = {
			1.8,
			1.6,
			1.4,
			1.0,
			1.09,
			1.08,
			1.06,
			1.05
	};
	copyArray(config->afterstartEnrich, afterstartEnrich);

	static const float afterstartHold[] = {
					6.0,
					5.0,
					5.0,
					4.0,
					4.0,
					4.0,
					2.0,
					3.0
	};
	copyArray(config->afterstartHoldTime, afterstartHold);

	static const float afterstartDecay[] = {
						15.0,
						15.0,
						15.0,
						15.0,
						15.0,
						15.0,
						11.0,
						10.0
	};
	copyArray(config->afterstartDecayTime, afterstartDecay);


}

/**
 * see also setTargetRpmCurve()
 */
static void setDefaultIdleSpeedTarget(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	setLinearCurve(config->cltIdleRpmBins, CLT_CURVE_RANGE_FROM, 140, 10);

	setCurveValue(config->cltIdleRpmBins, config->cltIdleRpm, CLT_CURVE_SIZE, -30, 1350);
	setCurveValue(config->cltIdleRpmBins, config->cltIdleRpm, CLT_CURVE_SIZE, -20, 1300);
	setCurveValue(config->cltIdleRpmBins, config->cltIdleRpm, CLT_CURVE_SIZE, -10, 1200);
	setCurveValue(config->cltIdleRpmBins, config->cltIdleRpm, CLT_CURVE_SIZE, 0, 1150);
	setCurveValue(config->cltIdleRpmBins, config->cltIdleRpm, CLT_CURVE_SIZE, 10, 1100);
	setCurveValue(config->cltIdleRpmBins, config->cltIdleRpm, CLT_CURVE_SIZE, 20, 1050);
	setCurveValue(config->cltIdleRpmBins, config->cltIdleRpm, CLT_CURVE_SIZE, 30, 1000);
	setCurveValue(config->cltIdleRpmBins, config->cltIdleRpm, CLT_CURVE_SIZE, 40, 1000);
	setCurveValue(config->cltIdleRpmBins, config->cltIdleRpm, CLT_CURVE_SIZE, 50, 950);
	setCurveValue(config->cltIdleRpmBins, config->cltIdleRpm, CLT_CURVE_SIZE, 60, 950);
	setCurveValue(config->cltIdleRpmBins, config->cltIdleRpm, CLT_CURVE_SIZE, 70, 930);
	setCurveValue(config->cltIdleRpmBins, config->cltIdleRpm, CLT_CURVE_SIZE, 80, 900);
	setCurveValue(config->cltIdleRpmBins, config->cltIdleRpm, CLT_CURVE_SIZE, 90, 900);
	setCurveValue(config->cltIdleRpmBins, config->cltIdleRpm, CLT_CURVE_SIZE, 100, 1000);
	setCurveValue(config->cltIdleRpmBins, config->cltIdleRpm, CLT_CURVE_SIZE, 110, 1100);
}

static void setDefaultStepperIdleParameters(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	engineConfiguration->idle.pinStepperDirection = GPIO_UNASSIGNED;
	engineConfiguration->idle.stepperStepPin = GPIO_UNASSIGNED;
	engineConfiguration->pinStepperEnable = GPIO_UNASSIGNED;
	engineConfiguration->idleStepperReactionTime = 10;
	engineConfiguration->idleStepperTotalSteps = 150;
}

static void setCanFrankensoDefaults(DECLARE_CONFIG_PARAMETER_SIGNATURE) {
	engineConfiguration->canTxPin = GPIOB_6;
	engineConfiguration->canRxPin = GPIOB_12;
}

/**
 * see also setDefaultIdleSpeedTarget()
 */
void setTargetRpmCurve(int rpm DECLARE_CONFIG_PARAMETER_SUFFIX) {
	setLinearCurve(config->cltIdleRpmBins, CLT_CURVE_RANGE_FROM, 90, 10);
	setLinearCurve(config->cltIdleRpm, rpm, rpm, 10);
}

int getTargetRpmForIdleCorrection(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	// error is already reported, let's take the value at 0C since that should be a nice high idle
	float clt = Sensor::get(SensorType::Clt).value_or(0);

	int targetRpm = interpolate2d("cltRpm", clt, config->cltIdleRpmBins, config->cltIdleRpm);

	return targetRpm;
}

void setDefaultMultisparkParameters(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	// 1ms spark + 2ms dwell
	engineConfiguration->multisparkSparkDuration = 1000;
	engineConfiguration->multisparkDwell = 2000;

	// Conservative defaults - probably won't blow up coils
	engineConfiguration->multisparkMaxRpm = 1500;
	engineConfiguration->multisparkMaxExtraSparkCount = 2;
	engineConfiguration->multisparkMaxSparkingAngle = 30;
}

void setDefaultStftSettings(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	auto& cfg = CONFIG(stft);

	// Default to disabled
	CONFIG(fuelClosedLoopCorrectionEnabled) = false;

	// Default to proportional mode (for wideband sensors)
	CONFIG(stftIgnoreErrorMagnitude) = false;

	// 60 second startup delay - some O2 sensors are slow to warm up.
	cfg.startupDelay = 60;

	// Only correct in [12.0, 17.0]
	cfg.minAfr = 120;
	cfg.maxAfr = 170;

	// Above 60 deg C
	cfg.minClt = 60;

	// 0.5% deadband
	cfg.deadband = 5;

	// Sensible region defaults
	cfg.maxIdleRegionRpm = 1000 / RPM_1_BYTE_PACKING_MULT;
	cfg.maxOverrunLoad = 35;
	cfg.minPowerLoad = 85;

	// Sensible cell defaults
	for (size_t i = 0; i < efi::size(cfg.cellCfgs); i++) {
		// 30 second time constant - nice and slow
		cfg.cellCfgs[i].timeConstant = 30 * 10;

		/// Allow +-5%
		cfg.cellCfgs[i].maxAdd = 5;
		cfg.cellCfgs[i].maxRemove = -5;
	}
}

void setDefaultGppwmParameters(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	// Same config for all channels
	for (size_t i = 0; i < efi::size(CONFIG(gppwm)); i++) {
		auto& cfg = CONFIG(gppwm)[i];

		cfg.pin = GPIO_UNASSIGNED;
		cfg.dutyIfError = 0;
		cfg.onAboveDuty = 60;
		cfg.offBelowDuty = 50;
		cfg.pwmFrequency = 250;

		for (size_t j = 0; j < efi::size(cfg.loadBins); j++) {
			uint8_t z = j * 100 / (efi::size(cfg.loadBins) - 1);
			cfg.loadBins[j] = z;

			// Fill some values in the table
			for (size_t k = 0; k < efi::size(cfg.rpmBins); k++) {
				cfg.table[j][k] = z;
			}
			
		}

		for (size_t j = 0; j < efi::size(cfg.rpmBins); j++) {
			cfg.rpmBins[j] = 1000 * j / RPM_1_BYTE_PACKING_MULT;
		}
	}
}

/**
 * @brief	Global default engine configuration
 * This method sets the global engine configuration defaults. These default values are then
 * overridden by engine-specific defaults and the settings are saved in flash memory.
 *
 * This method is invoked only when new configuration is needed:
 *  * recently re-flashed chip
 *  * flash version of configuration failed CRC check or appears to be older then FLASH_DATA_VERSION
 *  * 'rewriteconfig' command
 *  * 'set engine_type X' command
 *
 * This method should only change the state of the configuration data structure but should NOT change the state of
 * anything else.
 *
 * This method should NOT be setting any default pinout
 */
static void setDefaultEngineConfiguration(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
#if (! EFI_UNIT_TEST)
	memset(&persistentState.persistentConfiguration, 0, sizeof(persistentState.persistentConfiguration));
#endif
	prepareVoidConfiguration(engineConfiguration);

#if EFI_ALTERNATOR_CONTROL
	setDefaultAlternatorParameters(PASS_CONFIG_PARAMETER_SIGNATURE);
#endif /* EFI_ALTERNATOR_CONTROL */

#if EFI_IDLE_CONTROL
	setDefaultIdleParameters(PASS_CONFIG_PARAMETER_SIGNATURE);
#endif /* EFI_IDLE_CONTROL */


#if EFI_ELECTRONIC_THROTTLE_BODY
	setDefaultEtbParameters(PASS_CONFIG_PARAMETER_SIGNATURE);
	setDefaultEtbBiasCurve(PASS_CONFIG_PARAMETER_SIGNATURE);
#endif /* EFI_ELECTRONIC_THROTTLE_BODY */
#if EFI_BOOST_CONTROL
    setDefaultBoostParameters(PASS_CONFIG_PARAMETER_SIGNATURE);
#endif

#if EFI_VVT_CONTROL
    setDefaultVvtParameters(PASS_CONFIG_PARAMETER_SIGNATURE);
#endif

	engineConfiguration->canSleepPeriodMs = 50;
	engineConfiguration->canReadEnabled = true;
	engineConfiguration->canWriteEnabled = true;
	engineConfiguration->canNbcType = CAN_BUS_NBC_VAG;

	// Don't enable, but set default address
	engineConfiguration->verboseCanBaseAddress = CAN_DEFAULT_BASE;





	CONFIG(startCrankingDuration) = 7;

	engineConfiguration->idlePidRpmDeadZone = 50;
	engineConfiguration->startOfCrankingPrimingPulse = 0;

	engineConfiguration->acCutoffLowRpm = 700;
	engineConfiguration->acCutoffHighRpm = 5000;

	engineConfiguration->postCrankingDurationSec = 2;

	initTemperatureCurve(IAT_FUEL_CORRECTION_CURVE, 1);

	engineConfiguration->tachPulseDuractionMs = 4;
	engineConfiguration->tachPulseTriggerIndex = 4;

	engineConfiguration->alternatorControl.minValue = 10;
	engineConfiguration->alternatorControl.maxValue = 90;


	setDefaultWarmupIdleCorrection(PASS_CONFIG_PARAMETER_SIGNATURE);

	setDefaultWarmupFuelEnrichment(PASS_ENGINE_PARAMETER_SIGNATURE);

	setDefaultFuelCutParameters(PASS_ENGINE_PARAMETER_SIGNATURE);


	/**
	 * 4ms is global default dwell for the whole RPM range
	 * if you only have one coil and many cylinders or high RPM you would need lower value at higher RPM
	 */
	setConstantDwell(4 PASS_CONFIG_PARAMETER_SUFFIX);
	/**
	 * Use angle-based duration during cranking
	 * this is equivalent to 'disable cranking_constant_dwell' console command
	 */
	engineConfiguration->enableFixedDwellCranking = true;
	engineConfiguration->ignitionDwellForCrankingMs = 6;

	setTimingLoadBin(1.2, 4.4 PASS_CONFIG_PARAMETER_SUFFIX);
	setTimingRpmBin(800, 7000 PASS_CONFIG_PARAMETER_SUFFIX);


	// set_whole_timing_map 3

	setAfrMap(config->afrTable, 14.7);

	setDefaultVETable(PASS_ENGINE_PARAMETER_SIGNATURE);

#if (IGN_LOAD_COUNT == FUEL_LOAD_COUNT) && (IGN_RPM_COUNT == FUEL_RPM_COUNT)
	// todo: make setMap a template
	setMap(config->injectionPhase, -180);
#endif
	setRpmTableBin(config->injPhaseRpmBins, FUEL_RPM_COUNT);
	setFuelTablesLoadBin(10, 160 PASS_CONFIG_PARAMETER_SUFFIX);
	setDefaultIatTimingCorrection(PASS_ENGINE_PARAMETER_SIGNATURE);

	setLinearCurve(config->loadBasedAeDecayBins, 0, 32, 4);
	setLinearCurve(config->loadBasedAeDecayMult, 1, 1, 1);

	setLinearCurve(config->tpsTpsAccelFromRpmBins, 0, 100, 10);
	setLinearCurve(config->tpsTpsAccelToRpmBins, 0, 100, 10);





	engineConfiguration->clt.config = {0, 23.8889, 48.8889, 9500, 2100, 1000, 1500};

// todo: this value is way off! I am pretty sure temp coeffs are off also
	engineConfiguration->iat.config = {32, 75, 120, 9500, 2100, 1000, 2700};

#if EFI_PROD_CODE
	engineConfiguration->warningPeriod = 10;
#else
	engineConfiguration->warningPeriod = 0;
#endif /* EFI_PROD_CODE */

#if EFI_LAUNCH_CONTROL
	setDefaultLaunchParameters(PASS_CONFIG_PARAMETER_SIGNATURE);
#endif


#if EFI_ANTILAG
	setDefaultAntiLagParameters(PASS_CONFIG_PARAMETER_SIGNATURE);
#endif

	enableDefaultCan(PASS_ENGINE_PARAMETER_SIGNATURE);
	engineConfiguration->slowAdcAlpha = 0.33333;


	engineConfiguration->rpmLimit = 7000;
	engineConfiguration->cranking.rpm = 550;
	engineConfiguration->cutFuelOnHardLimit = true;
	engineConfiguration->cutSparkOnHardLimit = true;


	engineConfiguration->tChargeMinRpmMinTps = 0.25;
	engineConfiguration->tChargeMinRpmMaxTps = 0.25;
	engineConfiguration->tChargeMaxRpmMinTps = 0.25;
	engineConfiguration->tChargeMaxRpmMaxTps = 0.9;
	engineConfiguration->tChargeMode = TCHARGE_MODE_RPM_TPS;
	engineConfiguration->tChargeAirCoefMin = 0.098f;
	engineConfiguration->tChargeAirCoefMax = 0.902f;
	engineConfiguration->tChargeAirFlowMax = 153.6f;
	engineConfiguration->tChargeAirIncrLimit = 1.0f;
	engineConfiguration->tChargeAirDecrLimit = 12.5f;

	engineConfiguration->noAccelAfterHardLimitPeriodSecs = 3;

	setDefaultCrankingSettings(PASS_ENGINE_PARAMETER_SIGNATURE);

	setDefaultStftSettings(PASS_ENGINE_PARAMETER_SIGNATURE);

	/**
	 * Idle control defaults
	 */
	setDefaultIdleSpeedTarget(PASS_ENGINE_PARAMETER_SIGNATURE);
	//	setTargetRpmCurve(1200 PASS_ENGINE_PARAMETER_SUFFIX);

	engineConfiguration->idleRpmPid.pFactor = 0.05;
	engineConfiguration->idleRpmPid.iFactor = 0.002;

	engineConfiguration->idleRpmPid.minValue = 0.1;
	engineConfiguration->idleRpmPid.maxValue = 99;
	engineConfiguration->idlePidDeactivationTpsThreshold = 2;

	engineConfiguration->idle.solenoidFrequency = 200;
	// set idle_position 50
	engineConfiguration->manIdlePosition = 50;
	engineConfiguration->crankingIACposition = 50;
//	engineConfiguration->idleMode = IM_AUTO;
	engineConfiguration->idleMode = IM_MANUAL;

	engineConfiguration->useStepperIdle = false;

	setDefaultStepperIdleParameters(PASS_ENGINE_PARAMETER_SIGNATURE);

	/**
	 * Cranking defaults
	 */
	engineConfiguration->startUpFuelPumpDuration = 4;
	engineConfiguration->cranking.baseFuel = 5;
	engineConfiguration->crankingChargeAngle = 70;


	engineConfiguration->timingMode = TM_DYNAMIC;
	engineConfiguration->fixedModeTiming = 50;

	setDefaultMultisparkParameters(PASS_ENGINE_PARAMETER_SIGNATURE);

	setDefaultGppwmParameters(PASS_ENGINE_PARAMETER_SIGNATURE);


#if !EFI_UNIT_TEST
	engineConfiguration->analogInputDividerCoefficient = 2;
#endif

	// performance optimization




	engineConfiguration->specs.firingOrder = FO_1_3_4_2;
	engineConfiguration->crankingInjectionMode = IM_SIMULTANEOUS;
	engineConfiguration->injectionMode = IM_SEQUENTIAL;

	engineConfiguration->ignitionMode = IM_ONE_COIL;
	engineConfiguration->globalTriggerAngleOffset = 0;
	engineConfiguration->extraInjectionOffset = 0;
	engineConfiguration->ignitionOffset = 0;





	engineConfiguration->vbattDividerCoeff = ((float) (15 + 65)) / 15;

	engineConfiguration->fanOnTemperature = 95;
	engineConfiguration->fanOffTemperature = 91;

	engineConfiguration->tpsMin = convertVoltageTo10bitADC(1.250);
	engineConfiguration->tpsMax = convertVoltageTo10bitADC(4.538);
	engineConfiguration->tpsErrorDetectionTooLow = -10; // -10% open
	engineConfiguration->tpsErrorDetectionTooHigh = 110; // 110% open

	engineConfiguration->oilPressure.v1 = 0.5f;
	engineConfiguration->oilPressure.v2 = 4.5f;
	engineConfiguration->oilPressure.value1 = 0;
	engineConfiguration->oilPressure.value2 = 689.476f;	// 100psi = 689.476kPa

	setOperationMode(engineConfiguration, FOUR_STROKE_CAM_SENSOR);
	engineConfiguration->specs.cylindersCount = 4;
	engineConfiguration->specs.displacement = 2;
	/**
	 * By the way http://users.erols.com/srweiss/tableifc.htm has a LOT of data
	 */
	engineConfiguration->injector.size = 200;

	engineConfiguration->mapLowValueVoltage = 0;
	// todo: start using this for custom MAP
	engineConfiguration->mapHighValueVoltage = 5;



	engineConfiguration->trigger.type = TT_TOOTHED_WHEEL_60_2;





	engineConfiguration->globalFuelCorrection = 1;
	engineConfiguration->adcVcc = 3.0;


	engineConfiguration->baroSensor.lowValue = 0;
	engineConfiguration->baroSensor.highValue = 500;



	engineConfiguration->useOnlyRisingEdgeForTrigger = false;
	// Default this to on - if you want to diagnose, turn it off.



	engineConfiguration->primingSquirtDurationMs = 5;

	engineConfiguration->enableInjectors = true;
	engineConfiguration->enableIgnition = true;
	engineConfiguration->isCylinderCleanupEnabled = false; // this feature is evil if one does not have TPS, better turn off by default
	engineConfiguration->secondTriggerChannelEnabled = true;



	engineConfiguration->debugMode = DBG_ALTERNATOR_PID;

	engineConfiguration->acIdleRpmBump = 200;
	engineConfiguration->knockDetectionWindowStart = 35;
	engineConfiguration->knockDetectionWindowEnd = 135;

	engineConfiguration->fuelLevelEmptyTankVoltage = 0;
	engineConfiguration->fuelLevelFullTankVoltage = 5;

	/**
	 * this is RPM. 10000 rpm is only 166Hz, 800 rpm is 13Hz
	 */
	engineConfiguration->triggerSimulatorFrequency = 1200;

	engineConfiguration->alternatorPwmFrequency = 300;

	

	engineConfiguration->enableAlternatorControl = false;

	engineConfiguration->vehicleSpeedCoef = 1.0f;


	engineConfiguration->mapErrorDetectionTooLow = 5;
	engineConfiguration->mapErrorDetectionTooHigh = 250;



	engineConfiguration->engineLoadAccelLength = 6;
	engineConfiguration->loadBasedAeMaxEnrich = 5; // kPa
	engineConfiguration->loadBasedAeMult = 0; // todo: improve implementation and re-enable by default

	engineConfiguration->tpsAccelLength = 12;
	engineConfiguration->maxDeltaTps = 40; // TPS % change, per engine cycle

}

/**
 * @brief	Hardware board-specific default configuration (GPIO pins, ADC channels, SPI configs etc.)
 */
void setDefaultFrankensoConfiguration(DECLARE_CONFIG_PARAMETER_SIGNATURE) {

	setCanFrankensoDefaults(PASS_CONFIG_PARAMETER_SIGNATURE);

	engineConfiguration->map.sensor.hwChannel = EFI_ADC_4;
	engineConfiguration->clt.adcChannel = EFI_ADC_6;
	engineConfiguration->iat.adcChannel = EFI_ADC_7;
	engineConfiguration->afr.hwChannel = EFI_ADC_14;



	engineConfiguration->pinTrigger[0] = GPIO_UNASSIGNED;
	engineConfiguration->pinTrigger[1] = GPIO_UNASSIGNED;


	// set optional subsystem configs


}

void resetConfigurationExt(configuration_callback_t boardCallback DECLARE_ENGINE_PARAMETER_SUFFIX) {
	enginePins.reset(); // that's mostly important for functional tests
	/**
	 * Let's apply global defaults first
	 */
	setDefaultEngineConfiguration(PASS_ENGINE_PARAMETER_SIGNATURE);

	// set initial pin groups
	setDefaultBasePins(PASS_CONFIG_PARAMETER_SIGNATURE);

	boardCallback(engineConfiguration);


	// call overrided board-specific configuration setup, if needed (for custom boards only)
	setBoardConfigurationOverrides();



	applyNonPersistentConfiguration(PASS_ENGINE_PARAMETER_SUFFIX);


	syncTunerStudioCopy();

}

void emptyCallbackWithConfiguration(engine_configuration_s * engineConfiguration) {
	UNUSED(engineConfiguration);
}

void resetConfigurationExt(DECLARE_ENGINE_PARAMETER_SUFFIX) {
	resetConfigurationExt(&emptyCallbackWithConfiguration PASS_ENGINE_PARAMETER_SUFFIX);
}

void validateConfiguration(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	if (engineConfiguration->adcVcc > 5.0f || engineConfiguration->adcVcc < 1.0f) {
		engineConfiguration->adcVcc = 3.0f;
	}
}

void applyNonPersistentConfiguration(DECLARE_ENGINE_PARAMETER_SUFFIX) {
#if EFI_PROD_CODE
	efiAssertVoid(CUSTOM_APPLY_STACK, getCurrentRemainingStack() > EXPECTED_REMAINING_STACK, "apply c");
#endif

	assertEngineReference();


	ENGINE(initializeTriggerWaveform(PASS_ENGINE_PARAMETER_SUFFIX));


}



void prepareShapes(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	prepareOutputSignals(PASS_ENGINE_PARAMETER_SIGNATURE);

	engine->injectionEvents.addFuelEvents(PASS_ENGINE_PARAMETER_SIGNATURE);
}



float getRpmMultiplier(operation_mode_e mode) {
	if (mode == FOUR_STROKE_CAM_SENSOR) {
		return 0.5;
	} else if (mode == FOUR_STROKE_CRANK_SENSOR) {
		return 1;
	}
	return 1;
}

void setOperationMode(engine_configuration_s *engineConfiguration, operation_mode_e mode) {
	engineConfiguration->ambiguousOperationMode = mode;
}

void commonFrankensoAnalogInputs(engine_configuration_s *engineConfiguration) {
	/**
	 * VBatt
	 */
	engineConfiguration->vbattAdcChannel = EFI_ADC_14;
}

void copyTargetAfrTable(fuel_table_t const source, afr_table_t destination) {
	// todo: extract a template!
	for (int loadIndex = 0; loadIndex < FUEL_LOAD_COUNT; loadIndex++) {
		for (int rpmIndex = 0; rpmIndex < FUEL_RPM_COUNT; rpmIndex++) {
			destination[loadIndex][rpmIndex] = AFR_STORAGE_MULT * source[loadIndex][rpmIndex];
		}
	}
}

void copyFuelTable(fuel_table_t const source, fuel_table_t destination) {
	// todo: extract a template!
	for (int loadIndex = 0; loadIndex < FUEL_LOAD_COUNT; loadIndex++) {
		for (int rpmIndex = 0; rpmIndex < FUEL_RPM_COUNT; rpmIndex++) {
			destination[loadIndex][rpmIndex] = source[loadIndex][rpmIndex];
		}
	}
}

void copyTimingTable(ignition_table_t const source, ignition_table_t destination) {
	// todo: extract a template!
	for (int k = 0; k < IGN_LOAD_COUNT; k++) {
		for (int rpmIndex = 0; rpmIndex < IGN_RPM_COUNT; rpmIndex++) {
			destination[k][rpmIndex] = source[k][rpmIndex];
		}
	}
}

static const ConfigOverrides defaultConfigOverrides{};
// This symbol is weak so that a board_configuration.cpp file can override it
__attribute__((weak)) const ConfigOverrides& getConfigOverrides() {
	return defaultConfigOverrides;
}
