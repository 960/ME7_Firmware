/**
 * @file	mpu_util.h
 *
 * @date Jul 27, 2014
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

#include "stm32f7xx_hal_flash_ex.h"



#define SPI_BAUDRATEPRESCALER_2         (0x00000000U)
#define SPI_BAUDRATEPRESCALER_4         (SPI_CR1_BR_0)  // 27MHz
#define SPI_BAUDRATEPRESCALER_8         (SPI_CR1_BR_1)
#define SPI_BAUDRATEPRESCALER_16        (SPI_CR1_BR_1 | SPI_CR1_BR_0)
#define SPI_BAUDRATEPRESCALER_32        (SPI_CR1_BR_2)
#define SPI_BAUDRATEPRESCALER_64        (SPI_CR1_BR_2 | SPI_CR1_BR_0)
#define SPI_BAUDRATEPRESCALER_128       (SPI_CR1_BR_2 | SPI_CR1_BR_1)  //421.875kHz
#define SPI_BAUDRATEPRESCALER_256       (SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0)


#define SPI_CR1_8BIT_MODE 0
#define SPI_CR2_8BIT_MODE (SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0)

#define SPI_CR1_16BIT_MODE 0
#define SPI_CR2_16BIT_MODE SPI_CR2_DS_3 | SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0

/* 3 x 8-bit transfer */
#define SPI_CR1_24BIT_MODE 0
#define SPI_CR2_24BIT_MODE SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0
