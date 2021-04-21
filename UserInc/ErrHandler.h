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
#include "Interrupt.h"
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

#define HW_VERSION_ADDRESS              0x0C6004UL    /* Address of Hardware Version*/
#define HW_VERSION_CRC                  0x0C6008UL    /* Address of Hardware Version CRC*/
#define HW_SERIAL_NUMBER_ADDRESS        0x0C600CUL    /* Address of Hardware Serial Number*/
#define HW_SERIAL_NUMBER_CRC            0x0C6010UL    /* Address of Hardware Serial Number CRC*/
#define HW_VERSION_SIZE                 2           /* Size in bytes of Hardware version*/
#define HW_SERIAL_NUMBER_SIZE           7           /* Size in bytes of Hardware Serial Number*/

#define APP_VERSION_ADDRESS             0x0B7FF8UL    /* Address of Application sw Version*/
#define APP_CRC_ADDRESS                 0x0B7FFCUL    /* Address of Application CRC*/
#define MAX_SN                          0x25u       /* max sequence number, range: 0x01~0x25*/
#define MAX_BLOCK_SIZE                  256u
#define CRC_LENGTH                      2u          /* CRC Length in Bytes*/

#define BOOT_TOTAL_LEN                  0x5FF8U     /* Boot sector Length in 16bits, include Boot PN*/
#define APP_TOTAL_LEN                   0x2FFFCU    /* App sector Length in 16bits, include APP PN */

#define FLAG_TOTAL_LEN                  17U
#define FLAG_APPLI_ADDRESS              0x000C6000UL  /* Address of application valid Flag*/
#define MEM_APPCODE_START_ADDRESS       0x88000UL
#define MEM_APPCODE_END_ADDRESS         0xB7FF8UL

#define BOOT_EVEN_VALID                 0xBA5EBA11UL
#define BOOT_ODD_VALID                  0xC0DEBA5EUL
#define APP_VALID                       0xA5C6BD72UL      /* Value of Flag when application is valid*/

#define MEM_PREBOOT_START_ADDRESS       0x80000

/* used flash sector bit field presentation*/
#define BOOT0_SECTOR                    0x000E      /* Sector B|C|D, 0b1110 */
#define BOOT0_FLAG_SECTOR               0x0008      /* Sector D, 0b1000 */
#define BOOT1_SECTOR                    0x1C000    /* Sector O|P|Q, 0b1 1100 0000 0000 0000*/
#define BOOT1_FLAG_SECTOR               0x10000    /* Sector Q, 0b1  0000 0000 0000 0000 */
#define APP0_SECTOR                     0x03F0      /* Sector E|F|G|H|I|J, 0b0011 1111 0000*/

#define FLASH_BYTES_PER_WORD            2U


#define EXIT_FUNC_ADDR                  0x81000UL     /* Restore Exit_Boot, absolute address*/
typedef void (*pExitBoot)(uint32_t);
extern pExitBoot exitboot;
//extern void ExitBoot(uint32 EntryAddr);
/* GOTO start address of applicative area*/
#define StartApplication()              exitboot(MEM_APPCODE_START_ADDRESS)
/* GOTO start address of PreBootloader area*/
#define RESET()                         exitboot(MEM_PREBOOT_START_ADDRESS)
/* GOTO start address of Bootloader0 area*/
#define StartBootEven()                 exitboot(MEM_BOOT0_START_ADDRESS)
/* GOTO start address of Bootloader1 area*/
#define StartBootOdd()                  exitboot(MEM_BOOT1_START_ADDRESS)


#define Read_Data_Word(Addr)            *(uint16_t *)(Addr)

typedef struct
{
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
    uint8_t     u8SN;                 /* Received TransDataInfo Flag */
    uint8_t    *pu8RecvData;           /* data buffer used to receive can flash data and then be copied to flash buffer */
    uint16_t    u16RecvDataIdx;        /* data buffer(DataToCopy) index, unit: byte   */
}St_TransData;

typedef struct
{
    bool            bValidInfo;         /* Received TransDataInfo Flag */
    uint8_t         u8MemArea;          /* memory area*/
    uint8_t         u8BSC;              /* block sequence counter*/
    uint32_t        u32Address;         /* block Memory Address  */
    uint16_t        u16Size;            /* block Memory Size,   unit: byte, not include CRC part len */
    St_TransData   *pst_Data;        /* St_TransData pointer, used to restore received data */
}St_TransDataInfo;

typedef struct
{
    bool            bSecurityUnlocked;      /* Security Unlocked Flag, handled in CMD_SecurityAccess */
    bool            bAppMemoryErase;        /* Application Memory Erase Flag, updated in CMD_EraseMemory and TreatData() */
    bool            bLogMemoryErase;        /* Logistic Memory Erase Flag, updated in CMD_EraseMemory and TreatData() */
    bool            bFlashAuthorization;      /* Flash Authorization Flag,  updated in State_BOOT state overtime handler and DEFAULT_MODE request handler*/
}St_BootFlag, *pSt_BootFlag;

uint8_t IsRequestValid(stCanMsgObj Received_Message);

uint8_t IsLogisticValid(stCanMsgObj Received_Message);

uint8_t IsSWVersionCheckValid(stCanMsgObj Received_Message);

uint8_t IsSecurityValid(stCanMsgObj Received_Message);

uint8_t IsEraseValid(stCanMsgObj Received_Message, bool ucSecurityUnlocked);

uint8_t IsTransferInfoValid(stCanMsgObj Received_Message, St_TransDataInfo *pSt_TransDataInfo, St_BootFlag *stBootFlag);

uint8_t IsTransferDataValid(stCanMsgObj Received_Message, St_TransDataInfo *st_TransDataInfo);

uint8_t IsCRCRequestValid(stCanMsgObj Received_Message);

void LogiticRequestHandle(uint8_t Identifier);

void SWVersionComparetHandle(stCanMsgObj Received_Message, MyBootSys *Info, pSt_BootFlag ptr_st_BootFlag);

bool CheckWritingAddress(uint32_t Address, uint8_t MemoryArea, MyBootSys *Info);

void CRCWrite(stCanMsgObj ReceivedMessage, MyBootSys *BootStatus, St_BootFlag *stBootFlag);

void LogisticCRCWrite(stCanMsgObj ReceivedMessage);

#endif /* USERINC_ERRHANDLER_H_ */
