/**
 * @author Andrey Belomutskiy, (c) 2012-2020
 */
#include "engine.h"
#include "tps.h"
#include "sensor.h"
#if EFI_PROD_CODE

#endif /* EFI_PROD_CODE */

EXTERN_ENGINE;

void grabTPSIsClosed() {
#if EFI_PROD_CODE

	engineConfiguration->tpsMin = convertVoltageTo10bitADC(Sensor::getRaw(SensorType::Tps1));

#endif /* EFI_PROD_CODE */
}

void grabTPSIsWideOpen() {
#if EFI_PROD_CODE

	engineConfiguration->tpsMax = convertVoltageTo10bitADC(Sensor::getRaw(SensorType::Tps1));

#endif /* EFI_PROD_CODE */
}

void grabPedalIsUp() {
#if EFI_PROD_CODE
	engineConfiguration->throttlePedalUpVoltage = Sensor::getRaw(SensorType::AcceleratorPedal);

#endif /* EFI_PROD_CODE */
}

void grabPedalIsWideOpen() {
#if EFI_PROD_CODE
	engineConfiguration->throttlePedalWOTVoltage = Sensor::getRaw(SensorType::AcceleratorPedal);

#endif /* EFI_PROD_CODE */
}
