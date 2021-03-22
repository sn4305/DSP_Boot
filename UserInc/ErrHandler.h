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
//#include "CRC.h"
//#include "Flash.h"
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
#define SAME_SN                     0x14

#ifdef DCDC
#define MEMORY_AREA                     0x40        /* Value of Memory Area for DCDC*/
#else
#define MEMORY_AREA                     0x20        /* Value of Memory Area for OBC*/
#endif

#define HW_VERSION_ADDRESS              0x0C6004    /* Address of Hardware Version*/
#define HW_VERSION_CRC                  0x0C6008    /* Address of Hardware Version CRC*/
#define HW_SERIAL_NUMBER_ADDRESS        0x0C600C    /* Address of Hardware Serial Number*/
#define HW_SERIAL_NUMBER_CRC            0x0C6010    /* Address of Hardware Serial Number CRC*/
#define HW_VERSION_SIZE                 2           /* Size in bytes of Hardware version*/
#define HW_SERIAL_NUMBER_SIZE           7           /* Size in bytes of Hardware Serial Number*/

#define APP_VERSION_ADDRESS             0x0B7FF8    /* Address of Application sw Version*/
#define APP_CRC_ADDRESS                 0x0B7FFC    /* Address of Application CRC*/
#define MAX_SN                          0x25u       /* max sequence number, range: 0x01~0x25*/
#define MAX_BLOCK_SIZE                  256u
#define CRC_LENGTH                      2u          /* CRC Length in Bytes*/

#define BOOT_TOTAL_LEN                  0x5FF4u     /* Boot sector Length in 16bits*/
#define APP_TOTAL_LEN                   0x2FFFCul   /* App sector Length in 16bits */

#define FLAG_TOTAL_LEN                  17u
#define FLAG_APPLI_ADDRESS              0x000C6000  /* Address of application valid Flag*/
#define MEM_APPCODE_START_ADDRESS       0x88000
#define MEM_APPCODE_END_ADDRESS         0xB7FF8

#define BootEvenValid                   0xBA5EBA11
#define BootOddValid                    0xC0DEBA5E
#define APP_VALID                       0xA5C6BD72      /* Value of Flag when application is valid*/

#define MEM_PREBOOT_START_ADDRESS       0x80000

#define FLASH_BYTES_PER_WORD            2u

extern void ExitBoot(uint32 EntryAddr);
/* GOTO start address of applicative area*/
#define StartApplication()              ExitBoot(MEM_APPCODE_START_ADDRESS)
/* GOTO start address of PreBootloader area*/
#define RESET()                         ExitBoot(MEM_PREBOOT_START_ADDRESS)
/* GOTO start address of Bootloader0 area*/
#define StartBootEven()                 ExitBoot(MEM_BOOT0_START_ADDRESS)
/* GOTO start address of Bootloader1 area*/
#define StartBootOdd()                  ExitBoot(MEM_BOOT1_START_ADDRESS)

#define Read_Data_Word(Addr)            *(uint16_t *)(Addr)

typedef struct {
    uint8_t BootPolarity;
    uint32_t BootPNAddr;
    uint32_t BootValidFlagAddr;
    uint32_t OppositBootStartAddr;
    uint32_t OppositBootCRCAddr;
    uint32_t OppositBootFlagValidAddr;
    uint32_t OppositBootEndAddr;
    uint32_t OppositBootValidCode;
}MyBootSys;

typedef struct
{
    uint8_t     SN;                 /* Received TransDataInfo Flag */
    uint8_t    *pRecvData;           /* data buffer used to receive can flash data and then be copied to flash buffer */
    uint16_t    RecvDataIdx;        /* data buffer(DataToCopy) index, unit: byte   */
/*   uint16_t   *pFlashData;           data buffer used to receive can flash data and then be copied to flash buffer */
}St_TransData;

typedef struct
{
    uint8_t         ValidInfo;      /* Received TransDataInfo Flag */
    uint8_t         MemArea;        /* memory area*/
    uint8_t         BSC;            /* block sequence counter*/
    uint32_t        Address;        /* block Memory Address  */
    uint16_t        Size;           /* block Memory Size,   unit: byte, not include CRC part len */
    St_TransData   *ptr_St_Data;    /* St_TransData pointer, used to restore received data */
}St_TransDataInfo;

uint8_t IsRequestValid(tCANMsgObject Received_Message);

uint8_t IsLogisticValid(tCANMsgObject Received_Message);

uint8_t IsSWVersionCheckValid(tCANMsgObject Received_Message);

uint8_t IsSecurityValid(tCANMsgObject Received_Message);

uint8_t IsEraseValid(tCANMsgObject Received_Message, bool ucSecurityUnlocked);

uint8_t IsTransferInfoValid(tCANMsgObject Received_Message, St_TransDataInfo *pSt_TransDataInfo);

uint8_t IsTransferDataValid(tCANMsgObject Received_Message, St_TransDataInfo *st_TransDataInfo);

uint8_t IsCRCRequestValid(tCANMsgObject Received_Message);

void LogiticRequestHandle(uint8_t Identifier);

void SWVersionComparetHandle(tCANMsgObject Received_Message, MyBootSys Info, bool* Authorization);

bool CheckWritingAddress(uint32_t Address, uint8_t MemoryArea, MyBootSys Info);

void CRCWrite(tCANMsgObject ReceivedMessage, MyBootSys BootStatus);

void LogisticCRCWrite(tCANMsgObject ReceivedMessage);

#endif /* USERINC_ERRHANDLER_H_ */
