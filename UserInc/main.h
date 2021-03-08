/*
 * main.h
 *
 *  Created on: 2021Äê2ÔÂ25ÈÕ
 *      Author: E9981231
 */

#ifndef USERINC_MAIN_H_
#define USERINC_MAIN_H_

#include <string.h>
#include "F28x_Project.h"
#include "cancom.h"
#include "Flash.h"
#include "Timer.h"
#include "interrupt.h"


/* Macro definition */
#ifdef DEMOBOARD
#define BLINKY_LED_GPIO    12      //Red
#define CAN_LED_GPIO       13      //Blue
#endif

/*      SECTION B|C|D, BOOT0     START: 0x00082000 */
#define MEM_BOOT0_START_ADDRESS         0x00082000
#define BOOT0_PN_ADDRESS                0x00087FFA    /* Address of Boot0 ID*/
#define BOOT0_CRC_ADDRESS               0x00087FFD
#define MEM_BOOT0_END_ADDRESS           0x00087FFD    /* End of boot memory area*/
#define FLAG_BOOT0_ADDRESS              0x00087FFE
/*      SECTION B|C|D, BOOT0     END: 0x03FFF */

/*      SECTION O|P|Q, BOOT1     START: 0x000C0000 */
#define MEM_BOOT1_START_ADDRESS         0x000C0000
#define BOOT1_PN_ADDRESS                0x000C5FFA    /* Address of Boot0 ID*/
#define BOOT1_CRC_ADDRESS               0x000C5FFD
#define MEM_BOOT1_END_ADDRESS           0x000C5FFD    /* End of boot memory area*/
#define FLAG_BOOT1_ADDRESS              0x000C5FFE
/*      SECTION O|P|Q, BOOT1     END: 0x000C5FFE */

#define boot_even_flag                  *(uint32 *)FLAG_BOOT0_ADDRESS
#define boot_odd_flag                   *(uint32 *)FLAG_BOOT1_ADDRESS
#define BootEvenValid                   0xBA5EBA11
#define BootOddValid                    0xC0DEBA5E

#define APP_VALID                       0xA5C6BD72      /* Value of Flag when application is valid*/
#define FLAG_APPLI_ADDRESS              0x000C6000      /* Address of application valid Flag*/
#define UPDATE_APP_RQST                 0xC0DEFEED
#define APP_VALID_FLAG                  *(uint32 *)FLAG_APPLI_ADDRESS

#define MEM_APPCODE_START_ADDRESS       0x88000
#define MEM_PREBOOT_START_ADDRESS       0x80000

extern void ExitBoot(uint32 EntryAddr);

/* GOTO start address of applicative area*/
#define StartApplication()              ExitBoot(MEM_APPCODE_START_ADDRESS)
/* GOTO start address of PreBootloader area*/
#define RESET()                         ExitBoot(MEM_PREBOOT_START_ADDRESS)
/* GOTO start address of Bootloader0 area*/
#define StartBootEven()                 ExitBoot(MEM_BOOT0_START_ADDRESS)
/* GOTO start address of Bootloader1 area*/
#define StartBootOdd()                  ExitBoot(MEM_BOOT1_START_ADDRESS)

#define Clr_CanRxFlag() CAN_RX_Flag=0

#if defined (__IS_STANDALONE) && defined(__IS_ODD)
#define VALID_FLAG_ADDR                 FLAG_BOOT1_ADDRESS
#define MEMORY_END                      MEM_BOOT1_END_ADDRESS
#define BOOT_PN_ADDRESS                 BOOT1_PN_ADDRESS
#define BootIsValid                     BootOddValid
#pragma DATA_SECTION(u40BootVersion,".boot_ver");
const uint16_t u40BootVersion[3] = {0x0011, 0x2233, 0x44FF};
#pragma DATA_SECTION(u32BootValid,".boot_valid");
const uint32_t u32BootValid = BootOddValid;
#else
#define VALID_FLAG_ADDR                 FLAG_BOOT0_ADDRESS
#define MEMORY_END                      MEM_BOOT0_END_ADDRESS
#define BOOT_PN_ADDRESS                 BOOT0_PN_ADDRESS
#define BootIsValid                     BootEvenValid
#pragma DATA_SECTION(u40BootVersion,".boot_ver");
const uint16_t u40BootVersion[3] = {0x0101, 0x0301, 0x00FF};
#pragma DATA_SECTION(u32BootValid,".boot_valid");
const uint32_t u32BootValid = BootEvenValid;

#endif

#pragma DATA_SECTION(u32UpdataFlag,".updataflag");
uint32_t u32UpdataFlag;


typedef enum {
    State_TRANSITION,
    State_DEFAULT,
    State_BOOT
} BootMachineStates;


#endif /* USERINC_MAIN_H_ */
