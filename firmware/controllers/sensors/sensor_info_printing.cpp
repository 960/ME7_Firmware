#include "global.h"
#include "proxy_sensor.h"
#include "functional_sensor.h"
#include "redundant_sensor.h"
#include "rpm_calculator.h"
#include "linear_func.h"
#include "resistance_func.h"
#include "thermistor_func.h"
#include "efilib.h"


void ProxySensor::showInfo(const char* sensorName) const {
UNUSED(sensorName);
}

void FunctionalSensor::showInfo(const char* sensorName) const {
	UNUSED(sensorName);
	const auto [valid, value] = get();
	UNUSED(valid);
	UNUSED(value);
	// now print out the underlying function's info
	if (auto func = m_function) {
		func->showInfo(m_rawValue);
	}
}

#if EFI_CAN_SUPPORT
#include "can_sensor.h"

void CanSensorBase::showInfo(const char* sensorName) const {
	UNUSED(sensorName);
	const auto [valid, value] = get();
	UNUSED(valid);
	UNUSED(value);
}
#endif // EFI_CAN_SUPPORT

void RedundantSensor::showInfo(const char* sensorName) const {
	UNUSED(sensorName);
}

void RpmCalculator::showInfo( const char* sensorName) const {

}
