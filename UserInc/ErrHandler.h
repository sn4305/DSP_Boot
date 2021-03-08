/*
 * ErrHandler.h
 *
 *  Created on: 2021Äê3ÔÂ2ÈÕ
 *      Author: E9981231
 */

#ifndef USERINC_ERRHANDLER_H_
#define USERINC_ERRHANDLER_H_

#include "F28x_Project.h"
#include "Timer.h"
#include "cancom.h"
#include "inc/hw_can.h"
#include "driverlib/can.h"
#include "inc/hw_types.h"

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

#ifdef DCDC
#define MEMORY_AREA                     0x40    /* Value of Memory Area for DCDC*/
#else
#define MEMORY_AREA                     0x20    /* Value of Memory Area for OBC*/
#endif

#define HW_VERSION_ADDRESS              0x0C6002    /* Address of Hardware Version*/
#define HW_VERSION_CRC                  0x0C6003    /* Address of Hardware Version CRC*/
#define HW_SERIAL_NUMBER_ADDRESS        0x0C6004    /* Address of Hardware Serial Number*/
#define HW_SERIAL_NUMBER_CRC            0x0C6008    /* Address of Hardware Serial Number CRC*/
#define HW_VERSION_SIZE                 2           /* Size in bytes of Hardware version*/
#define HW_SERIAL_NUMBER_SIZE           7           /* Size in bytes of Hardware Serial Number*/

#define APP_VERSION_ADDRESS             0x0B7FFC    /* Address of Application sw Version*/
#define MAX_SN                          0x24u
#define CRC_LENGTH                      2u      /* CRC Length in Bytes*/

typedef struct {
    uint8_t BootPolarity;
    uint32_t BootPNAddr;
    uint32_t BootValidFlagAddr;
    uint32_t OppositBootStartAddr;
    uint32_t OppositBootCRCAddr;
    uint32_t OppositBootFlagValidAddr;
    uint32_t OppositBootEndAddr;
    uint32_t OppositBootValidCode;
} MyBootSys;

uint8_t IsRequestValid(tCANMsgObject Received_Message);

uint8_t IsLogisticValid(tCANMsgObject Received_Message);

uint8_t IsSWVersionCheckValid(tCANMsgObject Received_Message);

uint8_t IsSecurityValid(tCANMsgObject Received_Message);

uint8_t IsEraseValid(tCANMsgObject Received_Message, bool ucSecurityUnlocked);

uint8_t IsTransferInfoValid(tCANMsgObject Received_Message, bool ucSecurityUnlocked, bool ucAppMemoryErase, bool ucLogMemoryErase);

uint8_t IsTransferDataValid(tCANMsgObject Received_Message, bool InfoReceived, uint8_t SN, uint16_t CopiedData, uint16_t LenDataToCopy);

uint8_t IsCRCRequestValid(tCANMsgObject Received_Message);

void LogiticRequestHandle(uint8_t Identifier);

void SWVersionComparetHandle(tCANMsgObject Received_Message, MyBootSys Info, bool* Authorization);

#endif /* USERINC_ERRHANDLER_H_ */
