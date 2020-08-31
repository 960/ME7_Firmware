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
#include "fram_crc.h"

EXTERN_ENGINE;

#if EFI_SPI_FRAM

extern persistent_config_s configWorkingCopy;
extern persistent_config_container_s persistentState;
extern persistent_config_s *config;

#define FRAM_RETRIES 10
#define FRAM_DELAY_MS 10
#define FRAM_CMD_WREN  0x06	//write enable
#define FRAM_CMD_WRDI  0x04	//write disable
#define FRAM_CMD_RDSR  0x05	//read status reg
#define FRAM_CMD_WRSR  0x01	//write status reg
#define FRAM_CMD_READ  0x03
#define FRAM_CMD_WRITE 0x02

#define TS_SIZE TS_CONFIG_SIZE + 12

uint8_t tx[4] NO_CACHE;
uint8_t rbuf[32] NO_CACHE;

static SPIDriver *spid;

static SPIConfig fram_spicfg = {
		false,
		NULL,
		SPI_FRAM_CS_GPIO,
		SPI_FRAM_CS_PIN,
		SPI_CR1_MSTR | SPI_FRAM_SPEED,
		SPI_CR2_8BIT_MODE
};


static THD_WORKING_AREA(eeThreadStack, 256);

class EeSpi: public eeSpiStream {
public:

	void send_offset(uint8_t cmd, uint32_t offset) const {
		uint8_t b[3] = { cmd, uint8_t(offset>>16), (uint8_t)(offset >> 8) };
		        spiSend(spid, sizeof(b), b);
	}

	bool read(uint32_t offset, size_t size, uint8_t *buf) {
		for (uint8_t r = 0; r < FRAM_RETRIES; r++) {
			if (r != 0) {
				chThdSleepMilliseconds(FRAM_DELAY_MS);
			}

			{
				spiAcquireBus(spid);
				spiStart(spid, &fram_spicfg);
				spiSelect(spid);
				send_offset(FRAM_CMD_READ, offset);
				spiReceive(spid, size, buf);
				spiUnselect(spid);
			}
			uint32_t crc1 = crc_crc32(0, buf, size);
			{
				spiSelect(spid);
				send_offset(FRAM_CMD_READ, offset);
				spiReceive(spid, size, buf);
				spiUnselect(spid);
				spiReleaseBus(spid);
			}
			uint32_t crc2 = crc_crc32(0, buf, size);
			if (crc1 == crc2) {
				return true;
			}
		}
		return false;
	}

	bool write(uint32_t offset, size_t size, const uint8_t *buf) {
		for (uint8_t r = 0; r < FRAM_RETRIES; r++) {
			if (r != 0) {
				chThdSleepMilliseconds(FRAM_DELAY_MS);
			}
			spiAcquireBus(spid);
			spiStart(spid, &fram_spicfg);
			tx[0] = FRAM_CMD_WREN;
			spiSelect(spid);
			spiSend(spid, 1, tx);
			spiUnselect(spid);

			spiSelect(spid);
			send_offset(FRAM_CMD_WRITE, offset);
			spiSend(spid, size, buf);
			spiUnselect(spid);

			const uint8_t nverify = minI(size, sizeof(rbuf));
			uint32_t crc1 = crc_crc32(0, buf, nverify);

			spiSelect(spid);
			send_offset(FRAM_CMD_READ, offset);
			spiReceive(spid, nverify, rbuf);
			spiUnselect(spid);
			spiReleaseBus(spid);
			uint32_t crc2 = crc_crc32(0, rbuf, nverify);

			if (crc1 == crc2) {
				return true;
			}
		}
		return false;
	}
};

static EeSpi spi;

int writeEeprom(uint32_t offset, size_t size, const uint8_t *buffer) {
	int ret = spi.write(offset, size, buffer);
	bool result = (ret == true);
	if (result == false) {
		return FLASH_RETURN_BAD_FLASH;
	}
	return FLASH_RETURN_SUCCESS;
}

int readEeprom(uint32_t offset, size_t size, uint8_t *buffer) {
	int result = spi.read(offset, size, buffer);
	if (result == false) {
		return FLASH_RETURN_BAD_FLASH;
	}
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

	chThdCreateStatic(eeThreadStack, sizeof(eeThreadStack), NORMALPRIO,
			(tfunc_t) spi_thread_1, NULL);
}

#endif

