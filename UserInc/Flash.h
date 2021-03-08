/*
 * Flash.h
 *
 *  Created on: 2021Äê3ÔÂ1ÈÕ
 *      Author: E9981231
 */

#ifndef USERINC_FLASH_H_
#define USERINC_FLASH_H_

#include "ErrHandler.h"

#define FLASH_PAGE_SIZE_HEXA            0x400   /* Considering 512 words per page, and 2 flash addresses per word*/

void Init_Flash_Sectors(void);

void WriteLogisticInfo(void);

#endif /* USERINC_FLASH_H_ */
