/*
 * @file CJ125_logic.cpp
 *
 * @date Feb 1, 2019
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#include "cj125_logic.h"
#include "engine.h"
#include "error_handling.h"

EXTERN_ENGINE;

#define LOW_VOLTAGE "Low Voltage"

CJ125::CJ125() : wboHeaterControl("wbo"),
		heaterPid(&heaterPidConfig) {
}

void CJ125::SetHeater(float value DECLARE_ENGINE_PARAMETER_SUFFIX) {
	float maxDuty = (engine->sensors.vBatt > CJ125_HEATER_LIMITING_VOLTAGE) ? CJ125_HEATER_LIMITING_RATE : 1.0f;
	heaterDuty = (value < CJ125_HEATER_MIN_DUTY) ? 0.0f : minF(maxF(value, 0.0f), maxDuty);
	wboHeaterControl.setFrequency(CJ125_HEATER_PWM_FREQ);
	wboHeaterControl.setSimplePwmDutyCycle(heaterDuty);
}

void CJ125::SetIdleHeater(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	SetHeater(CJ125_HEATER_IDLE_RATE PASS_ENGINE_PARAMETER_SUFFIX);
}

bool CJ125::isWorkingState(void) const {
	return state != CJ125_ERROR && state != CJ125_INIT && state != CJ125_IDLE;
}

void CJ125::StartHeaterControl(pwm_gen_callback *stateChangeCallback DECLARE_ENGINE_PARAMETER_SUFFIX) {
	startSimplePwmExt(&wboHeaterControl, "wboHeaterPin",
			&engine->executor,
			engine->wboHeaterPin,
			&wboHeaterPin, CJ125_HEATER_PWM_FREQ, 0.0f, stateChangeCallback);
	SetIdleHeater(PASS_ENGINE_PARAMETER_SIGNATURE);
}

bool CJ125::cjIdentify(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	efiAssert(OBD_PCM_Processor_Fault, spi!= NULL, "No SPI pointer", false);
	int ident = spi->ReadRegister(IDENT_REG_RD) & CJ125_IDENT_MASK;
	spi->WriteRegister(INIT_REG1_WR, CJ125_INIT1_NORMAL_17);
	spi->WriteRegister(INIT_REG2_WR, CJ125_INIT2_DIAG);
	int init1 = spi->ReadRegister(INIT_REG1_RD);
	int init2 = spi->ReadRegister(INIT_REG2_RD);
	diag = spi->ReadRegister(DIAG_REG_RD);
	tsOutputChannels.widebandIdent = ident;

	if (ident != CJ125_IDENT) {
		setError(CJ125_ERROR_WRONG_IDENT PASS_ENGINE_PARAMETER_SUFFIX);
		return false;
	}
	if (init1 != CJ125_INIT1_NORMAL_17 || init2 != CJ125_INIT2_DIAG) {
		setError(CJ125_ERROR_WRONG_IDENT PASS_ENGINE_PARAMETER_SUFFIX);
		return false;
	}
	return true;
}

void CJ125::cjSetMode(cj125_mode_e m) {
	if (mode == m)
		return;
	switch (m) {
	case CJ125_MODE_NORMAL_8:
		spi->WriteRegister(INIT_REG1_WR, CJ125_INIT1_NORMAL_8);
		amplCoeff = 1.0f / 8.0f;
		break;
	case CJ125_MODE_NORMAL_17:
		spi->WriteRegister(INIT_REG1_WR, CJ125_INIT1_NORMAL_17);
		amplCoeff = 1.0f / 17.0f;
		break;
	case CJ125_MODE_CALIBRATION:
		spi->WriteRegister(INIT_REG1_WR, CJ125_INIT1_CALBRT);
		amplCoeff = 0.0f;
		break;
	default:
		;
	}
	mode = m;
}

bool CJ125::isValidState() const {
	if (!isWorkingState())
		return true;
	if (amplCoeff == 0.0f)
		return false;

	if (vUaCal < CJ125_UACAL_MIN || vUaCal > CJ125_UACAL_MAX)
		return false;
	return true;
}

void CJ125::cjInitPid(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
if (CONFIG(tuneCj125Pid)) {
	if (engineConfiguration->cj125isLsu49) {
		heaterPidConfig.pFactor = config->cj125Pfactor;
		heaterPidConfig.iFactor = config->cj125Ifactor;
	} else {
		heaterPidConfig.pFactor = config->cj125Pfactor;
		heaterPidConfig.iFactor = config->cj125Ifactor;
	}
} else {
	if (engineConfiguration->cj125isLsu49) {
			heaterPidConfig.pFactor = CJ125_PID_LSU49_P;
			heaterPidConfig.iFactor = CJ125_PID_LSU49_I;
		} else {
			heaterPidConfig.pFactor = CJ125_PID_LSU42_P;
			heaterPidConfig.iFactor = CJ125_PID_LSU42_I;
		}
}

	heaterPidConfig.dFactor = 0.0f;
	heaterPidConfig.minValue = 0;
	heaterPidConfig.maxValue = 1;
	heaterPidConfig.offset = 0;
	heaterPid.reset();
}
