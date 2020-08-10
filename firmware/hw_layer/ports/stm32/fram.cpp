#include "global.h"
#include "fram.h"
#include "engine.h"
#include "pin_repository.h"
#include "mpu_util.h"
EXTERN_ENGINE;

static void ramtron_lock(struct spi_dev_s *dev);
static inline void ramtron_unlock(struct spi_dev_s *dev);
static inline int ramtron_readid(struct ramtron_dev_s *priv);
static void ramtron_waitwritecomplete(struct ramtron_dev_s *priv);
static void ramtron_writeenable(struct ramtron_dev_s *priv);
static inline void ramtron_pagewrite(struct ramtron_dev_s *priv,
                                     const uint8_t *buffer, off_t offset);

static int ramtron_erase(struct mtd_dev_s *dev, off_t startblock, size_t nblocks);
static size_t ramtron_bread(struct mtd_dev_s *dev, off_t startblock,
                             size_t nblocks,  uint8_t *buf);
static size_t ramtron_bwrite(struct mtd_dev_s *dev, off_t startblock,
                              size_t nblocks,  const uint8_t *buf);
static size_t ramtron_read(struct mtd_dev_s *dev, off_t offset, size_t nbytes,
                            uint8_t *buffer);
static int ramtron_ioctl(struct mtd_dev_s *dev, int cmd, unsigned long arg);



struct mtdconfig_struct_s
{
   struct mtd_dev_s *mtd;  /* Contained MTD interface */
  //sem_t        exclsem;       /* Supports mutual exclusion */
  uint32_t     blocksize :14; /* Size of blocks in contained MTD */
  uint32_t     erasesize :18; /* Size of erase block  in contained MTD */
  size_t       nblocks;       /* Number of blocks available */
  size_t       neraseblocks;  /* Number of erase blocks available */
  off_t        readoff;       /* Read offset (for hexdump) */
  uint8_t *buffer;        /* Temp block read buffer */
};

struct mtdconfig_header_s
{
  uint8_t      flags;         /* Entry control flags */
  uint8_t      instance;      /* Instance of the item */
  uint16_t     id;            /* ID of the config data item */
  uint16_t     len;           /* Length of the data block */
} packed_struct;



struct ramtron_parts_s
{
  const char *name;
  uint8_t     id1;
  uint8_t     id2;
  uint32_t    size;
  uint8_t     addr_len;
  uint32_t    speed;
};

/* This type represents the state of the MTD device.  The struct mtd_dev_s
 * must appear at the beginning of the definition so that you can freely
 * cast between pointers to struct mtd_dev_s and struct ramtron_dev_s.
 */

struct ramtron_dev_s
{
	  struct mtd_dev_s *mtd;                    /* MTD interface */
	  struct spi_dev_s *dev;               /* Saved SPI interface instance */


  uint8_t sectorshift;
  uint8_t pageshift;
  uint16_t nsectors;
  uint32_t npages;
  const struct ramtron_parts_s *part;  /* Part instance */
};

/************************************************************************************
 * Supported Part Lists
 ************************************************************************************/

/* Defines the initial speed compatible with all devices. In case of RAMTRON
 * the defined devices within the part list have all the same speed.
 */

#define RAMTRON_INIT_CLK_MAX    40000000UL

static const struct ramtron_parts_s g_ramtron_parts[] =
{
  {
    "FM25V01",                    /* name */
    0x21,                         /* id1 */
    0x00,                         /* id2 */
    16L*1024L,                    /* size */
    2,                            /* addr_len */
    40000000                      /* speed */
  },
  {
    "FM25V02",                    /* name */
    0x22,                         /* id1 */
    0x00,                         /* id2 */
    32L*1024L,                    /* size */
    2,                            /* addr_len */
    40000000                      /* speed */
  },
  {
    "FM25VN02",                   /* name */
    0x22,                         /* id1 */
    0x01,                         /* id2 */
    32L*1024L,                    /* size */
    2,                            /* addr_len */
    40000000                      /* speed */
  },
  {
    "FM25V05",                    /* name */
    0x23,                         /* id1 */
    0x00,                         /* id2 */
    64L*1024L,                    /* size */
    2,                            /* addr_len */
    40000000                      /* speed */
  },
  {
    "FM25VN05",                   /* name */
    0x23,                         /* id1 */
    0x01,                         /* id2 */
    64L*1024L,                    /* size */
    2,                            /* addr_len */
    40000000                      /* speed */
  },
  {
    "FM25V10",                    /* name */
    0x24,                         /* id1 */
    0x00,                         /* id2 */
    128L*1024L,                   /* size */
    3,                            /* addr_len */
    40000000                      /* speed */
  },
  {
    "FM25VN10",                   /* name */
    0x24,                         /* id1 */
    0x01,                         /* id2 */
    128L*1024L,                   /* size */
    3,                            /* addr_len */
    40000000                      /* speed */
  },
  {
    "MB85RS1MT",                  /* name */
    0x27,                         /* id1 */
    0x03,                         /* id2 */
    128L*1024L,                   /* size */
    3,                            /* addr_len */
    25000000                      /* speed */
  },
  {
     "25W256",                  /* name */
     0x28,                         /* id1 */
     0x01,                         /* id2 */
	 32L*1024L,                    /* size */
     3,                            /* addr_len */
	 40000000                      /* speed */
   },
#ifdef CONFIG_RAMTRON_FRAM_NON_JEDEC
  {
    "FM25H20",                    /* name */
    0xff,                         /* id1 */
    0xff,                         /* id2 */
    256L*1024L,                   /* size */
    3,                            /* addr_len */
    40000000                      /* speed */
  },
  {
    NULL,                         /* name */
    0,                            /* id1 */
    0,                            /* id2 */
    0,                            /* size */
    0,                            /* addr_len */
    0                             /* speed */
  }
#endif
};
#define SPI_BUFFERS_SIZE    128U

static uint8_t txbuf[SPI_BUFFERS_SIZE] NO_CACHE;
static uint8_t rxbuf[SPI_BUFFERS_SIZE] NO_CACHE;

static const SPIConfig hs_spicfg = {
  false,
  NULL,
  GPIOE,
  15,
  SPI_CR1_MSTR | SPI_CR1_CPHA | SPI_CR1_BR_2 | SPI_CR1_BR_1,
  SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0
};
static THD_WORKING_AREA(eeThreadStack, 256);

static uint8_t eeRead(uint8_t reg) {
	spiSelect(&SPID1);
	unsigned i;
	/* Preparing data buffer and flushing cache.*/
	for (i = 0; i < SPI_BUFFERS_SIZE; i++) {
	txbuf[i] = (uint8_t)i;
	}
	txbuf[0] = reg;
	spiSend(&SPID1, 1, txbuf);
	spiReceive(&SPID1, 1, rxbuf);
	spiUnselect(&SPID1);

	return rxbuf[i];
}

static void eeWrite(const uint8_t *send, uint32_t send_len, uint8_t *recv, uint32_t recv_len) {

	unsigned i;
	 /* Preparing data buffer and flushing cache.*/
		for (i = 0; i < SPI_BUFFERS_SIZE; i++) {
		txbuf[i] = (uint8_t)i;
}

		txbuf[0] = 0x9f;
		//rxbuf[i] = (recv)i;
		 /* Slave selection and data exchange.*/
		spiSelect(&SPID1);
		spiExchange(&SPID1, 1, txbuf, recv);
		spiUnselect(&SPID1);

}
class EeSpi : public eeSpiStream {
public:
	uint8_t eeReadId(uint8_t reg) {
			return eeRead(reg);
		}

	bool read_registers(uint8_t first_reg, uint8_t *recv, uint32_t recv_len)
	   {
	       first_reg |= _read_flag;
	       return transfer(&first_reg, 1, recv, recv_len);
	   }

	bool write_register(uint8_t reg, uint8_t val, bool checked=false)
	   {
	       uint8_t buf[2] = { reg, val };
	       if (checked) {
	           set_checked_register(reg, val);
	       }

	       return transfer(buf, sizeof(buf), nullptr, 0);
	   }


	bool do_transfer(const uint8_t *send, uint8_t *recv, uint32_t len) {
		unsigned i;
			 /* Preparing data buffer and flushing cache.*/
				for (i = 0; i < SPI_BUFFERS_SIZE; i++) {
				txbuf[i] = (uint8_t)i;
		}

		txbuf[0] = *send;
		*recv = rxbuf[len];
		spiSelect(&SPID1);
		spiSend(&SPID1, 1, txbuf);
		spiReceive(&SPID1, len, rxbuf);

		//spiExchange(&SPID1, len, txbuf, rxbuf);
		spiUnselect(&SPID1);
		return *recv;
	}

	bool transfer(const uint8_t *send, uint32_t send_len, uint8_t *recv, uint32_t recv_len) {
		if ((send_len == recv_len && send == recv) || !send || !recv) {
		        // simplest cases, needed for DMA
		        return do_transfer(send, recv, recv_len?recv_len:send_len);
		    }
		    uint8_t buf[send_len+recv_len];
		    if (send_len > 0) {
		        memcpy(buf, send, send_len);
		    }
		    if (recv_len > 0) {
		        memset(&buf[send_len], 0, recv_len);
		    }
		    bool ret = do_transfer(buf, buf, send_len+recv_len);
		    if (ret && recv_len > 0) {
		        memcpy(recv, &buf[send_len], recv_len);
		    }
		    return ret;
		}


	void WriteRegister(const uint8_t *send, uint32_t send_len, uint8_t *recv, uint32_t recv_len) {
		eeWrite(send, send_len, recv, recv_len);
	}
};

static EeSpi spi;

static inline int ramtron_readid(struct ramtron_dev_s *priv)
{
  uint16_t manufacturer;
  uint16_t memory;
  uint16_t capacity;
  uint16_t part;
  int i;


  /* Lock the SPI bus, configure the bus, and select this FLASH part. */



  /* Send the "Read ID (RDID)" command */
  uint16_t mfg = spi.eeReadId(RAMTRON_RDID);
  spiSelect(&SPID1);

  txbuf[0] = RAMTRON_RDID;
  txbuf[1] = RAMTRON_DUMMY;
  spiSend(&SPID1, 1, txbuf);

 uint16_t buf[6];
  for (i = 0; i < 6; i++)
    {
      /* Read the next manufacturer byte */
	 buf[i] = spi.eeReadId(RAMTRON_DUMMY);

    }
  manufacturer = mfg;
  memory   = buf[1];
  capacity = buf[2];  /* fram.id1 */
  part     = buf[3];  /* fram.id2 */

  /* Deselect the FLASH and unlock the bus */

  spiUnselect(&SPID1);


  /* Select part from the part list */

  for (priv->part = g_ramtron_parts;
       priv->part->name != NULL &&
         !(priv->part->id1 == capacity && priv->part->id2 == part);
       priv->part++);

  if (priv->part->name)
    {
      UNUSED(manufacturer); /* Eliminate warnings when debug is off */
      UNUSED(memory);       /* Eliminate warnings when debug is off */




      priv->sectorshift = RAMTRON_EMULATE_SECTOR_SHIFT;
      priv->nsectors    = priv->part->size / (1 << RAMTRON_EMULATE_SECTOR_SHIFT);
      priv->pageshift   = RAMTRON_EMULATE_PAGE_SHIFT;
      priv->npages      = priv->part->size / (1 << RAMTRON_EMULATE_PAGE_SHIFT);
      return true;
    }

  return false;
}


#define RAMTRON_RETRIES 10
#define RAMTRON_DELAY_MS 10

/*
  list of supported devices. Thanks to NuttX ramtron driver
 */
const AP_RAMTRON::ramtron_id AP_RAMTRON::ramtron_ids[] = {
    { 0x21, 0x00,  16, 2, RDID_type::Cypress }, // FM25V01
    { 0x21, 0x08,  16, 2, RDID_type::Cypress }, // FM25V01A
    { 0x22, 0x00,  32, 2, RDID_type::Cypress }, // FM25V02
    { 0x22, 0x08,  32, 2, RDID_type::Cypress }, // FM25V02A
    { 0x22, 0x48,  32, 2, RDID_type::Cypress }, // FM25V02A - Extended Temperature Version
    { 0x22, 0x01,  32, 2, RDID_type::Cypress }, // FM25VN02
    { 0x23, 0x00,  64, 2, RDID_type::Cypress }, // FM25V05
    { 0x23, 0x01,  64, 2, RDID_type::Cypress }, // FM25VN05
    { 0x24, 0x00, 128, 3, RDID_type::Cypress }, // FM25V10
    { 0x24, 0x01, 128, 3, RDID_type::Cypress }, // FM25VN10
    { 0x25, 0x08, 256, 3, RDID_type::Cypress }, // FM25V20A
    { 0x26, 0x08, 512, 3, RDID_type::Cypress }, // CY15B104Q

    { 0x27, 0x03, 128, 3, RDID_type::Fujitsu }, // MB85RS1MT
    { 0x05, 0x09,  32, 2, RDID_type::Fujitsu }, // MB85RS256B
    { 0x24, 0x03,  16, 2, RDID_type::Fujitsu }, // MB85RS128TY
};


// initialise the driver
bool AP_RAMTRON::init(void)
{
    struct cypress_rdid {
        uint8_t manufacturer[6];
        uint8_t memory;
        uint8_t id1;
        uint8_t id2;
    };


    uint8_t rdid[sizeof(cypress_rdid)];

    if (!spi.read_registers(RAMTRON_RDID, rdid, sizeof(rdid))) {
        return false;
    }

    for (uint8_t i = 0; i < ARRAY_SIZE(ramtron_ids); i++) {
        if (ramtron_ids[i].rdid_type == RDID_type::Cypress) {
            const cypress_rdid *cypress = (const cypress_rdid *)rdid;
            if (ramtron_ids[i].id1 == cypress->id1 &&
                ramtron_ids[i].id2 == cypress->id2) {
                id = i;
                break;
            }
        }

    }

    if (id == UINT8_MAX) {

        return false;
    }

    return true;
}

/*
  send a command and offset
 */
void AP_RAMTRON::send_offset(uint8_t cmd, uint32_t offset) const
{
    if (ramtron_ids[id].addrlen == 3) {
        uint8_t b[4] = { cmd, uint8_t((offset>>16)&0xFF), uint8_t((offset>>8)&0xFF), uint8_t(offset&0xFF) };
      spi.WriteRegister(b, sizeof(b), nullptr, 0);
    } else /* len 2 */ {
        uint8_t b[3] = { cmd, uint8_t((offset>>8)&0xFF), uint8_t(offset&0xFF) };
       spi.WriteRegister(b, sizeof(b), nullptr, 0);
    }
}

// read from device
bool AP_RAMTRON::read(uint32_t offset, uint8_t *buf, uint32_t size) {
    // Don't allow reads outside of the FRAM memory.
    // NOTE: The FRAM devices will wrap back to address 0x0000 if they read past
    // the end of their internal memory, so while we'll get data back, it won't
    // be what we expect.
    if ((size > get_size()) ||
        (offset > (get_size() - size))) {
        return false;
    }
    const uint8_t maxread = 128;
    while (size > maxread) {
        if (!read(offset, buf, maxread)) {
            return false;
        }
        offset += maxread;
        buf += maxread;
        size -= maxread;
    }
return 0;
}

// write to device
bool AP_RAMTRON::write(uint32_t offset, const uint8_t *buf, uint32_t size)
{
    // Don't allow writes outside of the FRAM memory.
    // NOTE: The FRAM devices will wrap back to address 0x0000 if they write past
    // the end of their internal memory, so we could accidentally overwrite the
    // wrong memory location.
    if ((size > get_size()) ||
        (offset > (get_size() - size))) {
        return false;
    }

     //   spi.WriteRegister(RAMTRON_WREN, 1, nullptr, 0);

        send_offset(RAMTRON_WRITE, offset);
        spi.WriteRegister(buf, size, nullptr, 0);
        /*
          verify first 32 bytes of every write using a crc
         */
        uint8_t rbuf[32] {};

        send_offset(RAMTRON_READ, offset);
     //  spi.WriteRegister(nullptr, 0, rbuf);

   return 0;
}

static AP_RAMTRON eeprom;


struct mtd_dev_s *ramtron_initialize()
{
  struct ramtron_dev_s *priv;



  /* Allocate a state structure (we allocate the structure instead of using
   * a fixed, static allocation so that we can handle multiple FLASH devices.
   * The current implementation would handle only one FLASH part per SPI
   * device (only because of the SPIDEV_FLASH definition) and so would have
   * to be extended to handle multiple FLASH parts on the same SPI bus.
   */

  priv = (struct ramtron_dev_s *) (sizeof(struct ramtron_dev_s));
  if (priv)
    {
      /* Initialize the allocated structure. (unsupported methods were
       * nullified by kmm_zalloc).
       */

     // priv->mtd.erase  = ramtron_erase;
      //priv->mtd.bread  = ramtron_bread;
      //priv->mtd.bwrite = ramtron_bwrite;
      //priv->mtd.read   = ramtron_read;
      //priv->mtd.ioctl  = ramtron_ioctl;
      //priv->dev        = dev;

      /* Deselect the FLASH */

    //  spiUnselect(&SPID1);

      /* Identify the FLASH chip and get its capacity */

      if (ramtron_readid(priv) != true)
        {
          /* Unrecognized! Discard all of that work we just did and return NULL */

         // kmm_free(priv);
          priv = NULL;
        }
    }

  /* Register the MTD with the procfs system if enabled */

#ifdef CONFIG_MTD_REGISTRATION
  mtd_register(&priv->mtd, "ramtron");
#endif

  /* Return the implementation-specific state structure as the MTD device */


  return (struct mtd_dev_s *)priv;
}

bool identify(void) {
	ramtron_initialize();
	        {
	          /* Unrecognized! Discard all of that work we just did and return NULL */

	         // kmm_free(priv);

	        }
	 return true;
}

static msg_t eeThread(void) {
  chRegSetThreadName("SPI thread 1");
  while (1) {
 	    chThdSleepMilliseconds(500);
 	  }
  return -1;
}

void initEeprom(void) {


	/* init SPI pins */
	palSetPad(SPI_FRAM_CS_GPIO, SPI_FRAM_CS_PIN);

	palSetPadMode(SPI_FRAM_CS_GPIO, SPI_FRAM_CS_PIN,
		PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);

	palSetPadMode(getBrainPort(SPI_FRAM_SCK), getBrainPinIndex(SPI_FRAM_SCK),
		PAL_MODE_ALTERNATE(EFI_SPI1_AF) | PAL_STM32_OSPEED_HIGHEST);

	palSetPadMode(getBrainPort(SPI_FRAM_MISO), getBrainPinIndex(SPI_FRAM_MISO),
		PAL_MODE_ALTERNATE(EFI_SPI1_AF) | PAL_STM32_OSPEED_HIGHEST);

	palSetPadMode(getBrainPort(SPI_FRAM_MOSI), getBrainPinIndex(SPI_FRAM_MOSI),
		PAL_MODE_ALTERNATE(EFI_SPI1_AF) | PAL_STM32_OSPEED_HIGHEST);
	/* Deactivate WP */
	palSetPad(getBrainPort(SPI_FRAM_WP), getBrainPinIndex(SPI_FRAM_WP));
	palSetPadMode(getBrainPort(SPI_FRAM_WP), getBrainPinIndex(SPI_FRAM_WP),
		PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
	/* Deactivate HOLD */
	palSetPad(getBrainPort(SPI_FRAM_HOLD), getBrainPinIndex(SPI_FRAM_HOLD));
	palSetPadMode(getBrainPort(SPI_FRAM_HOLD), getBrainPinIndex(SPI_FRAM_HOLD),
		PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
	// WP and Hold
	enginePins.cj125ModePin.setValue(1);
	enginePins.cj125ModePin2.setValue(1);
	 palSetLine(GPIOE_ZIO_D38);
	 palSetLine(GPIOB_ZIO_D36);

	spiStart(&SPID1, &hs_spicfg);


	chThdCreateStatic(eeThreadStack, sizeof(eeThreadStack), NORMALPRIO, (tfunc_t)(void*) eeThread, NULL);
	identify();
		eeprom.init();
		eeprom.get_size();
}
