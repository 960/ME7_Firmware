/*
 ***************************************************************************************************
 *
 * (C) All rights reserved by RUUD BILELEKTRO, NORWAY
 *
 ***************************************************************************************************
 *
 * File: fram.cpp
 */

#include "global.h"
#include "fram.h"
#include "engine.h"
#include "pin_repository.h"
#include "mpu_util.h"
#include "flash_main.h"
#include "flash_int.h"

EXTERN_ENGINE;

#if EFI_SPI_FRAM

extern persistent_config_s configWorkingCopy;
extern persistent_config_container_s persistentState;
extern persistent_config_s *config;


#define FRAM_CMD_WREN  0x06	//write enable
#define FRAM_CMD_WRDI  0x04	//write disable
#define FRAM_CMD_RDSR  0x05	//read status reg
#define FRAM_CMD_WRSR  0x01	//write status reg
#define FRAM_CMD_READ  0x03
#define FRAM_CMD_WRITE 0x02

#define TS_SIZE TS_CONFIG_SIZE + 12
static uint8_t txbuf[TS_SIZE]NO_CACHE;

uint16_t tx[2] NO_CACHE;
uint16_t cmd[1] NO_CACHE;

static SPIDriver *spid;

static SPIConfig fram_spicfg = {
		false,
		NULL,
		SPI_FRAM_CS_GPIO,
		SPI_FRAM_CS_PIN,
		SPI_CR1_MSTR | SPI_FRAM_SPEED,
		SPI_CR2_8BIT_MODE
};


static THD_WORKING_AREA(eeThreadStack, 2048);


int readFullEeprom(flashaddr_t offset, char *buffer, size_t size) {

	spiSelect(spid);
	cmd[0] = FRAM_CMD_READ;
	spiSend(spid, 1, cmd);
	txbuf[0] = (uint8_t)(offset >> 16);
	txbuf[1] = (uint8_t)(offset >> 8);
	spiSend(spid, 2, txbuf);
	spiReceive(spid, size, buffer);
	spiUnselect(spid);

	return FLASH_RETURN_SUCCESS;
}


int writeFullEeprom(flashaddr_t offset, size_t size, char *buffer) {

	tx[0] = FRAM_CMD_WREN;
	spiSelect(spid);
	spiSend(spid, 1, tx);
	spiUnselect(spid);

	spiSelect(spid);
	cmd[0] = FRAM_CMD_WRITE;
	spiSend(spid, 1, cmd);
	txbuf[0] = (uint8_t)(offset >> 16);
	txbuf[1] = (uint8_t)(offset >> 8);
	spiSend(spid, 2, txbuf);
	spiSend(spid, size, buffer);
	spiUnselect(spid);

	return FLASH_RETURN_SUCCESS;
}


static THD_FUNCTION(spi_thread_1, arg) {
	(void) arg;
	chRegSetThreadName("eeprom");
}

void initEeprom(void) {
	/* init SPI pins */
	palSetPad(SPI_FRAM_CS_GPIO, SPI_FRAM_CS_PIN);

	palSetPadMode(SPI_FRAM_CS_GPIO, SPI_FRAM_CS_PIN,
			PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);

	palSetPadMode(getBrainPort(SPI_FRAM_SCK), getBrainPinIndex(SPI_FRAM_SCK),
			PAL_MODE_ALTERNATE(SPI_FRAM_AF) | PAL_STM32_OSPEED_HIGHEST);

	palSetPadMode(getBrainPort(SPI_FRAM_MISO), getBrainPinIndex(SPI_FRAM_MISO),
			PAL_MODE_ALTERNATE(SPI_FRAM_AF) | PAL_STM32_OSPEED_HIGHEST);

	palSetPadMode(getBrainPort(SPI_FRAM_MOSI), getBrainPinIndex(SPI_FRAM_MOSI),
			PAL_MODE_ALTERNATE(SPI_FRAM_AF) | PAL_STM32_OSPEED_HIGHEST);

	//set HOLD & WP High
	palSetPad(SPI_FRAM_HOLD_PORT, SPI_FRAM_HOLD_PIN);
	palSetPad(SPI_FRAM_WP_PORT, SPI_FRAM_WP_PIN);

	spid = SPI_FRAM_SPI;
	spiAcquireBus(spid);
	spiStart(spid, &fram_spicfg);

	chThdCreateStatic(eeThreadStack, sizeof(eeThreadStack), NORMALPRIO,
			(tfunc_t) spi_thread_1, NULL);

}

#endif

