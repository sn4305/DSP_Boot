/**
 * @file ErrHandler.c
 *
 *  Created on: 20210302
 *      Author: E9981231
 */

#include "ErrHandler.h"
#include "CRC.h"
#include "scicom.h"

extern const uint16_t g_u40BootVersion[3];
extern uint32_t g_u32UpdataFlag;

extern uint16_t WriteFlash(uint32_t Address, uint16_t* Data, uint16_t len);
extern uint16_t SwitchBank(uint16_t BankIdx);
extern int prv_Sector_Erase(uint32_t sectors);

/** exitboot function pointer point the fix address 0x81000 which is defined in .cmd file*/
pExitBoot exitboot = (pExitBoot)EXIT_FUNC_ADDR;

/**
* Get address and size from CMD_TransferInformation. Be called in IsTransferInfoValid().
*
* @param Received_Message pointer of received message.
* @param pSt_TransDataInfo pointer of transfer data information struct.
*********************************************************/
static void GetInformation(volatile uint8_t* Received_Message, St_TransDataInfo *pSt_TransDataInfo)
{
    /* Start of data write : reception of address and length */
    pSt_TransDataInfo->u32Address = ((uint32_t) Received_Message[2] << 16) +
            ((uint32_t) Received_Message[3] << 8) +
            (uint32_t) (Received_Message[4]);

    pSt_TransDataInfo->u16Size = ((uint16_t) Received_Message[5] << 8) + (uint16_t) Received_Message[6];
}

uint8_t IsRequestValid(stCanMsgObj Received_Message)
{
    uint8_t error = NO_ERROR;
    if(Received_Message.u16MsgLen != 8)
    {
        error = WRONG_REQUEST_FORMAT;
    }
    return error;
}

uint8_t IsLogisticValid(stCanMsgObj Received_Message)
{
    uint8_t error = NO_ERROR;
    uint8_t data0 = *(Received_Message.pu8MsgData);
    if(Received_Message.u16MsgLen != 1)
    {
        error = WRONG_REQUEST_FORMAT;
    }
    else if(data0 != 0x21 && data0 != 0x22 &&
            data0 != 0x23 && data0 != 0x24 &&
            data0 != 0x33 && data0 != 0x34)
    {
        error = ID_NOT_SUPPORTED;
    }
    return error;
}

uint8_t IsSWVersionCheckValid(stCanMsgObj Received_Message)
{
    uint8_t error = NO_ERROR;
    uint8_t data0 = *(Received_Message.pu8MsgData);
    if(Received_Message.u16MsgLen != 6)
    {
        error = WRONG_REQUEST_FORMAT;
    }
    else if(data0 != 0x23 && data0 != 0x33 &&
            data0 != 0x24 && data0 != 0x34)
    {
        error = ID_NOT_SUPPORTED;
    }
    return error;
}
uint8_t IsSecurityValid(stCanMsgObj Received_Message)
{
    uint8_t error = NO_ERROR;
    if (Received_Message.u16MsgLen != 8)
    {
        error = WRONG_REQUEST_FORMAT;
    }
    else if(*Received_Message.pu8MsgData != 0x20 && *Received_Message.pu8MsgData != 0x30)
    {
        error = ID_NOT_SUPPORTED;
    }
    else
    {
        if(*Received_Message.pu8MsgData | 0xF0 == 0x20)
        {/** unsecure secondary DSP*/
            bool IsKeyValid = (*(Received_Message.pu8MsgData+1) == 0x4D);
            IsKeyValid &= (*(Received_Message.pu8MsgData+2) == 0x41);
            IsKeyValid &= (*(Received_Message.pu8MsgData+3) == 0x52);
            IsKeyValid &= (*(Received_Message.pu8MsgData+4) == 0x54);
            IsKeyValid &= (*(Received_Message.pu8MsgData+5) == 0x45);
            IsKeyValid &= (*(Received_Message.pu8MsgData+6) == 0x4B);
            IsKeyValid &= (*(Received_Message.pu8MsgData+7) == 0x30);
            if (IsKeyValid)
            {
                error = NO_ERROR;
            }
            else
            {
                error = WRONG_KEY;
            }
        }
        else
        {/** unsecure primary DSP*/
            ;
        }
    }
    return error;
}

uint8_t IsEraseValid(stCanMsgObj Received_Message, bool bSecurityUnlocked)
{
    uint8_t error = NO_ERROR;
    uint8_t data0 = *(Received_Message.pu8MsgData);
    if (Received_Message.u16MsgLen != 1)
    {
        error = WRONG_REQUEST_FORMAT;
    }
    else if(data0 != 0x20 && data0 != 0x21 &&
            data0 != 0x22 && data0 != 0x30 &&
            data0 != 0x24 && data0 != 0x34)
    {
        error = ID_NOT_SUPPORTED;
    }
    else if((data0 & 0xf0) == 0x20 && false == bSecurityUnlocked)
    {
        error = SECURITY_LOCKED;
    }
    else {
        error = NO_ERROR;
    }
    return error;
}

uint8_t IsTransferInfoValid(stCanMsgObj Received_Message, St_TransDataInfo *pSt_TransDataInfo, St_BootFlag *stBootFlag)
{
    uint8_t error = NO_ERROR;
    uint8_t data0 = *(Received_Message.pu8MsgData);

    if(NULL == pSt_TransDataInfo || NULL == stBootFlag)
    {
        return error;
    }
    GetInformation(Received_Message.pu8MsgData, pSt_TransDataInfo);
    if (Received_Message.u16MsgLen != 8)
    {
        error = WRONG_REQUEST_FORMAT;
    }
    else if(stBootFlag->bSecurityUnlocked != true &&
            (data0 | 0xf0 == 0x20))
    {
        error = SECURITY_LOCKED;
    }
    else if(data0 != 0x20 && data0 != 0x21 &&
            data0 != 0x22 && data0 != 0x30 &&
            data0 != 0x24 && data0 != 0x34)
    {
        error = ID_NOT_SUPPORTED;
    }
    else if( ((data0  == 0x20) || (data0 == 0x24)) && (stBootFlag->bAppMemoryErase  != true) )
    {
        error = MEMORY_NOT_BLANK;
    }
    else if( ((data0  == 0x21) || (data0 == 0x22)) && stBootFlag->bLogMemoryErase != true)
    {
        error = MEMORY_NOT_BLANK;
    }
    else if((pSt_TransDataInfo->u8MemArea & 0x0F) == 1 && pSt_TransDataInfo->u16Size != HW_VERSION_SIZE)
    {
        /* Transfer Data for HW version is wrong*/
        error = WRONG_REQUEST_FORMAT;
    }
    else if((pSt_TransDataInfo->u8MemArea & 0x0F) == 2 && pSt_TransDataInfo->u16Size != HW_SERIAL_NUMBER_SIZE)
    {
        /* Transfer Data for HW Serial Number is wrong*/
        error = WRONG_REQUEST_FORMAT;
    }
    else if(pSt_TransDataInfo->u16Size > MAX_BLOCK_SIZE)
    {
        error = WRONG_REQUEST_FORMAT;
    }
    else
    {
        error = NO_ERROR;
    }
    return error;
}

uint8_t IsTransferDataValid(stCanMsgObj Received_Message, St_TransDataInfo *pSt_TransDataInfo)
{
    uint8_t error = NO_ERROR;
    uint8_t data0 = *(Received_Message.pu8MsgData);

    if(TMR2_SoftwareCounterGet() >= 5)
    {
        /*50ms Timeout*/
        error = TIMEOUT;
        TMR2_SoftwareCounterClear();
        return error;
    }

    if(NULL == pSt_TransDataInfo)
    {
        return error;
    }

    if(!(pSt_TransDataInfo->bValidInfo))
    {
        error = WRONG_REQUEST_FORMAT;
    }
    else if(data0 == (pSt_TransDataInfo->pst_Data->u8SN))
    {
        /* Same sequence number as previous frame: ignore the frame*/
        error = SAME_SN;
    }
    else if(data0 != (pSt_TransDataInfo->pst_Data->u8SN + 1) || data0 > MAX_SN)
    {
        error = WRONG_REQUEST_FORMAT;
    }
    else if(((pSt_TransDataInfo->u8MemArea & 0x0F) == 0 || (pSt_TransDataInfo->u8MemArea & 0x0F) == 4) && Received_Message.u16MsgLen != 8)
    {
        if((pSt_TransDataInfo->pst_Data->u16RecvDataIdx + Received_Message.u16MsgLen - CRC_LENGTH - 1) == pSt_TransDataInfo->u16Size)
        {
            error = NO_ERROR;
        }
        else
        {
            error = WRONG_REQUEST_FORMAT;
        }
    }
    else
    {
        error = NO_ERROR;
    }
    return error;
}

uint8_t IsCRCRequestValid(stCanMsgObj Received_Message)
{
    uint8_t error = NO_ERROR;
    uint8_t data0 = *(Received_Message.pu8MsgData);
    if(Received_Message.u16MsgLen != 3)
    {
        error = WRONG_REQUEST_FORMAT;
    }
    else if(data0 != 0x20 && data0 != 0x21 &&
            data0 != 0x22 && data0 != 0x30 &&
            data0 != 0x24 && data0 != 0x34)
    {
        error = ID_NOT_SUPPORTED;
    }
    else
    {
        error = NO_ERROR;
    }
    return error;
}

void LogiticRequestHandle(uint8_t Identifier)
{
    switch(Identifier)
    {
        case SECD_MEMORY_AREA + 1:
        {
            uint16_t CRCCalc;
            /*Version Number*/
            uint16_t *pHwVer = (uint16_t *)HW_VERSION_ADDRESS;
            uint8_t Data[2];
            Data[0] = (uint8_t)(*pHwVer >> 8);
            Data[1] = (uint8_t)(*pHwVer);
            CRCCalc = CRC16(0, (uint16_t *)HW_VERSION_ADDRESS, HW_VERSION_SIZE);
            /*Add received CRC to calculated CRC*/
            CRCCalc = CRC16(CRCCalc, (uint16_t *)HW_VERSION_CRC, CRC_LENGTH);
            if(CRCCalc == 0xF0B8)
            {
                /*Send*/
                SendLogisticResponse(Identifier, Data, HW_VERSION_SIZE);
            }
            else
            {
                /*Wrong CRC*/
                SendGenericResponse(Identifier, WRONG_CRC);
            }
            break;
        }

        case SECD_MEMORY_AREA + 2:
        {
            uint16_t CRCCalc;
            /* Serial Number*/
            uint16_t *pHwSer = (uint16_t *)HW_SERIAL_NUMBER_ADDRESS;
            uint8_t Data[7];
            Data[0] = (uint8_t)(*pHwSer >> 8);
            Data[1] = (uint8_t)(*pHwSer++);
            Data[2] = (uint8_t)(*pHwSer >> 8);
            Data[3] = (uint8_t)(*pHwSer++);
            Data[4] = (uint8_t)(*pHwSer >> 8);
            Data[5] = (uint8_t)(*pHwSer++);
            Data[6] = (uint8_t)(*pHwSer >> 8);
            CRCCalc = CRC16(0, (uint16_t *)HW_SERIAL_NUMBER_ADDRESS, HW_SERIAL_NUMBER_SIZE);
            /*Add received CRC to calculated CRC*/
            CRCCalc = CRC16(CRCCalc, (uint16_t *)HW_SERIAL_NUMBER_CRC, CRC_LENGTH);
            if(CRCCalc == 0xF0B8)
            {
                /*CRC Correct*/
                SendLogisticResponse(Identifier, Data, HW_SERIAL_NUMBER_SIZE);
            }
            else
            {
                /*Wrong CRC*/
                SendGenericResponse(Identifier, WRONG_CRC);
            }
            break;
        }

        case SECD_MEMORY_AREA + 3:
        {
            /*Read PN Number*/
            uint8_t Data[5];
            /*Reconstruct PN number
             * 0xAABBCCDDEE is sent 0xAA, 0xBB...*/
            Data[0] = g_u40BootVersion[0] >> 8;
            Data[1] = g_u40BootVersion[0];
            Data[2] = g_u40BootVersion[1] >> 8;
            Data[3] = g_u40BootVersion[1];
            Data[4] = g_u40BootVersion[2] >> 8;
            /*Send*/
            SendLogisticResponse(Identifier, Data, 5);
            break;
        }

        case SECD_MEMORY_AREA + 4:
        {
            /*Read Application PN Number*/

            uint16_t *pAppVer = (uint16_t *)APP_VERSION_ADDRESS;
            uint8_t Data[5];
            Data[0] = (uint8_t)(*pAppVer >> 8);
            Data[1] = (uint8_t)(*pAppVer++);
            Data[2] = (uint8_t)(*pAppVer >> 8);
            Data[3] = (uint8_t)(*pAppVer++);
            Data[4] = (uint8_t)(*pAppVer >> 8);
            /* Send PN*/
            SendLogisticResponse(Identifier, Data, 5);
            break;
        }

        case SECD_MEMORY_AREA + 5:
            /* OBC Primary SW PN, Not used in Bootloader*/
        default:
            /* Do Nothing
             * Error is handled before*/
            break;
    }
}

void SWVersionComparetHandle(stCanMsgObj Received_Message, MyBootSys *Info, pSt_BootFlag ptr_st_BootFlag)
{
    uint16_t *pVer;
    uint32_t PN_Addr;
    uint8_t i, DiffErr, RespMemArea;
    uint8_t ActualVersion[5];
    uint8_t *data = (uint8_t *)(Received_Message.pu8MsgData);

    RespMemArea = *data & 0xF0;
    switch(RespMemArea)
    {
        case SECD_MEMORY_AREA:
            if((*data & 0x0F) == 4)
            {
                PN_Addr = Info->BootPNAddr;
                RespMemArea = *data;
            }
            else if((*data & 0x0F) == 3)
            {
                PN_Addr = APP_VERSION_ADDRESS;
                RespMemArea = *data;
            }

            DiffErr = 0;
            pVer = (uint16_t *)PN_Addr;

            ActualVersion[0] = (uint8_t)(*pVer >> 8) & 0xFF;
            ActualVersion[1] = (uint8_t)(*pVer++) & 0xFF;
            ActualVersion[2] = (uint8_t)(*pVer >> 8) & 0xFF;
            ActualVersion[3] = (uint8_t)(*pVer++) & 0xFF;
            ActualVersion[4] = (uint8_t)(*pVer >> 8) & 0xFF;

            data++;
            for(i = 0; i < 5; i++)
            {
                if (ActualVersion[i] != *data++)
                {
                    DiffErr ++;
                }
            }
            if(DiffErr)
            {
                ptr_st_BootFlag->bFlashAuthorization = true;
            }
            else
            {
                ptr_st_BootFlag->bFlashAuthorization = false;
            }
            SendLogisticResponse(RespMemArea, ActualVersion, 5);
            break;

        case PRIM_MEMORY_AREA:
            SCI_Send_Cmd(SCI_SWVersionCheck, data, 6);
            break;

        default:
            break;
    }

}

bool CheckWritingAddress(uint32_t Address, uint8_t MemoryArea, MyBootSys *Info)
{
    bool ReturnValue = false;

    if ((MemoryArea & 0x0F) == 0)
    {
        /* Writing app */
        if (Address < MEM_APPCODE_START_ADDRESS || Address > MEM_APPCODE_END_ADDRESS)
        {
            /* Writing in Bootloader area*/
            ReturnValue = false;
            SendGenericResponse(SECD_MEMORY_AREA, WRITING_INVALID);
        }
        else
        {
            /* give flash access, app0 exsit in BANK0*/
            if(0 != SwitchBank(0))
            {
                ReturnValue = false;
            }
            else
            {
                ReturnValue = true;
            }
        }
    }
    else if ((MemoryArea & 0x0F) == 4)
    {
        if (Address < Info->OppositBootStartAddr || Address > Info->OppositBootEndAddr)
        {
            ReturnValue = false;
            SendGenericResponse(SECD_MEMORY_AREA, WRITING_INVALID);
        }
        else
        {
            if( 0 == Info->BootPolarity)
            {
                /*current boot is EVEN in bank0, need switch bank to 1 to write data*/
                if(0 != SwitchBank(1))
                {
                    ReturnValue = false;
                }
                else
                {
                    ReturnValue = true;
                }
            }
            else if( 1 == Info->BootPolarity)
            {
                /*current boot is ODD in bank1, need switch bank to 0 to write data*/
                if(0 != SwitchBank(0))
                {
                    ReturnValue = false;
                }
                else
                {
                    ReturnValue = true;
                }
            }
        }
    }
    else
    {
        /* Writing logistic information*/
        if (Address < FLAG_APPLI_ADDRESS || Address >= (FLAG_APPLI_ADDRESS + FLAG_TOTAL_LEN))
        {
            /*Writing before or after last page*/
            ReturnValue = false;
            SendGenericResponse(SECD_MEMORY_AREA, WRITING_INVALID);
        }
        else
        {
            /*Good area*/
            ReturnValue = true;
        }
    }
    EDIS;

    return ReturnValue;
}

#pragma CODE_SECTION(CRCWrite,".TI.ramfunc");
void CRCWrite(stCanMsgObj ReceivedMessage, MyBootSys *BootStatus, St_BootFlag *stBootFlag)
{
    uint16_t WriteBuf[2];
    uint16_t ReceivedCRC;
    uint16_t CRCFlash;
    uint32_t EraseSector;

    g_u32UpdataFlag = 0;
    ReceivedCRC = ((uint16_t) ReceivedMessage.pu8MsgData[1] << 8) + ReceivedMessage.pu8MsgData[2];

    if((ReceivedMessage.pu8MsgData[0] & 0x0F) == 4)
    {
        /*Calculate CRC for boot memory area*/
        CRCFlash = CalcCRC_FLASH(0, BootStatus->OppositBootStartAddr, BOOT_TOTAL_LEN);
    }
    else
    {
        /*Calculate the CRC for app memory area*/
        CRCFlash = CalcCRC_FLASH(0, MEM_APPCODE_START_ADDRESS, APP_TOTAL_LEN);
    }

    if(CRCFlash == ReceivedCRC)
    {
        ServiceDog();
        /*Send OK response */
        SendGenericResponse(SECD_MEMORY_AREA, NO_ERROR);

        if((ReceivedMessage.pu8MsgData[0] & 0x0F) == 4)
        {
            if(0 == BootStatus->BootPolarity)
            {
                SwitchBank(1);
            }
            else
            {
                SwitchBank(0);
            }
            WriteBuf[0] = (uint16_t) BootStatus->OppositBootValidCode;
            WriteBuf[1] = (uint16_t) (BootStatus->OppositBootValidCode >> 16);
            WriteFlash(BootStatus->OppositBootFlagValidAddr, WriteBuf, 2);
            ReleaseFlashPump();
            ServiceDog();
            if(0 == BootStatus->BootPolarity)
            {
                EraseSector = BOOT0_FLAG_SECTOR;
                SwitchBank(0);

            }
            else
            {
                EraseSector = BOOT1_FLAG_SECTOR;
                SwitchBank(1);
            }
            /*Erase Current boot Flash */
            prv_Sector_Erase(EraseSector);
        }
        else
        {
            SwitchBank(1);
            WriteBuf[0] = (uint16_t) APP_VALID;
            WriteBuf[1] = (uint16_t) (APP_VALID >> 16);
            WriteFlash(FLAG_APPLI_ADDRESS, WriteBuf, 2);
            ServiceDog();
        }
        stBootFlag->bAppMemoryErase = false;
        DEADLOOP();     /**< use dead loop trigger watch dog reset*/
    }
    else
    {
        /*Wrong CRC*/
        SendGenericResponse(SECD_MEMORY_AREA, WRONG_CRC);
    }

}

void LogisticCRCWrite(stCanMsgObj ReceivedMessage)
{
    uint16_t ReceivedCRC, HardwareVersionRead, HardwareSNRead[4], DataForCRC[5], CRCCalc;

    if((ReceivedMessage.pu8MsgData[0] & 0x0F) == 1)
    {
        /* Hardware version case */
        /*Save received CRC*/
        ReceivedCRC = (ReceivedMessage.pu8MsgData[1] << 8) + ReceivedMessage.pu8MsgData[2];
        /*Read Saved data*/
        HardwareVersionRead = (uint16_t) Read_Data_Word(HW_VERSION_ADDRESS);
        /*Calculate CRC including received CRC*/
        DataForCRC[0] = HardwareVersionRead;
        DataForCRC[1] = 0xFFFF;
        DataForCRC[2] = 0xFFFF;
        DataForCRC[3] = 0xFFFF;
        DataForCRC[4] = ReceivedCRC;

        CRCCalc = CRC16(0, DataForCRC, HW_VERSION_SIZE);
        /*Add received CRC to calculated CRC*/
        CRCCalc = CRC16(CRCCalc, &ReceivedCRC, CRC_LENGTH);
        if(CRCCalc == 0xF0B8)
        {
            /*Correct CRC : save*/
            SwitchBank(1);
            WriteFlash(HW_VERSION_ADDRESS, DataForCRC, 5);
            ReleaseFlashPump();
            SendGenericResponse(SECD_MEMORY_AREA, NO_ERROR);
        }
        else
        {
            /*Wrong CRC*/
            SendGenericResponse(SECD_MEMORY_AREA, WRONG_CRC);
        }
    }
    else if((ReceivedMessage.pu8MsgData[0] & 0x0F) == 2)
    {
        /*Serial Number case*/
        /* Save in RAM received CRC*/
        ReceivedCRC = (ReceivedMessage.pu8MsgData[1] << 8) + ReceivedMessage.pu8MsgData[2];

        /*Read Saved Serial Number*/
        HardwareSNRead[0] = Read_Data_Word(HW_SERIAL_NUMBER_ADDRESS);
        HardwareSNRead[1] = Read_Data_Word(HW_SERIAL_NUMBER_ADDRESS + 1);
        HardwareSNRead[2] = Read_Data_Word(HW_SERIAL_NUMBER_ADDRESS + 2);
        HardwareSNRead[3] = Read_Data_Word(HW_SERIAL_NUMBER_ADDRESS + 3);

        /*Calculate CRC on data + received CRC*/
        DataForCRC[0] = HardwareSNRead[0];
        DataForCRC[1] = HardwareSNRead[1];
        DataForCRC[2] = HardwareSNRead[2];
        DataForCRC[3] = HardwareSNRead[3];
        DataForCRC[4] = ReceivedCRC;

        /*CRC on data*/
        CRCCalc = CRC16(0, DataForCRC, HW_SERIAL_NUMBER_SIZE);
        /*Add received CRC to calculated CRC*/
        CRCCalc = CRC16(CRCCalc, &ReceivedCRC, CRC_LENGTH);

        if(CRCCalc == 0xF0B8)
        {
            /* Correct CRC : save CRC*/
            SeizeFlashPump_Bank1();
            WriteFlash(HW_SERIAL_NUMBER_ADDRESS, DataForCRC, 5);
            SendGenericResponse(SECD_MEMORY_AREA, NO_ERROR);
        }
        else
        {
            /*Wrong CRC*/
            SendGenericResponse(SECD_MEMORY_AREA, WRONG_CRC);
        }
    }
}

