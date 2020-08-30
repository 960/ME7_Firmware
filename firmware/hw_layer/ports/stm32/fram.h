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
void initEeprom(void);

int writeFullEeprom(flashaddr_t offset, size_t size, char *buffer);
int readFullEeprom(flashaddr_t offset, char *buffer, size_t size);
int writeEeprom(flashaddr_t offset, size_t size, const uint8_t *buffer);







