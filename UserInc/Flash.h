/*
 * Flash.h
 *
 *  Created on: 2021��3��1��
 *      Author: E9981231
 */

#ifndef USERINC_FLASH_H_
#define USERINC_FLASH_H_

#include "ErrHandler.h"
#include "flash_programming_c28.h" // Flash API example header file

/*      SECTION B|C|D, BOOT0     START: 0x00082000 */
#define MEM_BOOT0_START_ADDRESS         0x00082000UL
#define BOOT0_PN_ADDRESS                0x00087FF4UL    /* Address of Boot0 ID*/
#define BOOT0_CRC_ADDRESS               0x00087FF8UL
#define MEM_BOOT0_END_ADDRESS           0x00087FFCUL    /* End of boot memory area*/
#define FLAG_BOOT0_ADDRESS              0x00087FFCUL
/*      SECTION B|C|D, BOOT0     END: 0x03FFF */

/*      SECTION O|P|Q, BOOT1     START: 0x000C0000 */
#define MEM_BOOT1_START_ADDRESS         0x000C0000UL
#define BOOT1_PN_ADDRESS                0x000C5FF4UL    /* Address of Boot0 ID*/
#define BOOT1_CRC_ADDRESS               0x000C5FF8UL
#define MEM_BOOT1_END_ADDRESS           0x000C5FFCUL    /* End of boot memory area*/
#define FLAG_BOOT1_ADDRESS              0x000C5FFCUL
/*      SECTION O|P|Q, BOOT1     END: 0x000C5FFE */

#define FLASH_PAGE_SIZE_HEXA            0x400   /* Considering 512 words per page, and 2 flash addresses per word*/


#define APP_VALID_FLAG                  *(uint32 *)FLAG_APPLI_ADDRESS


#define FLASH_SECTOR_NUM                28U
#define FLASH_WORDS_PER_ROW             128U

typedef struct
{
    uint32_t    App_Valid_Flag;
    uint32_t    Resv0;
    uint16_t    HW_Ver;
    uint16_t    Resv1[3];
    uint16_t    HW_Ver_CRC;
    uint16_t    Resv2[3];
    uint16_t    HW_Ser_H[3];
    uint16_t    HW_Ser_L;
    uint16_t    HW_Ser_CRC;
}St_FLAG;

typedef union
{
    St_FLAG     flag;
    uint16_t    data[FLAG_TOTAL_LEN];
}Un_FLAG;

void Init_Flash_Sectors(void);

void EraseFlash(uint8_t MemoryArea, MyBootSys Info, pSt_BootFlag ptr_st_BootFlag);

uint16_t SwitchBank(uint16_t BankIdx);

uint16_t WriteFlash(uint32_t Address, uint16_t* Data, uint16_t len);

void WriteLogisticInfo(volatile uint8_t *Data, uint8_t MemoryArea);

#endif /* USERINC_FLASH_H_ */
