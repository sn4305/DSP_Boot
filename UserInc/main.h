/**
 * \file main.h
 * \brief header file of main.c
 *  Created on: 20210225
 *  Author: E9981231
 *  Defined main.c used Macro and typedef
 */

#ifndef USERINC_MAIN_H_
#define USERINC_MAIN_H_


#include <Interrupt.h>
#include <string.h>
#include "F28x_Project.h"
#include "cancom.h"
#include "scicom.h"
#include "CRC.h"
#include "Flash.h"
#include "Timer.h"
#include <SciStack/APL/SCI_APL.h>

#define _DEFINE_PREBOOT_VER  0x0000

//#define DEMOBOARD

/* Macro definition */
#ifdef DEMOBOARD
#define BLINKY_LED_GPIO                 12      //Red
#define CAN_LED_GPIO                    13      //Blue
#endif

/**
 * \def boot_even_flag
 * \brief A macro that returns the boot0 flag value.
 ******************************************************************/
#define boot_even_flag                  *(uint32 *)FLAG_BOOT0_ADDRESS

/**
 * \def boot_odd_flag
 * \brief A macro that returns the boot1 flag value.
 ******************************************************************/
#define boot_odd_flag                   *(uint32 *)FLAG_BOOT1_ADDRESS

/**
 * \def UPDATE_APP_RQST
 * \brief Update application request flag, constant value.
 ******************************************************************/
#define UPDATE_APP_RQST                 0xC0DEFEED

#define BOOT_DELAY                      480U        /**< delay 200ms then jump to Application*/
#define BOOT_TIMEOUT                    5000U        /**< wait 5s if no cmd from gateway in boot mode*/

#define UPDATE_APP_RQST                 0xC0DEFEED

#define Clr_CanRxFlag()                 g_bCAN_RX_Flag=0

#if defined (__IS_STANDALONE) && defined(__IS_ODD)
/**< if it's standalone boot1*/
#define VALID_FLAG_ADDR                 FLAG_BOOT1_ADDRESS
#define MEMORY_END                      MEM_BOOT1_END_ADDRESS
#define BOOT_PN_ADDRESS                 BOOT1_PN_ADDRESS
#define BootIsValid                     BOOT_ODD_VALID

#define U16_SW_VER_1                    (((uint16_t)SECD_MEMORY_AREA<<8) | (1))      // MEMORY_AREA<<8 & ODD_ID
#define U16_SW_VER_2                    0x2107                                  // year(H) + month(H)
#define U16_SW_VER_3                    0x01FF                                  // Ver(0x00...0xFF) + 0xFF

#else
/**< if it's system boot or standalone boot0*/
#define VALID_FLAG_ADDR                 FLAG_BOOT0_ADDRESS
#define MEMORY_END                      MEM_BOOT0_END_ADDRESS
#define BOOT_PN_ADDRESS                 BOOT0_PN_ADDRESS
#define BootIsValid                     BOOT_EVEN_VALID

#ifdef  __IS_STANDALONE
/**< if it's standalone boot0*/
#define U16_SW_VER_1                    (((uint16_t)SECD_MEMORY_AREA<<8) | (2))      // MEMORY_AREA<<8 & EVEN_ID
#define U16_SW_VER_2                    0x2107                                  // year(H) + month(H)
#define U16_SW_VER_3                    0x01FF                                  // Ver(0x00...0xFF) + 0xFF
#else
/**< if it's system boot */
#pragma DATA_SECTION(u32BootValid,".boot_valid");
const uint32_t u32BootValid = BOOT_EVEN_VALID;
/**< if it's standalone boot, the Bootvalid flag should be writed by opposite boot CRCWrite()*/

#define U16_SW_VER_1                    _DEFINE_PREBOOT_VER
#define U16_SW_VER_2                    _DEFINE_PREBOOT_VER
#define U16_SW_VER_3                    _DEFINE_PREBOOT_VER
#endif
#endif

/**
 * \var g_u32UpdataFlag
 * \brief Update request flag.
 *
 * This value will be placed in RAM address 0x008000.
 * normally it will be changed by application software to indicate
 * application software updating is request from gateway.
 ******************************************************************/
#pragma DATA_SECTION(g_u32UpdataFlag,".updataflag");
uint32_t g_u32UpdataFlag;

/**
 * \typedef BootMachineStates
 * \brief A ENUM of Boot Machine.
 *
 ******************************************************************/
typedef enum {
    State_TRANSITION,   /**< StateMachine entry point, a branch node. */
    State_DEFAULT,      /**< StateMachine default state, when there is no application software and Boot mode request. */
    State_BOOT          /**< StateMachine Boot, when received Boot mode request. */
} BootMachineStates;

#endif /* USERINC_MAIN_H_ */
