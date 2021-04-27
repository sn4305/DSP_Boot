/**
 * @file Flash.h
 *
 *  Created on: 20210301
 *      Author: E9981231
 */

#ifndef USERINC_FLASH_H_
#define USERINC_FLASH_H_

#include "ErrHandler.h"
#include "flash_programming_c28.h" // Flash API example header file

/**  SECTION B|C|D, BOOT0     START: 0x00082000 */
#define MEM_BOOT0_START_ADDRESS         0x00082000UL
#define BOOT0_PN_ADDRESS                0x00087FF4UL    /**< Address of Boot0 ID*/
#define BOOT0_CRC_ADDRESS               0x00087FF8UL
#define MEM_BOOT0_END_ADDRESS           0x00087FFCUL    /**< End of boot0 memory area*/
#define FLAG_BOOT0_ADDRESS              0x00087FFCUL
/**<      SECTION B|C|D, BOOT0     END: 0x03FFF */

/**  SECTION O|P|Q, BOOT1     START: 0x000C0000 */
#define MEM_BOOT1_START_ADDRESS         0x000C0000UL
#define BOOT1_PN_ADDRESS                0x000C5FF4UL    /**< Address of Boot1 ID*/
#define BOOT1_CRC_ADDRESS               0x000C5FF8UL
#define MEM_BOOT1_END_ADDRESS           0x000C5FFCUL    /**< End of boot1 memory area*/
#define FLAG_BOOT1_ADDRESS              0x000C5FFCUL
/**<      SECTION O|P|Q, BOOT1     END: 0x000C5FFE */

#define FLASH_PAGE_SIZE_HEXA            0x400   /**< Considering 512 words per page, and 2 flash addresses per word*/


#define APP_VALID_FLAG                  *(uint32 *)FLAG_APPLI_ADDRESS


#define FLASH_SECTOR_NUM                28U
#define FLASH_WORDS_PER_ROW             128U

/** Flag struct. */
typedef struct
{
    uint32_t    App_Valid_Flag;         /**< Application software Valid Flag, stored in Flash address 0x000C6000UL @see APP_VERSION_ADDRESS*/
    uint32_t    Resv0;
    uint16_t    HW_Ver;                 /**< Hardware version, stored in Flash address 0x0C6004UL @see HW_VERSION_ADDRESS*/
    uint16_t    Resv1[3];
    uint16_t    HW_Ver_CRC;             /**< Hardware version CRC*/
    uint16_t    Resv2[3];
    uint16_t    HW_Ser_H[3];            /**< Hardware serial number high */
    uint16_t    HW_Ser_L;               /**< Hardware serial number low*/
    uint16_t    HW_Ser_CRC;             /**< Hardware serial number CRC*/
}St_FLAG;

/** Flag union. */
typedef union
{
    St_FLAG     flag;                   /**< Flag struct */
    uint16_t    data[FLAG_TOTAL_LEN];
}Un_FLAG;

/** Initiate Flash BANK0&1 by calling Fapi_initializeAPI().
 * Be called in MainBoot.
*
* @see MainBoot()
*********************************************************/
void Init_Flash_Sectors(void);

/** Erase Flash sector.
*  Calls:  CMD_EraseMemory
*  @param[in] MemoryArea
*            0x20: OBC Application,
*            0x21: OBC HW Version,
*            0x22: HW_SERIAL_NUMBER,
*            0x24: OBC Bootloader.
*  @param[in] Info Boot Information.
*  @param[in] ptr_st_BootFlag Boot Flag pointer.
*************************************************/
void EraseFlash(uint8_t MemoryArea, MyBootSys Info, pSt_BootFlag ptr_st_BootFlag);

/** Active Flash Bank[BankIdx].
*  @param[in] BankIdx 0 or 1
*************************************************/
uint16_t SwitchBank(uint16_t BankIdx);

/** Write Flash.
*  Calls:  TreatData()
*  @param[in] Address Flash Address, should aligned with 0x4.
*  @param[in] Data Data buffer head.
*  @param[in] len Data length be flashed, should aligned with 0x4.
*************************************************/
uint16_t WriteFlash(uint32_t Address, uint16_t* Data, uint16_t len);

/** Write Logistic Information.
*  Calls:  CMD_TransferData
*  @param[in] Data Data buffer head.
*  @param[in] MemoryArea
*            0x21: OBC HW Version,
*            0x22: HW_SERIAL_NUMBER.
*************************************************/
void WriteLogisticInfo(volatile uint8_t *Data, uint8_t MemoryArea);

#endif /* USERINC_FLASH_H_ */
