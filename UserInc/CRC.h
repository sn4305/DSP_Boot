/**
 * @file CRC.h
 *
 *  Created on: 20210309
 *      Author: E9981231
 */

#ifndef USERINC_CRC_H_
#define USERINC_CRC_H_

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_types.h"

#define FlashReadBufSize            0x400
#define VALIDATE_CRC                0xf0b8

/** Data Block CRC calculation before WriteFlash().
 * @param[in] Address Block head address, get by GetInformation().
 * @param[in] LenDataToCopy Block size, get by GetInformation().
 * @param[in] WriteMemoryArea 0x20: OBC Application, 0x24: OBC Bootloader.
 * @param[in] DataToCopy Data buffer head pointer.
 * @return CRC16 value */
uint16_t CalcCRC_Bloc(uint32_t Address, uint16_t LenDataToCopy, uint8_t WriteMemoryArea, uint8_t* DataToCopy);

/** CRC calculation for big data with length.
 * @param[in] reg_init CRC initiate value input.
 * @param[in] data data buffer head.
 * @param[in] len data buffer length. */
uint16_t CRC16(uint16_t reg_init, uint16_t *data, uint16_t len);

/** Free Data CRC calculation, be called by CRCWrite().
 * @param[in] Init Block CRC initiate value input.
 * @param[in] CodeStartAddr Start address of calculated area.
 * @param[in] len_word Length in word of calculated area.
 * @return CRC16 value */
uint16_t CalcCRC_FLASH(uint16_t Init, uint32_t CodeStartAddr, uint32_t len_word);

#endif /* USERINC_CRC_H_ */
