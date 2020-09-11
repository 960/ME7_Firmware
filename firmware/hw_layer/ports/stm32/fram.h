/*
 ***************************************************************************************************
 *
 * (C) All rights reserved by RUUD BILELEKTRO, NORWAY
 *
 ***************************************************************************************************
 *
 * File: fram.h
 */
#pragma once
#include "global.h"

typedef uintptr_t flashaddr_t;

class eeSpiStream {
public:

	void send_offset(uint8_t cmd, uint32_t offset) const;
	virtual bool read(uint32_t offset, size_t size, uint8_t *buf) = 0;
	virtual bool write(uint32_t offset, size_t size, const uint8_t *buf) = 0;
};

void initEeprom(void);
int writeEeprom(flashaddr_t offset, size_t size, const uint8_t *buffer);
int readEeprom(flashaddr_t offset, size_t size, uint8_t *buffer);






