/*
 * can.h
 *
 *  Created on: 2021Äê2ÔÂ26ÈÕ
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

#define GENERAL_RESP      0xCEF01A0
#define LOGI_RESP         0xCEF02A0
#define DIAG_SESSION      0xCFF03A0

/* CAN Id */
#define EraseMemory                    0x4E0FF01
#define TransferInformation            0x4E1FF01
#define TransferData                   0x4E2FF01
#define CRCRequest                     0x4E3FF01
#define SecurityAccess                 0x4E4FF01
#define LogisticRequest                0x4E5FF01
#define SWVersionCheck                 0x4E6FF01

#ifdef DCDC
#define ModeRequest                    0x4D3FF01   /* Receivec CAN ID for mode request for DCDC*/
#else
#define ModeRequest                    0x4D0FF01   /* Receivec CAN ID for mode request for OBC*/
#endif

/* CAN mail box ID */
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
#define DEFAULT_MODE                   6            /* Used for goto default mode*/
#define BOOT_MODE                      5            /* Used for goto Boot mode*/
#define DIAGNOSTICSESSION              3            /* Used for goto Diagnostic session*/

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


extern unsigned char txMsgData[8];
extern unsigned char rxMsgData[8];
extern tCANMsgObject sTXCANMessage;
extern tCANMsgObject sRXCANMessage;

void InitCana(void);
void SendDiagnosticResponse(uint8_t MemoryArea, uint8_t Config);
void SendGenericResponse(uint8_t MemoryArea, uint8_t error);
void SendLogisticResponse(uint8_t MemoryArea, uint8_t* Config, uint8_t DataSize);

#endif /* USERINC_CANCOM_H_ */
