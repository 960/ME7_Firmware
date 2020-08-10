/**
 * @file	fuel_math.cpp
 * @brief	Fuel amount calculation logic
 *
 *
 * @date May 27, 2013
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
#include "airmass.h"
#include "alphan_airmass.h"
#include "maf_airmass.h"
#include "speed_density_airmass.h"
#include "fuel_math.h"
#include "interpolation.h"
#include "engine_configuration.h"
#include "allsensors.h"
#include "engine_math.h"
#include "rpm_calculator.h"
#include "speed_density.h"
#include "perf_trace.h"
#include "sensor.h"
#include "speed_density_base.h"

EXTERN_ENGINE;

fuel_Map3D_t fuelPhaseMap("fl ph");
extern fuel_Map3D_t veMap;
extern afr_Map3D_t afrMap;




floatms_t getCrankingFuel3(floatms_t baseFuel, uint32_t revolutionCounterSinceStart DECLARE_ENGINE_PARAMETER_SUFFIX) {
	// these magic constants are in Celsius
	float baseCrankingFuel;
	if (engineConfiguration->useRunningMathForCranking) {
		baseCrankingFuel = baseFuel;
	} else {
		baseCrankingFuel = engineConfiguration->cranking.baseFuel;
	}
	/**
	 * Cranking fuel changes over time
	 */
	engine->engineState.cranking.durationCoefficient = interpolate2d("crank", revolutionCounterSinceStart, config->crankingCycleBins,
			config->crankingCycleCoef);

	/**
	 * Cranking fuel is different depending on engine coolant temperature
	 * If the sensor is failed, use 20 deg C
	 */
	auto clt = Sensor::get(SensorType::Clt);
	engine->engineState.cranking.coolantTemperatureCoefficient =
		interpolate2d("crank", clt.value_or(20), config->crankingFuelBins, config->crankingFuelCoef);


	floatms_t crankingFuel = baseCrankingFuel
			* engine->engineState.cranking.durationCoefficient
			* engine->engineState.cranking.coolantTemperatureCoefficient;

	engine->engineState.cranking.fuel = crankingFuel;

	if (crankingFuel <= 0) {
		warning(CUSTOM_ERR_ZERO_CRANKING_FUEL, "Cranking fuel value %f", crankingFuel);
	}
	return crankingFuel;
}

/* DISPLAY_ELSE */

floatms_t getRunningFuel(floatms_t baseFuel DECLARE_ENGINE_PARAMETER_SUFFIX) {
	ScopePerf perf(PE::GetRunningFuel);


	engine->engineState.running.baseFuel = baseFuel;

	float iatCorrection = ENGINE(engineState.running.intakeTemperatureCoefficient);

	float cltCorrection = ENGINE(engineState.running.coolantTemperatureCoefficient);


	float postCrankingFuelCorrection = ENGINE(engineState.running.postCrankingFuelCorrection);
#if EFI_LAUNCH_CONTROL
	float launchFuel;

	if ((engine->isLaunchCondition) && (CONFIG(enableLaunchFuel))) {
		launchFuel = CONFIG(launch.launchFuelAdded);
	} else {
		launchFuel = 1;
	}
#else
	launchFuel = 1;
#endif
	floatms_t runningFuel = baseFuel * iatCorrection * cltCorrection * postCrankingFuelCorrection * launchFuel * ENGINE(engineState.running.pidCorrection);

	ENGINE(engineState.running.fuel) = runningFuel;

	return runningFuel;
}

/* DISPLAY_ENDIF */

constexpr float convertToGramsPerSecond(float ccPerMinute) {
	float ccPerSecond = ccPerMinute / 60;
	return ccPerSecond * 0.72f;	// 0.72g/cc fuel density
}

/**
 * @return per cylinder injection time, in seconds
 */
float getInjectionDurationForAirmass(float airMass, float afr DECLARE_ENGINE_PARAMETER_SUFFIX) {
	float gPerSec = convertToGramsPerSecond(CONFIG(injector.size));

	return airMass / (afr * gPerSec);
}

static SpeedDensityAirmass sdAirmass(veMap);
static MafAirmass mafAirmass(veMap);
static AlphaNAirmass alphaNAirmass(veMap);

AirmassModelBase* getAirmassModel(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	switch (CONFIG(fuelAlgorithm)) {
		case LM_SPEED_DENSITY: return &sdAirmass;
		case LM_REAL_MAF: return &mafAirmass;
		case LM_ALPHA_N_2: return &alphaNAirmass;
#if EFI_UNIT_TEST
		case LM_MOCK: return engine->mockAirmassModel;
#endif
		default: return nullptr;
	}
}

/**
 * per-cylinder fuel amount
 * todo: rename this method since it's now base+TPSaccel
 */
floatms_t getBaseFuel(int rpm DECLARE_ENGINE_PARAMETER_SUFFIX) {
	ScopePerf perf(PE::GetBaseFuel);

	floatms_t tpsAccelEnrich = ENGINE(tpsAccelEnrichment.getTpsEnrichment(PASS_ENGINE_PARAMETER_SIGNATURE));
	efiAssert(CUSTOM_ERR_ASSERT, !cisnan(tpsAccelEnrich), "NaN tpsAccelEnrich", 0);
	ENGINE(engineState.tpsAccelEnrich) = tpsAccelEnrich;

	// airmass modes - get airmass first, then convert to fuel
	auto model = getAirmassModel(PASS_ENGINE_PARAMETER_SIGNATURE);
	efiAssert(CUSTOM_ERR_ASSERT, model != nullptr, "Invalid airmass mode", 0.0f);

	auto airmass = model->getAirmass(rpm);

	// The airmass mode will tell us how to look up AFR - use the provided Y axis value
	float targetAfr = afrMap.getValue(rpm, airmass.EngineLoadPercent);

	// Plop some state for others to read
	ENGINE(engineState.targetAFR) = targetAfr;
	ENGINE(engineState.sd.airMassInOneCylinder) = airmass.CylinderAirmass;
	ENGINE(engineState.fuelingLoad) = airmass.EngineLoadPercent;
	// TODO: independently selectable ignition load mode
	ENGINE(engineState.ignitionLoad) = airmass.EngineLoadPercent;

	float baseFuel = getInjectionDurationForAirmass(airmass.CylinderAirmass, targetAfr PASS_ENGINE_PARAMETER_SUFFIX) * 1000;
	efiAssert(CUSTOM_ERR_ASSERT, !cisnan(baseFuel), "NaN baseFuel", 0);

	engine->engineState.baseFuel = baseFuel;

	return tpsAccelEnrich + baseFuel;
}

angle_t getInjectionOffset(float rpm, float load DECLARE_ENGINE_PARAMETER_SUFFIX) {
	if (cisnan(rpm)) {
		return 0; // error already reported
	}

	if (cisnan(load)) {
		return 0; // error already reported
	}

	angle_t value = fuelPhaseMap.getValue(rpm, load);

	if (cisnan(value)) {
		// we could be here while resetting configuration for example
		warning(CUSTOM_ERR_6569, "phase map not ready");
		return 0;
	}

	angle_t result = value + CONFIG(extraInjectionOffset);
	fixAngle(result, "inj offset#2", CUSTOM_ERR_6553);
	return result;
}

/**
 * Number of injections using each injector per engine cycle
 * @see getNumberOfSparks
 */
int getNumberOfInjections(injection_mode_e mode DECLARE_ENGINE_PARAMETER_SUFFIX) {
	switch (mode) {
	case IM_SIMULTANEOUS:
	case IM_SINGLE_POINT:
		return engineConfiguration->specs.cylindersCount;
	case IM_BATCH:
		return 2;
	case IM_SEQUENTIAL:
		return 1;
	default:
		firmwareError(CUSTOM_ERR_INVALID_INJECTION_MODE, "Unexpected injection_mode_e %d", mode);
		return 1;
	}
}

/**
 * This is more like MOSFET duty cycle since durations include injector lag
 * @see getCoilDutyCycle
 */
percent_t getInjectorDutyCycle(int rpm DECLARE_ENGINE_PARAMETER_SUFFIX) {
	floatms_t totalInjectiorAmountPerCycle = ENGINE(injectionDuration) * getNumberOfInjections(engineConfiguration->injectionMode PASS_ENGINE_PARAMETER_SUFFIX);
	floatms_t engineCycleDuration = getEngineCycleDuration(rpm PASS_ENGINE_PARAMETER_SUFFIX);
	return 100 * totalInjectiorAmountPerCycle / engineCycleDuration;
}

static floatms_t getFuel(bool isCranking, floatms_t baseFuel DECLARE_ENGINE_PARAMETER_SUFFIX) {
	if (isCranking) {
		return getCrankingFuel(baseFuel PASS_ENGINE_PARAMETER_SUFFIX);
	} else {
		return getRunningFuel(baseFuel PASS_ENGINE_PARAMETER_SUFFIX);
	}
}

/**
 * @returns	Length of each individual fuel injection, in milliseconds
 *     in case of single point injection mode the amount of fuel into all cylinders, otherwise the amount for one cylinder
 */
floatms_t getInjectionDuration(int rpm DECLARE_ENGINE_PARAMETER_SUFFIX) {
	ScopePerf perf(PE::GetInjectionDuration);


	bool isCranking = ENGINE(rpmCalculator).isCranking(PASS_ENGINE_PARAMETER_SIGNATURE);
	injection_mode_e mode = ENGINE(getCurrentInjectionMode(PASS_ENGINE_PARAMETER_SIGNATURE));
	int numberOfInjections = getNumberOfInjections(mode PASS_ENGINE_PARAMETER_SUFFIX);
	if (numberOfInjections == 0) {
		warning(CUSTOM_CONFIG_NOT_READY, "config not ready");
		return 0; // we can end up here during configuration reset
	}

	// Always update base fuel - some cranking modes use it
	floatms_t baseFuel = getBaseFuel(rpm PASS_ENGINE_PARAMETER_SUFFIX);

	floatms_t fuelPerCycle = getFuel(isCranking, baseFuel PASS_ENGINE_PARAMETER_SUFFIX);

	if (mode == IM_SINGLE_POINT) {
		// here we convert per-cylinder fuel amount into total engine amount since the single injector serves all cylinders
		fuelPerCycle *= engineConfiguration->specs.cylindersCount;
	}
	// Fuel cut-off isn't just 0 or 1, it can be tapered
	fuelPerCycle *= ENGINE(engineState.fuelCutoffCorrection);
	// If no fuel, don't add injector lag
	if (fuelPerCycle == 0.0f)
		return 0;

	floatms_t theoreticalInjectionLength = fuelPerCycle / numberOfInjections;
	floatms_t injectorLag = ENGINE(engineState.running.injectorLag);
	if (cisnan(injectorLag)) {
		warning(CUSTOM_ERR_INJECTOR_LAG, "injectorLag not ready");
		return 0; // we can end up here during configuration reset
	}
	return theoreticalInjectionLength * engineConfiguration->globalFuelCorrection + injectorLag;

}

/**
 * @brief	Injector lag correction
 * @param	vBatt	Battery voltage.
 * @return	Time in ms for injection opening time based on current battery voltage
 */
floatms_t getInjectorLag(float vBatt DECLARE_ENGINE_PARAMETER_SUFFIX) {
	if (cisnan(vBatt)) {
		warning(OBD_System_Voltage_Malfunction, "vBatt=%.2f", vBatt);
		return 0;
	}
	
	return interpolate2d("lag", vBatt, engineConfiguration->injector.battLagCorrBins, engineConfiguration->injector.battLagCorr);
}

/**
 * @brief	Initialize fuel map data structure
 * @note this method has nothing to do with fuel map VALUES - it's job
 * is to prepare the fuel map data structure for 3d interpolation
 */
void initFuelMap(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	INJECT_ENGINE_REFERENCE(&sdAirmass);
	INJECT_ENGINE_REFERENCE(&mafAirmass);

#if (IGN_LOAD_COUNT == FUEL_LOAD_COUNT) && (IGN_RPM_COUNT == FUEL_RPM_COUNT)
	fuelPhaseMap.init(config->injectionPhase, config->injPhaseLoadBins, config->injPhaseRpmBins);
#endif /* (IGN_LOAD_COUNT == FUEL_LOAD_COUNT) && (IGN_RPM_COUNT == FUEL_RPM_COUNT) */
}

/**
 * @brief Engine warm-up fuel correction.
 */
float getCltFuelCorrection(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	const auto [valid, clt] = Sensor::get(SensorType::Clt);
	
	if (!valid)
		return 1; // this error should be already reported somewhere else, let's just handle it

	return interpolate2d("cltf", clt, config->cltFuelCorrBins, config->cltFuelCorr);
}

float getIatFuelCorrection(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	const auto [valid, iat] = Sensor::get(SensorType::Iat);

	if (!valid)
		return 1; // this error should be already reported somewhere else, let's just handle it

	return interpolate2d("iatc", iat, config->iatFuelCorrBins, config->iatFuelCorr);
}

float getAfterStartEnrichment(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
		const auto [valid, clt] = Sensor::get(SensorType::Clt);
			if (!valid)
			return 1;
			int revolutionCounter = engine->rpmCalculator.getRevolutionCounterSinceStart();
			float afterStartEnrich;
			float afterstartHoldTime;
			float afterstartDecayTime;
			float correction = 1.0f;
			float runTime = engine->engineState.running.timeSinceCrankingInSecs;


			afterStartEnrich = interpolate2d("aseEnrich", clt, config->afterstartCoolantBins, config->afterstartEnrich);
			afterstartHoldTime = interpolate2d("aseHold", clt, config->afterstartCoolantBins, config->afterstartHoldTime);
			afterstartDecayTime = interpolate2d("aseDecay", clt, config->afterstartCoolantBins, config->afterstartDecayTime);
			engine->engineState.running.timeSinceCrankingInSecs = NT2US(engine->engineState.timeSinceCranking) / 1000000.0f;



			if (ENGINE(rpmCalculator).isRunning(PASS_ENGINE_PARAMETER_SIGNATURE)) {
				if (runTime < afterstartHoldTime) {
					correction = afterStartEnrich;
				 if (afterStartEnrich < 1)
					 correction = 1.0f;
			    }
				if (runTime > afterstartHoldTime)  {
					correction = interpolateClamped(afterstartHoldTime, afterStartEnrich, (afterstartDecayTime + afterstartHoldTime), 1.0f , runTime);
				if (correction < 1)
					correction = 1.0f;
				}
				if (runTime > (afterstartHoldTime + afterstartDecayTime)) {
					correction = 1.0f;
				}
				} else {
					correction = 1.0f;
			}
			if (clt > 70 || revolutionCounter > 200) {
				correction = 1.0f;
			}
	return correction;
}
/**
 * @brief	Called from EngineState::periodicFastCallback to update the state.
 * @note The returned value is float, not boolean - to implement taper (smoothed correction).
 * @return	Fuel duration correction for fuel cut-off control (ex. if coasting). No correction if 1.0
 */
float getFuelCutOffCorrection(efitick_t nowNt, int rpm DECLARE_ENGINE_PARAMETER_SUFFIX) {
	// no corrections by default
	float fuelCorr = 1.0f;

	// coasting fuel cut-off correction
	if (CONFIG(enableDfco)) {
		auto [tpsValid, tpsPos] = Sensor::get(SensorType::Tps1);
		if (!tpsValid) {
			return 1.0f;
		}

		const auto [cltValid, clt] = Sensor::get(SensorType::Clt);
		if (!cltValid) {
			return 1.0f;
		}

		float map = getMap(PASS_ENGINE_PARAMETER_SIGNATURE);
	
		// gather events
		bool mapDeactivate = (map >= CONFIG(mapTresholdDfco));
		bool tpsDeactivate = (tpsPos >= CONFIG(tpsTresholdDfco));
		// If no CLT sensor (or broken), don't allow DFCO
		bool cltDeactivate = clt < (float)CONFIG(cltTresholdDfco);
		bool rpmDeactivate = (rpm < CONFIG(rpmMinDfco));
		bool rpmActivate = (rpm > CONFIG(rpmMaxDfco));
		
		// state machine (coastingFuelCutStartTime is also used as a flag)
		if (!mapDeactivate && !tpsDeactivate && !cltDeactivate && rpmActivate) {
			ENGINE(engineState.coastingFuelCutStartTime) = nowNt;
		} else if (mapDeactivate || tpsDeactivate || rpmDeactivate || cltDeactivate) {
			ENGINE(engineState.coastingFuelCutStartTime) = 0;
		}
		// enable fuelcut?
		if (ENGINE(engineState.coastingFuelCutStartTime) != 0) {
			// todo: add taper - interpolate using (nowNt - coastingFuelCutStartTime)?
			fuelCorr = 0.0f;
		}
	}
	
	// todo: add other fuel cut-off checks here (possibly cutFuelOnHardLimit?)
	return fuelCorr;
}

float getBaroCorrection(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	if (hasBaroSensor(PASS_ENGINE_PARAMETER_SIGNATURE)) {
		float correction = 1;
		if (cisnan(correction) || correction < 0.01) {
			warning(OBD_Barometric_Press_Circ_Range_Perf, "Invalid baro correction %f", correction);
			return 1;
		}
		return correction;
	} else {
		return 1;
	}
}


/**
 * @return Duration of fuel injection while craning
 */
floatms_t getCrankingFuel(float baseFuel DECLARE_ENGINE_PARAMETER_SUFFIX) {
	return getCrankingFuel3(baseFuel, engine->rpmCalculator.getRevolutionCounterSinceStart() PASS_ENGINE_PARAMETER_SUFFIX);
}

float getStandardAirCharge(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	float totalDisplacement = CONFIG(specs.displacement);
	float cylDisplacement = totalDisplacement / CONFIG(specs.cylindersCount);

	// Calculation of 100% VE air mass in g/cyl - 1 cylinder filling at 1.204/L
	// 101.325kpa, 20C
	return idealGasLaw(cylDisplacement, 101.325f, 273.15f + 20.0f);
}

float getFuelRate(floatms_t totalInjDuration, efitick_t timePeriod DECLARE_ENGINE_PARAMETER_SUFFIX) {
	if (timePeriod <= 0.0f)
		return 0.0f;
	float timePeriodMs = (float)NT2US(timePeriod) / 1000.0f;
	float fuelRate = totalInjDuration / timePeriodMs;
	const float cc_min_to_L_h = 60.0f / 1000.0f;
	return fuelRate * CONFIG(injector.size) * cc_min_to_L_h;
}


