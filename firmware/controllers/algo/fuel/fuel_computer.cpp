#include "fuel_computer.h"

EXTERN_ENGINE;

mass_t FuelComputerBase::getCycleFuel(mass_t airmass, int rpm, float load) const {
	float stoich = getStoichiometricRatio();
	float lambda = getTargetLambda(rpm, load);
	float afr = stoich * lambda;

	ENGINE(engineState.targetAFR) = afr;

	return airmass / afr;
}

FuelComputer::FuelComputer(const ValueProvider3D& afrTable) : m_afrTable(&afrTable) {}

float FuelComputer::getStoichiometricRatio() const {
	// TODO: vary this with ethanol content/configured setting/whatever
	float rawConfig = (float)CONFIG(stoichRatioPrimary) / PACK_MULT_AFR_CFG;

	// Config compatibility: this field may be zero on ECUs with old defaults
	if (rawConfig < 5) {
		return 14.7f;
	}

	return rawConfig;
}

float FuelComputer::getTargetLambda(int rpm, float load) const {
	efiAssert(OBD_PCM_Processor_Fault, m_afrTable != nullptr, "AFR table null", 0);

	// TODO: set the table value in lambda instead of afr
	return m_afrTable->getValue(rpm, load) / 14.7f;
};
