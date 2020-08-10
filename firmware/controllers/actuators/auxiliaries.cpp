/*
 * auxiliaries.cpp
 *
 *  Created on: 10. mar. 2020
 *      (C) All rights reserved by RUUD BILELEKTRO, NORWAY
 */
#include "global.h"
#if EFI_AUXILIARIES

#if EFI_TUNER_STUDIO
#include "tunerstudio_outputs.h"
#endif /* EFI_TUNER_STUDIO */
#include "auxiliaries.h"
#include "engine.h"
#include "sensor.h"
#include "allsensors.h"
#include "thermistors.h"
#include "tps.h"
#include "map.h"
#include "io_pins.h"
#include "engine_configuration.h"
#include "gppwm.h"
#include "engine_controller.h"
#include "periodic_task.h"
#include "pin_repository.h"
#include "local_version_holder.h"
#include "pwm_generator_logic.h"
#if defined(HAS_OS_ACCESS)
#error "Unexpected OS ACCESS HERE"
#endif
EXTERN_ENGINE;

#define NO_PIN_PERIOD 500


extern pin_output_mode_e DEFAULT_OUTPUT;

class Auxiliaries: public PeriodicTimerController {
	int getPeriodMs() override {
		return 100;
	}

	void PeriodicTask() override {


		if (CONFIG(pinMainRelay) != GPIO_UNASSIGNED) {
			enginePins.mainRelay.setValue(1);
		}
		// see STARTER_RELAY_LOGIC
		if (CONFIG(pinStartRelay) != GPIO_UNASSIGNED) {
			enginePins.starterRelayDisable.setValue(engine->rpmCalculator.getRpm() < engineConfiguration->cranking.rpm);
		}
		// see FAN_CONTROL_LOGIC
		if (CONFIG(pinFan) != GPIO_UNASSIGNED) {
			auto clt = Sensor::get(SensorType::Clt);
			enginePins.fanRelay.setValue(!clt.Valid || (enginePins.fanRelay.getLogicValue() && (clt.Value > engineConfiguration->fanOffTemperature)) ||
				(clt.Value > engineConfiguration->fanOnTemperature) || engine->isCltBroken);
		}
		// see AC_RELAY_LOGIC
		if (CONFIG(pinAcRelay) != GPIO_UNASSIGNED) {
			enginePins.acRelay.setValue(getAcToggle(PASS_ENGINE_PARAMETER_SIGNATURE) && engine->rpmCalculator.getRpm() > 850);
		}
		// see FUEL_PUMP_LOGIC
		if (CONFIG(pinFuelPump) != GPIO_UNASSIGNED) {
			enginePins.fuelPumpRelay.setValue((getTimeNowSeconds() < engineConfiguration->startUpFuelPumpDuration) || (engine->rpmCalculator.getRpm() > 0));
		}


	}

};

static Auxiliaries AuxControl;


void stopAuxPins(void) {
	brain_pin_markUnused(activeConfiguration.pinMainRelay);
	brain_pin_markUnused(activeConfiguration.pinStartRelay);
	brain_pin_markUnused(activeConfiguration.pinFan);
	brain_pin_markUnused(activeConfiguration.pinAcRelay);
	brain_pin_markUnused(activeConfiguration.pinFuelPump);
}

void initAuxiliaries(DECLARE_ENGINE_PARAMETER_SIGNATURE) {


	stopAuxPins();
	AuxControl.Start();
}
#endif

