/*
 * main.h
 *
 *  Created on: 2021��2��25��
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

/* Error Codes*/
#define NO_ERROR                    0x00
#define SECURITY_LOCKED             0x02
#define WRONG_KEY                   0x03
#define WRONG_CRC                   0x04
#define WRITING_INVALID             0x05
#define WRONG_REQUEST_FORMAT        0x06
#define MEMORY_NOT_BLANK            0x09
#define ID_NOT_SUPPORTED            0x10
#define TIMEOUT                     0x11

#define MEM_APPCODE_START_ADDRESS   0x088000

/* GOTO start address of applicative area*/
#define StartApplication() ExitBoot(MEM_APPCODE_START_ADDRESS)



#pragma DATA_SECTION(u40BootVersion,".boot_ver");
const uint16 u40BootVersion[3] = {0xAABB, 0xCCDD, 0x56FF};

#pragma DATA_SECTION(u32UpdataFlag,".updataflag");
uint32 u32UpdataFlag;


typedef enum {
    State_TRANSITION,
    State_DEFAULT,
    State_BOOT
} BootMachineStates;



#endif /* USERINC_MAIN_H_ */
