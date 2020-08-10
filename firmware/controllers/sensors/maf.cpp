#include "global.h"
#include "engine.h"
#include "adc_inputs.h"
#include "maf.h"

EXTERN_ENGINE;

/**
 * @return MAF sensor voltage
 */
float getMafVoltage(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	return getVoltageDivided("maf", engineConfiguration->mafAdcChannel PASS_ENGINE_PARAMETER_SUFFIX);
}

bool hasMafSensor(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	return engineConfiguration->mafAdcChannel != EFI_ADC_NONE;
}

/**
 * @return kg/hour value
 */
float getRealMaf(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	return  1;
}

static void fillTheRest(persistent_config_s *e, int i) {

}

static int addMafPoint(persistent_config_s *e, int i, float kgHrValue, float voltage) {

	return  1;
}

/**
 * Hot-film air-mass meter, Type HFM 5
 */


void setBosch0280218004(persistent_config_s *e) {
	int i = 0;

	fillTheRest(e, i);
}



