/*
  driver for RAMTRON FRAM persistent memory devices
 */
#pragma once
#include "global.h"
bool identify(void);


#define RAMTRON_EMULATE_SECTOR_SHIFT  9
#define RAMTRON_EMULATE_PAGE_SHIFT    9

/* RAMTRON Indentification register values */

#define RAMTRON_MANUFACTURER         0x7F
#define RAMTRON_MEMORY_TYPE          0xC2

/* Instructions:
 *      Command          Value       N Description             Addr Dummy Data */
#define RAMTRON_WREN      0x06    /* 1 Write Enable              0   0     0 */
#define RAMTRON_WRDI      0x04    /* 1 Write Disable             0   0     0 */
#define RAMTRON_RDSR      0x05    /* 1 Read Status Register      0   0     >=1 */
#define RAMTRON_WRSR      0x01    /* 1 Write Status Register     0   0     1 */
#define RAMTRON_READ      0x03    /* 1 Read Data Bytes           A   0     >=1 */
#define RAMTRON_FSTRD     0x0b    /* 1 Higher speed read         A   1     >=1 */
#define RAMTRON_WRITE     0x02    /* 1 Write                     A   0     1-256 */
#define RAMTRON_SLEEP     0xb9    // TODO:
#define RAMTRON_RDID	  0x9f    /* 1 Read Identification       0   0     1-3 */
#define RAMTRON_SN        0xc3    // TODO:

/* Status register bit definitions */

#define RAMTRON_SR_WIP            (1 << 0)                /* Bit 0: Write in progress bit */
#define RAMTRON_SR_WEL            (1 << 1)                /* Bit 1: Write enable latch bit */
#define RAMTRON_SR_BP_SHIFT       (2)                     /* Bits 2-4: Block protect bits */
#define RAMTRON_SR_BP_MASK        (7 << RAMTRON_SR_BP_SHIFT)
#  define RAMTRON_SR_BP_NONE      (0 << RAMTRON_SR_BP_SHIFT) /* Unprotected */
#  define RAMTRON_SR_BP_UPPER64th (1 << RAMTRON_SR_BP_SHIFT) /* Upper 64th */
#  define RAMTRON_SR_BP_UPPER32nd (2 << RAMTRON_SR_BP_SHIFT) /* Upper 32nd */
#  define RAMTRON_SR_BP_UPPER16th (3 << RAMTRON_SR_BP_SHIFT) /* Upper 16th */
#  define RAMTRON_SR_BP_UPPER8th  (4 << RAMTRON_SR_BP_SHIFT) /* Upper 8th */
#  define RAMTRON_SR_BP_UPPERQTR  (5 << RAMTRON_SR_BP_SHIFT) /* Upper quarter */
#  define RAMTRON_SR_BP_UPPERHALF (6 << RAMTRON_SR_BP_SHIFT) /* Upper half */
#  define RAMTRON_SR_BP_ALL       (7 << RAMTRON_SR_BP_SHIFT) /* All sectors */
#define RAMTRON_SR_SRWD           (1 << 7)                /* Bit 7: Status register write protect */

#define RAMTRON_DUMMY     0xa5


class eeSpiStream {
public:

	virtual void WriteRegister(const uint8_t *send, uint32_t send_len, uint8_t *recv, uint32_t recv_len) = 0;
	virtual bool transfer(const uint8_t *send, uint32_t send_len,
	                          uint8_t *recv, uint32_t recv_len) = 0;
	bool do_transfer(const uint8_t *send, uint8_t *recv, uint32_t len);
	bool read(uint8_t *recv, uint32_t recv_len)
	    {

	        return transfer(nullptr, 0, recv, recv_len);
	    }

    /**
     * set a value for a checked register
     */
    void set_checked_register(uint8_t reg, uint8_t val);
protected:
    uint8_t _read_flag = 0;
};


class AP_RAMTRON {
public:
    // initialise the driver
    // this will retry RAMTRON_RETRIES times until successful
    bool init(void);

    // get size in bytes
    uint32_t get_size(void) const { return (id == UINT8_MAX) ? 0 : ramtron_ids[id].size_kbyte * 1024UL; }

    // read from device
    // this will retry RAMTRON_RETRIES times until two successive reads return the same data
    bool read(uint32_t offset, uint8_t * const buf, uint32_t size);

    // write to device
    bool write(uint32_t offset, uint8_t const * const buf, uint32_t size);

private:

    enum class RDID_type :uint8_t {
        Cypress,
        Fujitsu,
    };

    struct ramtron_id {
        uint8_t id1;
        uint8_t id2;
        uint16_t size_kbyte;
        uint8_t addrlen;
        RDID_type rdid_type;
    };
    static const struct ramtron_id ramtron_ids[];
    uint8_t id = UINT8_MAX;

    // perform a single device initialisation
    bool _init(void);
    // perform a single device read
    bool _read(uint32_t offset, uint8_t * const buf, uint32_t size);

    void send_offset(uint8_t cmd, uint32_t offset) const;

};


void initEeprom(void);

