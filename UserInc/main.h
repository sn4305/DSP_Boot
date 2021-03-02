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

extern void ExitBoot(uint32 EntryAddr);

/* Macro definition */
#ifdef DEMOBOARD
#define BLINKY_LED_GPIO    12      //Red
#define CAN_LED_GPIO       13      //Blue
#endif

#define APP_VALID                      0xA5C6BD72  /* Value of Flag when application is valid*/
#define FLAG_APPLI_ADDRESS             0x0B8000    /* Address of application valid Flag*/
#define UPDATE_APP_RQST                0xC0DEFEED
#define APP_VALID_FLAG                 *(uint32 *)FLAG_APPLI_ADDRESS

#define FLASH_PAGE_SIZE_HEXA            0x400   /* Considering 512 words per page, and 2 flash addresses per word*/


#define HW_VERSION_SIZE                2            /* Size in bytes of Hardware version*/
#define HW_SERIAL_NUMBER_SIZE          7            /* Size in bytes of Hardware Serial Number*/



#define MEM_APPCODE_START_ADDRESS   0x88000
#define MEM_BOOTCODE_START_ADDRESS  0x80000

/* GOTO start address of applicative area*/
#define StartApplication() ExitBoot(MEM_APPCODE_START_ADDRESS)
/* GOTO start address of Bootloader area*/
#define RESET() ExitBoot(MEM_BOOTCODE_START_ADDRESS)

#define Clr_CanRxFlag() CAN_RX_Flag=0

#pragma DATA_SECTION(u40BootVersion,".boot_ver");
const uint16_t u40BootVersion[3] = {0xAABB, 0xCCDD, 0x56FF};

#pragma DATA_SECTION(u32UpdataFlag,".updataflag");
uint32_t u32UpdataFlag;


typedef enum {
    State_TRANSITION,
    State_DEFAULT,
    State_BOOT
} BootMachineStates;



#endif /* USERINC_MAIN_H_ */
