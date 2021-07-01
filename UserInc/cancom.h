/**
 * @file cancom.h
 *
 *  Created on: 20210226
 *      Author: E9981231
 */

#ifndef USERINC_CANCOM_H_
#define USERINC_CANCOM_H_

#include "F28x_Project.h"
#include "F021_F2837xS_C28x.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_can.h"
#include "driverlib/can.h"

//#define DEMOBOARD

/** CAN Id */
#define GENERAL_RESP      0xCEF01A0                     /**< General response CAN ID*/
#define LOGI_RESP         0xCEF02A0                     /**< Logistic response CAN ID*/
#define DIAG_SESSION      0xCFF03A0                     /**< Diagnostic response CAN ID*/

#define EraseMemory                    0x4E0FF01        /**< EraseMemory command CAN ID*/
#define TransferInformation            0x4E1FF01        /**< TransferInformation command CAN ID*/
#define TransferData                   0x4E2FF01        /**< TransferData command CAN ID*/
#define CRCRequest                     0x4E3FF01        /**< CRCRequest command CAN ID*/
#define SecurityAccess                 0x4E4FF01        /**< SecurityAccess command CAN ID*/
#define LogisticRequest                0x4E5FF01        /**< LogisticRequest command CAN ID*/
#define SWVersionCheck                 0x4E6FF01        /**< SWVersionCheck command CAN ID*/

#ifdef DCDC
#define ModeRequest                    0x4D3FF01   /* Receivec CAN ID for mode request for DCDC*/
#else
//#define ModeRequest                    0x4D0FF01        /**< Received CAN ID for OBC mode request*/
#define ModeRequest                    0x4D0A001        /**< Received CAN ID for OBC mode request*/
#endif

/** CAN mail box ID */
#define ID_RX_OBJ_START                1
#define ID_RX_ModeRequest              1
#define ID_RX_LogisticRequest          2
#define ID_RX_SecurityAccess           3
#define ID_RX_EraseMemory              4
#define ID_RX_TransferInformation      5
#define ID_RX_TransferData             6
#define ID_RX_CRCRequest               7
#define ID_RX_SWVersionCheck           8
#define ID_RX_OBJ_END                  8

#define ID_TX_OBJ_START                16
#define ID_TX_GENERAL_RESP             16
#define ID_TX_LOGI_RESP                17
#define ID_TX_DIAG_SESSION             18
#define ID_TX_OBJ_END                  18

/* Mode definition*/
#define DEFAULT_MODE                   2            /**< Used for goto default mode*/
#define BOOT_MODE                      7            /**< Used for goto Boot mode*/
#define DIAGNOSTICSESSION              3            /**< Used for goto Diagnostic session*/
#define BOTH_BOOT_STATE                3U           /**< 0b11*/
#define ONLY_SEC_BOOT_STATE            1U           /**< 0b01*/

/** Enum used to Indicate subsystem state of Boot. */
typedef enum {
    CMD_ModeRequest,
    CMD_LogisticRequest,
    CMD_SWVersionCheck,
    CMD_SecurityAccess,
    CMD_EraseMemory,
    CMD_TransferInformation,
    CMD_TransferData,
    CMD_CRCRequest
} CAN_CMD;


extern tCANMsgObject g_stRXCANMessage;              /**< Global CAN message buffer used in interrupt.c to receive temporal message */

/** Initiate CAN A. Include GPIO, clock, CAN INT, CAN mail box configuration.
 * Be called in MainBoot.
*
* @see MainBoot()
*********************************************************/
void InitCana(void);

/** Send Diagnostic Response from CANa.
*  Calls:          Main() state machine  @see main()
*  @param[in] MemoryArea
*            0x20: OBC Application,
*            0x21: OBC HW Version,
*            0x22: HW_SERIAL_NUMBER,
*            0x24: OBC Bootloader.
*  @param[in] Config MODE code.
*************************************************/
void SendDiagnosticResponse(uint8_t MemoryArea, uint8_t Config);

/** Send Generic Response from CANa.
*  Calls:          Main() state machine @see main()
*  @param[in] MemoryArea
*            0x20: OBC Application,
*            0x21: OBC HW Version,
*            0x22: HW_SERIAL_NUMBER,
*            0x24: OBC Bootloader.
*  @param[in] error error code.
*************************************************/
void SendGenericResponse(uint8_t MemoryArea, uint8_t error);

/** Send Logistic Response from CANa.
*  Calls:          Main() state machine @see main()
*  @param[in] MemoryArea
*            0x20: OBC Application,
*            0x21: OBC HW Version,
*            0x22: HW_SERIAL_NUMBER,
*            0x24: OBC Bootloader.
*  @param[in] Config configuration data buffer head point.
*  @param[in] DataSize configuration data buffer size.
*************************************************/
void SendLogisticResponse(uint8_t MemoryArea, uint8_t* Config, uint8_t DataSize);

#endif /* USERINC_CANCOM_H_ */
