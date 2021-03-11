/*
 * CRC.h
 *
 *  Created on: 2021Äê3ÔÂ9ÈÕ
 *      Author: E9981231
 */

#ifndef USERINC_CRC_H_
#define USERINC_CRC_H_

#include <stdbool.h>
#include <stdint.h>
#include "Flash.h"

#define FlashReadBufSize            0x400



uint16_t CalcCRC_Bloc(uint32_t Address, uint16_t LenDataToCopy, uint8_t WriteMemoryArea, uint8_t* DataToCopy);

uint16_t CRC16(uint16_t reg_init, uint8_t *data, uint16_t len);

uint16_t CalcCRC_FLASH(uint16_t Init, uint16_t CodeStartAddr, uint16_t len_word);

#endif /* USERINC_CRC_H_ */
