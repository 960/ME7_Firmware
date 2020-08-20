/*
  driver for RAMTRON FRAM persistent memory devices
 */
#pragma once
#include "global.h"
bool identify(void);
typedef uintptr_t flashaddr_t;

class eeSpiStream {
public:
	//virtual void WriteRegister(const char *send, size_t send_len, uint8_t *recv, uint32_t recv_len) = 0;
	//virtual void WriteReg(uint8_t regAddress, uint8_t regVal) = 0;
	//virtual void eeRead(uint32_t cmd) = 0;
	//virtual void eeWrite(uint32_t cmd, size_t n, uint8_t *p) = 0;
	//virtual uint8_t ReadRegister(uint8_t regAddr) = 0;
	//virtual uint8_t WriteRegister(const uint8_t *send, uint32_t send_len, uint8_t *recv, uint32_t recv_len) = 0;
	//virtual bool transfer(const uint8_t *send, uint32_t send_len, uint8_t *recv, uint32_t recv_len) = 0;
	//bool do_transfer(const uint8_t *send, uint8_t *recv, uint32_t len);
	//bool read(uint8_t *recv, uint32_t recv_len)
	 //   {

	   //     return transfer(nullptr, 0, recv, recv_len);
	    //}

    /**
     * set a value for a checked register
     */
    void set_checked_register(uint8_t reg, uint8_t val);
protected:
    uint8_t _read_flag = 0;
};


    // initialise the driver
    // this will retry RAMTRON_RETRIES times until successful
    bool init(void);

    // get size in bytes
  //  uint32_t get_size(void) const { return (id == UINT8_MAX) ? 0 : ramtron_ids[id].size_kbyte * 1024UL; }

    // read from device
    // this will retry RAMTRON_RETRIES times until two successive reads return the same data
    bool read(uint32_t offset, uint8_t * const buf, uint32_t size);

    // write to device
   // bool write(uint32_t offset, uint8_t const * const buf, uint32_t size);

    bool write(flashaddr_t offset, const uint8_t *buf, size_t size);

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
 //   static const struct ramtron_id ramtron_ids[];
  //  uint8_t id = UINT8_MAX;

    // perform a single device initialisation
    bool _init(void);
    // perform a single device read
    bool _read(uint32_t offset, uint8_t * const buf, uint32_t size);

  //  void send_offset(uint8_t cmd, uint32_t offset) const;




void initEeprom(void);

