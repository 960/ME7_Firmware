#include "global.h"
#include "fram.h"
#include "engine.h"
#include "pin_repository.h"
#include "mpu_util.h"

EXTERN_ENGINE;

#if EFI_SPI_FRAM

#define SPI_BUFFERS_SIZE    128U

#define DUMMYBYTE  0xFE	//dummy bytes to make easier to sniff

#define FRAM_CMD_WREN  0x06	//write enable
#define FRAM_CMD_WRDI  0x04	//write disable
#define FRAM_CMD_RDSR  0x05	//read status reg
#define FRAM_CMD_WRSR  0x01	//write status reg
#define FRAM_CMD_READ  0x03
#define FRAM_CMD_WRITE 0x02
//Not for all devices
#define FRAM_CMD_FSTRD  0x0B	//fast read
#define FRAM_CMD_SLEEP  0xB9	//power down
#define FRAM_CMD_RDID  0x9F	  //read JEDEC ID = Manuf+ID (suggested)
#define FRAM_CMD_SNR  0xC3	  //Reads 8-byte serial number
static uint8_t txbuf[SPI_BUFFERS_SIZE];
static uint8_t rxbuf[SPI_BUFFERS_SIZE];


static const SPIConfig hs_spicfg = {
  false,
  NULL,
  SPI_FRAM_CS_GPIO,
  SPI_FRAM_CS_PIN,
  SPI_CR1_MSTR | SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0,
  SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0
};

static const uint8_t flash_conf[1] = {
    (8 << 4U) | 0x0FU
  };
#define M25Q_SUPPORTED_MEMORY_TYPE_IDS      {0xBA, 0xBB, 0x40}
#define M25Q_SUPPORTED_MANUFACTURE_IDS      {0x20, 0xef}
static const uint8_t m25q_manufacturer_ids[] = M25Q_SUPPORTED_MANUFACTURE_IDS;
static const uint8_t m25q_memory_type_ids[] = M25Q_SUPPORTED_MEMORY_TYPE_IDS;


static bool m25q_find_id(const uint8_t *set, size_t size, uint8_t element) {
  size_t i;

  for (i = 0; i < size; i++) {
    if (set[i] == element) {
      return true;
    }
  }
  return false;
}

static THD_WORKING_AREA(eeThreadStack, 256);

class EeSpi : public eeSpiStream {
public:

	//spi.send(0x9F, DUMMYBYTE, sizeof dev_id, dev_id);
	uint8_t send (uint32_t cmd, uint8_t *send, size_t n, uint8_t *recv) {
	 uint8_t buf[1];

	  spiSelect(&SPID3);
	  buf[0] = cmd;
	  spiSend(&SPID3, 1, buf);
	  spiExchange(&SPID3, n, send, recv);
	  //spiSend(&SPID3, n, send);
	  //spiReceive(&SPID3, n, p);
	  spiUnselect(&SPID3);
return recv[n];
	}

	void eeWrite(uint32_t cmd) {
		 uint8_t buf[1];
		 spiSelect(&SPID3);
		 buf[0] = cmd;
		 spiSend(&SPID3, 1, buf);
		 spiUnselect(&SPID3);
		}

	void eeTransmit(uint32_t cmd, uint8_t *recv) {
			 uint8_t tbuf[1];
			 spiSelect(&SPID3);
			 tbuf[0] = cmd;
			 spiSend(&SPID3, 1, tbuf);
			 spiReceive(&SPID3, 1, recv);
			 spiUnselect(&SPID3);

		}



	void eeRead(uint32_t cmd, size_t n, uint8_t *p) {
		uint8_t buf[1];
		spiSelect(&SPID3);
		buf[0] = cmd;
		spiSend(&SPID3, 1, buf);
		spiReceive(&SPID3, n, p);
		spiUnselect(&SPID3);

	}
};

static EeSpi spi;


uint8_t readSR() {
	 uint8_t buf[10];
	 spi.eeWrite(0x90);
	 spi.eeRead(DUMMYBYTE, sizeof buf, buf);
	 // dataBuffer = spi.eeRead(FRAM_CMD_RDSR);
	 return buf[10];
}

uint8_t readReg() {
	uint8_t data[2];
	spi.eeRead(FRAM_CMD_RDSR, sizeof data, data);

	return data[2];
	chThdSleepMilliseconds(1);
}

uint8_t readReg2() {
	 uint8_t data2[2];

	 spi.eeWrite(FRAM_CMD_WREN);
	 chThdSleepMilliseconds(10);
	 spi.eeRead(FRAM_CMD_RDSR, sizeof data2, data2);

	 return data2[2];

}


uint8_t readUniqueId() {
	uint8_t device_id[10];
	 spi.eeWrite(FRAM_CMD_RDID);

	  for (int i = 0; i < 10; i++)
	    {
	      /* Read the next manufacturer byte */
	   //   spi.eeRead(DUMMYBYTE, sizeof device_id, device_id);
		  spi.eeWrite(DUMMYBYTE);
		  //spi.eeTransmit(DUMMYBYTE, device_id);
	    }
	 // spi.eeWrite(0x90);
	  //	 spi.eeWrite(0xC2);
	  //	 spi.eeWrite(0xC3);



	 return device_id[10];
}

uint8_t send() {
	uint8_t dev_id[6];
	//spi.send(FRAM_CMD_RDID, DUMMYBYTE, sizeof dev_id, dev_id);
	return dev_id[6];
}


bool identify(void) {
	chThdSleepMilliseconds(500);
	readSR();
	readReg();
	chThdSleepMilliseconds(5);
	readUniqueId();
	//send();
	readReg2();
	return true;
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
	PAL_MODE_ALTERNATE(EFI_SPI3_AF) | PAL_STM32_OSPEED_HIGHEST);

	palSetPadMode(getBrainPort(SPI_FRAM_MISO), getBrainPinIndex(SPI_FRAM_MISO),
	PAL_MODE_ALTERNATE(EFI_SPI3_AF) | PAL_STM32_OSPEED_HIGHEST);

	palSetPadMode(getBrainPort(SPI_FRAM_MOSI), getBrainPinIndex(SPI_FRAM_MOSI),
	PAL_MODE_ALTERNATE(EFI_SPI3_AF) | PAL_STM32_OSPEED_HIGHEST);
	//efiSetPadMode("mosi", SPI_FRAM_MISO, PI_PULLDOWN);
	// HOLD & WP High
	palSetPad(GPIOE, 14);
	palSetPad(GPIOB, 10);
	spiAcquireBus(&SPID3);
	spiStart(&SPID3, &hs_spicfg);

	chThdCreateStatic(eeThreadStack, sizeof(eeThreadStack), NORMALPRIO, (tfunc_t)spi_thread_1, NULL);

}


#endif

