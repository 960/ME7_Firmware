/**
 * @file    accel_enrichment.cpp
 * @brief   Acceleration enrichment calculator
 *
 * In this file we have three strategies for acceleration/deceleration fuel correction
 *
 * 1) MAP rate-of-change correction
 * 2) TPS rate-of-change correction
 * 3) fuel film/wal wetting correction
 *   AWC Added to Wall Coefficient, %
 *   AWA Added to Wall Amount
 *   SOC Sucked Off wall Coefficient, %
 *   SOA Sucked Off wall amount
 *   WF  current on-Wall Fuel amount
 *
 *
 * http://rusefi.com/wiki/index.php?title=Manual:Software:Fuel_Control
 * @date Apr 21, 2014
 * @author Dmitry Sidin
 * @author Andrey Belomutskiy, (c) 2012-2020
 * @author Matthew Kennedy
 */

#include "global.h"
#include "engine.h"
#include "trigger_central.h"
#include "accel_enrichment.h"
#include "allsensors.h"
#include "engine_math.h"
#include "perf_trace.h"
#if EFI_TUNER_STUDIO
#include "tunerstudio_outputs.h"
#endif /* EFI_TUNER_STUDIO */

EXTERN_ENGINE;

tps_tps_Map3D_t tpsTpsMap("tpsTps");


void WallFuel::resetWF() {
	wallFuel = 0;
}

//
floatms_t WallFuel::adjust(floatms_t desiredFuel DECLARE_ENGINE_PARAMETER_SUFFIX) {
	invocationCounter++;
	if (cisnan(desiredFuel)) {
		return desiredFuel;
	}
	// disable this correction for cranking
	if (ENGINE(rpmCalculator).isCranking(PASS_ENGINE_PARAMETER_SIGNATURE)) {
		return desiredFuel;
	}

	ScopePerf perf(PE::WallFuelAdjust);

	/*
		this math is based on 
				SAE 810494 by C. F. Aquino
				SAE 1999-01-0553 by Peter J Maloney

		M_cmd = commanded fuel mass (output of this function)
		desiredFuel = desired fuel mass (input to this function)
		fuelFilmMass = fuel film mass (how much is currently on the wall)

		First we compute how much fuel to command, by accounting for
		a) how much fuel will evaporate from the walls, entering the air
		b) how much fuel from the injector will hit the walls, being deposited

		Next, we compute how much fuel will be deposited on the walls.  The net
		effect of these two steps is computed (some leaves walls, some is deposited)
		and stored back in fuelFilmMass.

		alpha describes the amount of fuel that REMAINS on the wall per cycle.
		It is computed as a function of the evaporation time constant (tau) and
		the time the fuel spent on the wall this cycle, (recriprocal RPM).

		beta describes the amount of fuel that hits the wall.  

		TODO: these parameters, tau and beta vary with various engine parameters,
		most notably manifold pressure (as a proxy for air speed), and coolant
		temperature (as a proxy for the intake valve and runner temperature).

		TAU: decreases with increasing temperature.
		     decreases with decreasing manifold pressure.

		BETA: decreases with increasing temperature.
		     decreases with decreasing manifold pressure.
	*/

	// if tau is really small, we get div/0.
	// you probably meant to disable wwae.

	float tau = CONFIG(wwaeTau);
	if (tau < 0.01f) {
		return desiredFuel;
	}

	// Ignore really slow RPM
	int rpm = GET_RPM();
	if (rpm < 100) {
		return desiredFuel;
	}

	float alpha = expf_taylor(-120 / (rpm * tau));
	float beta = CONFIG(wwaeBeta);

	// If beta is larger than alpha, the system is underdamped.
	// For reasonable values {tau, beta}, this should only be possible
	// at extremely low engine speeds (<300rpm ish)
	// Clamp beta to less than alpha.
	if (beta > alpha) {
		beta = alpha;
	}

	float fuelFilmMass = wallFuel;
	float M_cmd = (desiredFuel - (1 - alpha) * fuelFilmMass) / (1 - beta);
	
	// We can't inject a negative amount of fuel
	// If this goes below zero we will be over-fueling slightly,
	// but that's ok.
	if (M_cmd <= 0) {
		M_cmd = 0;
	}

	// remainder on walls from last time + new from this time
	float fuelFilmMassNext = alpha * fuelFilmMass + beta * M_cmd;




	return M_cmd;
}

floatms_t WallFuel::getWallFuel() const {
	return wallFuel;
}

int AccelEnrichment::getMaxDeltaIndex(DECLARE_ENGINE_PARAMETER_SIGNATURE) {

	int len = minI(cb.getSize(), cb.getCount());
	if (len < 2)
		return 0;
	int ci = cb.currentIndex - 1;
	float maxValue = cb.get(ci) - cb.get(ci - 1);
	int resultIndex = ci;

	// todo: 'get' method is maybe a bit heavy because of the branching
	// todo: this could be optimized with some careful magic

	for (int i = 1; i<len - 1;i++) {
		float v = cb.get(ci - i) - cb.get(ci - i - 1);
		if (v > maxValue) {
			maxValue = v;
			resultIndex = ci - i;
		}
	}

	return resultIndex;
}

float AccelEnrichment::getMaxDelta(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	int index = getMaxDeltaIndex(PASS_ENGINE_PARAMETER_SIGNATURE);

	return (cb.get(index) - (cb.get(index - 1)));
}

// todo: eliminate code duplication between these two methods! Some pointer magic would help.
floatms_t TpsAccelEnrichment::getTpsEnrichment(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	ScopePerf perf(PE::GetTpsEnrichment);

	int maxDeltaIndex = getMaxDeltaIndex(PASS_ENGINE_PARAMETER_SIGNATURE);

//	FuelSchedule *fs = engineConfiguration->injectionEvents;
	percent_t tpsTo = cb.get(maxDeltaIndex);
	percent_t tpsFrom = cb.get(maxDeltaIndex - 1);
	percent_t deltaTps = tpsTo - tpsFrom;

	float valueFromTable = tpsTpsMap.getValue(tpsFrom, tpsTo);

	floatms_t extraFuel;
	if (deltaTps > engineConfiguration->maxDeltaTps) {
		extraFuel = valueFromTable;
	} else if (deltaTps < -engineConfiguration->maxDeltaTpsEnlean) {
		extraFuel = deltaTps * engineConfiguration->tpsTpsEnleanFactor;
	} else {
		extraFuel = 0;
	}

	// Fractional enrichment (fuel portions are accumulated and split between several engine cycles.
	// This is a crude imitation of carburetor's acceleration pump.
	if (CONFIG(tpsAccelFractionCycles) > 1 || CONFIG(tpsAccelFractionDivisor) > 1.0f) {
		// make sure both values are non-zero
		float periodF = (float)maxI(CONFIG(tpsAccelFractionCycles), 1);
		float divisor = maxF(CONFIG(tpsAccelFractionDivisor), 1.0f);

		// if current extra fuel portion is not "strong" enough, then we keep up the "pump pressure" with the accumulated portion
		floatms_t maxExtraFuel = maxF(extraFuel, accumulatedValue);
		// use only a fixed fraction of the accumulated portion
		floatms_t injFuel = maxExtraFuel / divisor;

		// update max counters
		maxExtraPerCycle = maxF(extraFuel, maxExtraPerCycle);
		maxInjectedPerPeriod = maxF(injFuel, maxInjectedPerPeriod);

		// evenly split it between several engine cycles
		extraFuel = injFuel / periodF;
	} else {
		resetFractionValues();
	}

	if (engineConfiguration->debugMode == DBG_TPS_ACCEL) {
#if EFI_TUNER_STUDIO
		tsOutputChannels.debugFloatField1 = tpsFrom;
		tsOutputChannels.debugFloatField2 = tpsTo;
		tsOutputChannels.debugFloatField3 = valueFromTable;
		tsOutputChannels.debugFloatField4 = extraFuel;
		tsOutputChannels.debugFloatField5 = accumulatedValue;
		tsOutputChannels.debugFloatField6 = maxExtraPerPeriod;
		tsOutputChannels.debugFloatField7 = maxInjectedPerPeriod;
		tsOutputChannels.debugIntField1 = cycleCnt;
#endif /* EFI_TUNER_STUDIO */
	}

	return extraFuel;
}

float LoadAccelEnrichment::getEngineLoadEnrichment(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	int index = getMaxDeltaIndex(PASS_ENGINE_PARAMETER_SIGNATURE);

	float d = (cb.get(index) - (cb.get(index - 1))) * CONFIG(specs.cylindersCount);

	float result = 0;
	int distance = 0;
	float decay = 0;
	if (d > engineConfiguration->loadBasedAeMaxEnrich) {

		distance = cb.currentIndex - index;
		if (distance <= 0) // checking if indexes are out of order due to circular buffer nature
			distance += minI(cb.getCount(), cb.getSize());

		decay = interpolate2d("accel", distance, config->loadBasedAeDecayBins, config->loadBasedAeDecayMult);

		result = decay * d * engineConfiguration->loadBasedAeMult;
	} else if (d < -engineConfiguration->loadBasedAeMaxEnleanment) {
		result = d * engineConfiguration->loadBasedAeMult;
	}

	if (engineConfiguration->debugMode == DBG_EL_ACCEL) {
#if EFI_TUNER_STUDIO
		tsOutputChannels.debugIntField1 = distance;
		tsOutputChannels.debugFloatField1 = result;
		tsOutputChannels.debugFloatField2 = decay;
#endif /* EFI_TUNER_STUDIO */
	}
	return result;
}

void AccelEnrichment::resetAE() {
	cb.clear();
}

void TpsAccelEnrichment::resetAE() {
	AccelEnrichment::resetAE();
	resetFractionValues();
}

void TpsAccelEnrichment::resetFractionValues() {
	accumulatedValue = 0;
	maxExtraPerCycle = 0;
	maxExtraPerPeriod = 0;
	maxInjectedPerPeriod = 0;
	cycleCnt = 0;
}

void AccelEnrichment::setLength(int length) {
	cb.setSize(length);
}

void AccelEnrichment::onNewValue(float currentValue DECLARE_ENGINE_PARAMETER_SUFFIX) {
	cb.add(currentValue);
}

void TpsAccelEnrichment::onEngineCycleTps(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	// we update values in handleFuel() directly by calling onNewValue()

	onUpdateInvocationCounter++;

	// we used some extra fuel during the current cycle, so we "charge" our "acceleration pump" with it
	accumulatedValue -= maxExtraPerPeriod;
	maxExtraPerPeriod = maxF(maxExtraPerCycle, maxExtraPerPeriod);
	maxExtraPerCycle = 0;
	accumulatedValue += maxExtraPerPeriod;

	// update the accumulated value every 'Period' engine cycles
	if (--cycleCnt <= 0) {
		maxExtraPerPeriod = 0;

		// we've injected this portion during the cycle, so we set what's left for the next cycle
		accumulatedValue -= maxInjectedPerPeriod;
		maxInjectedPerPeriod = 0;

		// it's an infinitely convergent series, so we set a limit at some point
		// (also make sure that accumulatedValue is positive, for safety)
		static const floatms_t smallEpsilon = 0.001f;
		if (accumulatedValue < smallEpsilon)
			accumulatedValue = 0;

		// reset the counter
		cycleCnt = CONFIG(tpsAccelFractionCycles);
	}
}

void LoadAccelEnrichment::onEngineCycle(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	onNewValue(getFuelingLoad(PASS_ENGINE_PARAMETER_SIGNATURE) PASS_ENGINE_PARAMETER_SUFFIX);
}

AccelEnrichment::AccelEnrichment() {
	resetAE();
	cb.setSize(4);
}

void setEngineLoadAccelThr(float value) {
	engineConfiguration->loadBasedAeMaxEnrich = value;
}

void setEngineLoadAccelMult(float value) {
	engineConfiguration->loadBasedAeMult = value;
}

void setTpsAccelThr(float value) {
	engineConfiguration->maxDeltaTps = value;
}

void setTpsDecelThr(float value) {
	engineConfiguration->maxDeltaTpsEnlean = value;
}

void setTpsDecelMult(float value) {
	engineConfiguration->tpsTpsEnleanFactor = value;
}

void setDecelThr(float value) {
	engineConfiguration->loadBasedAeMaxEnleanment = value;
}

void setDecelMult(float value) {
	engineConfiguration->engineLoadDecelEnleanmentMultiplier = value;
}

void setTpsAccelLen(int length) {
	if (length < 1) {

		return;
	}
	engine->tpsAccelEnrichment.setLength(length);
}

void setEngineLoadAccelLen(int length) {
	if (length < 1) {

		return;
	}
	engine->engineLoadAccelEnrichment.setLength(length);
}

void updateAccelParameters() {
	setEngineLoadAccelLen(engineConfiguration->engineLoadAccelLength);
	setTpsAccelLen(engineConfiguration->tpsAccelLength);
}

void initAccelEnrichment( DECLARE_ENGINE_PARAMETER_SUFFIX) {
	tpsTpsMap.init(config->tpsTpsAccelTable, config->tpsTpsAccelFromRpmBins, config->tpsTpsAccelToRpmBins);

	updateAccelParameters();
}

