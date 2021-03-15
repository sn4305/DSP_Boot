/*
 * ErrHandler.c
 *
 *  Created on: 2021Äê3ÔÂ2ÈÕ
 *      Author: E9981231
 */

#include "ErrHandler.h"

extern bool ucSecurityUnlocked, ucAppMemoryErase, ucLogMemoryErase, ReceivedInfo, FlashAuthorization;
extern const uint16_t u40BootVersion[3];
extern uint32_t u32UpdataFlag;

extern uint16_t WriteFlash(uint32_t Address, uint16_t* Data, uint16_t len);
extern uint16_t CalcCRC_FLASH(uint16_t Init, uint16_t CodeStartAddr, uint16_t len_word);
extern uint16_t CRC16(uint16_t reg_init, uint8_t *data, uint16_t len);

static void GetInformation(uint8_t* Received_Message, St_TransDataInfo *pSt_TransDataInfo)
{
    /* Start of data write : reception of address and length */
    pSt_TransDataInfo->Address = ((uint32_t) Received_Message[2] << 16) +
            ((uint32_t) Received_Message[3] << 8) +
            (uint32_t) (Received_Message[4]);

    pSt_TransDataInfo->Size = ((uint16_t) Received_Message[5] << 8) + (uint16_t) Received_Message[6];
}

uint8_t IsRequestValid(tCANMsgObject Received_Message)
{
    uint8_t error = NO_ERROR;
    if(Received_Message.ui32MsgLen != 8)
    {
        error = WRONG_REQUEST_FORMAT;
    }
    return error;
}

uint8_t IsLogisticValid(tCANMsgObject Received_Message)
{
    uint8_t error = NO_ERROR;
    uint8_t data0 = *(Received_Message.pucMsgData);
    if(Received_Message.ui32MsgLen != 1)
    {
        error = WRONG_REQUEST_FORMAT;
    }
    else if(data0 != 0x21 && data0 != 0x22 &&
            data0 != 0x23 && data0 != 0x24 &&
            data0 != 0x25 && data0 != 0x41 &&
            data0 != 0x42 && data0 != 0x43 &&
            data0 != 0x44 && data0 != 0x45)
    {
        error = ID_NOT_SUPPORTED;
    }
    return error;
}

uint8_t IsSWVersionCheckValid(tCANMsgObject Received_Message)
{
    uint8_t error = NO_ERROR;
    uint8_t data0 = *(Received_Message.pucMsgData);
    if(Received_Message.ui32MsgLen != 6)
    {
        error = WRONG_REQUEST_FORMAT;
    }
    else if(data0 != 0x23 && data0 != 0x43 &&
            data0 != 0x24 && data0 != 0x44)
    {
        error = ID_NOT_SUPPORTED;
    }
    return error;
}
uint8_t IsSecurityValid(tCANMsgObject Received_Message)
{
    uint8_t error = NO_ERROR;
    if (Received_Message.ui32MsgLen != 8)
    {
        error = WRONG_REQUEST_FORMAT;
    }
    else if(*Received_Message.pucMsgData != 0x20 && *Received_Message.pucMsgData != 0x40)
    {
        error = ID_NOT_SUPPORTED;
    }
    else
    {
        bool IsKeyValid = (*(Received_Message.pucMsgData+1) == 0x4D);
        IsKeyValid &= (*(Received_Message.pucMsgData+2) == 0x41);
        IsKeyValid &= (*(Received_Message.pucMsgData+3) == 0x52);
        IsKeyValid &= (*(Received_Message.pucMsgData+4) == 0x54);
        IsKeyValid &= (*(Received_Message.pucMsgData+5) == 0x45);
        IsKeyValid &= (*(Received_Message.pucMsgData+6) == 0x4B);
        IsKeyValid &= (*(Received_Message.pucMsgData+7) == 0x30);
        if (IsKeyValid)
        {
            error = NO_ERROR;
        }
        else
        {
            error = WRONG_KEY;
        }
    }
    return error;
}

uint8_t IsEraseValid(tCANMsgObject Received_Message, bool ucSecurityUnlocked)
{
    uint8_t error = NO_ERROR;
    uint8_t data0 = *(Received_Message.pucMsgData);
    if (Received_Message.ui32MsgLen != 1)
    {
        error = WRONG_REQUEST_FORMAT;
    }
    else if(data0 != 0x20 && data0 != 0x21 &&
            data0 != 0x22 && data0 != 0x40 &&
            data0 != 0x41 && data0 != 0x42 &&
            data0 != 0x24 && data0 != 0x44)
    {
        error = ID_NOT_SUPPORTED;
    }
    else if(false == ucSecurityUnlocked)
    {
        error = SECURITY_LOCKED;
    }
    else {
        error = NO_ERROR;
    }
    return error;
}

uint8_t IsTransferInfoValid(tCANMsgObject Received_Message, uint8_t* Message_Data, St_TransDataInfo *pSt_TransDataInfo)
{
    uint8_t error = NO_ERROR;
    uint8_t data0 = *(Received_Message.pucMsgData);

    GetInformation(Message_Data, pSt_TransDataInfo);
    if (Received_Message.ui32MsgLen != 8)
    {
        error = WRONG_REQUEST_FORMAT;
    }
    else if(ucSecurityUnlocked != true)
    {
        error = SECURITY_LOCKED;
    }
    else if(data0 != 0x20 && data0 != 0x21 &&
            data0 != 0x22 && data0 != 0x40 &&
            data0 != 0x41 && data0 != 0x42 &&
            data0 != 0x24 && data0 != 0x44)
    {
        error = ID_NOT_SUPPORTED;
    }
    else if( ((data0 & 0x0F) == 0) || ((data0 & 0x0F) == 4) && (ucAppMemoryErase  != true) )
    {
        error = MEMORY_NOT_BLANK;
    }
    else if( ((data0 & 0x0F) != 0) && ((data0 & 0x0F) != 4) && ucLogMemoryErase != true)
    {
        error = MEMORY_NOT_BLANK;
    }
    else if(pSt_TransDataInfo->Size > MAX_BLOCK_SIZE)
    {
        error = WRONG_REQUEST_FORMAT;
    }
    else
    {
        error = NO_ERROR;
    }
    return error;
}

uint8_t IsTransferDataValid(tCANMsgObject Received_Message, St_TransDataInfo *st_TransDataInfo)
{
    uint8_t error = NO_ERROR;
    uint8_t data0 = *(Received_Message.pucMsgData);

    if(TMR3_SoftwareCounterGet() >= 10){ /*50ms*/
        /*Timeout*/
        error = TIMEOUT;
        TMR3_SoftwareCounterClear();
    }

    if(!(st_TransDataInfo->ValidInfo))
    {
        error = WRONG_REQUEST_FORMAT;
    }
    else if(data0 == (st_TransDataInfo->ptr_St_Data->SN - 1))
    {
        /* Same sequence number as previous frame: ignore the frame*/
        error = SAME_SN;
    }
    else if(data0 != (st_TransDataInfo->ptr_St_Data->SN + 1) || data0 > MAX_SN)
    {
        error = WRONG_REQUEST_FORMAT;
    }
    else if(Received_Message.ui32MsgLen != 8)
    {
        if((st_TransDataInfo->ptr_St_Data->RecvDataIdx + Received_Message.ui32MsgLen - CRC_LENGTH) == st_TransDataInfo->Size + 1)
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

uint8_t IsCRCRequestValid(tCANMsgObject Received_Message)
{
    uint8_t error = NO_ERROR;
    uint8_t data0 = *(Received_Message.pucMsgData);
    if(Received_Message.ui32MsgLen != 3)
    {
        error = WRONG_REQUEST_FORMAT;
    }
    else if(data0 != 0x20 && data0 != 0x21 &&
            data0 != 0x22 && data0 != 0x40 &&
            data0 != 0x41 && data0 != 0x42 &&
            data0 != 0x24 && data0 != 0x44)
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
        case MEMORY_AREA + 1:
        {
            /*Version Number*/
            uint16_t *pHwVer = (uint16_t *)HW_VERSION_ADDRESS;
            uint8_t Data[2];
            Data[0] = (uint8_t)(*pHwVer >> 8);
            Data[1] = (uint8_t)(*pHwVer);
            /*Send*/
            SendLogisticResponse(Identifier, Data, HW_VERSION_SIZE);
            break;
        }

        case MEMORY_AREA + 2:
        {
            /* Serial Number*/
#if 0
            uint32_t HWSNL = Read_Data_Word((uint32_t) HW_SERIAL_NUMBER_ADDRESS);
            uint32_t HWSNH = Read_Data_Word((uint32_t) HW_SERIAL_NUMBER_ADDRESS + 2);
            uint32_t HWSNLast = Read_Data_Word((uint32_t) HW_SERIAL_NUMBER_ADDRESS + 4);
            uint32_t HWSN_CRC = Read_Data_Word((uint32_t) HW_SERIAL_NUMBER_ADDRESS + 6);
            uint16_t CRC = 0;
            uint8_t Data[9]; /*7 bytes + 2 for CRC*/
            Data[0] = (uint8_t) (HWSNL >> 8);
            Data[1] = (uint8_t) (HWSNL);
            Data[2] = (uint8_t) (HWSNL >> 16);
            Data[3] = (uint8_t) (HWSNH >> 8);
            Data[4] = (uint8_t) (HWSNH);
            Data[5] = (uint8_t) (HWSNH >> 16);
            Data[6] = (uint8_t) (HWSNLast);
            Data[7] = (uint8_t) (HWSN_CRC >> 8);
            Data[8] = (uint8_t) (HWSN_CRC);
            int i;
            /*Calculate CRC*/
            for (i = 0; i < 8; i++){
                CRC = UpdateCrcKermit(CRC, Data[i]);
            }
            if (CRC == 0xF0B8){
                /*CRC Correct*/
                SendLogisticResponse(Identifier, Data, 7);
            }
            else {
                /*Wrong CRC*/
                SendGenericResponse(Identifier, WRONG_CRC);
            }
#endif
            uint16_t *pHwSer = (uint16_t *)HW_SERIAL_NUMBER_ADDRESS;
            uint8_t Data[7];
            Data[0] = (uint8_t)(*pHwSer >> 8);
            Data[1] = (uint8_t)(*pHwSer++);
            Data[2] = (uint8_t)(*pHwSer >> 8);
            Data[3] = (uint8_t)(*pHwSer++);
            Data[4] = (uint8_t)(*pHwSer >> 8);
            Data[5] = (uint8_t)(*pHwSer++);
            Data[6] = (uint8_t)(*pHwSer >> 8);
            /*Send*/
            SendLogisticResponse(Identifier, Data, HW_SERIAL_NUMBER_SIZE);
            break;
        }

        case MEMORY_AREA + 3:
        {
            /*Read PN Number*/
            uint8_t Data[5];
            /*Reconstruct PN number
             * 0xAABBCCDDEE is sent 0xAA, 0xBB...*/
            Data[0] = u40BootVersion[0] >> 8;
            Data[1] = u40BootVersion[0];
            Data[2] = u40BootVersion[1] >> 8;
            Data[3] = u40BootVersion[1];
            Data[4] = u40BootVersion[2] >> 8;
            /*Send*/
            SendLogisticResponse(Identifier, Data, 5);
            break;
        }

        case MEMORY_AREA + 4:
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

        case MEMORY_AREA + 5:
            /* OBC Primary SW PN, Not used in Bootloader*/
        default:
            /* Do Nothing
             * Error is handled before*/
            break;
    }
}

void SWVersionComparetHandle(tCANMsgObject Received_Message, MyBootSys Info, bool* Authorization)
{
    uint16_t *pVer;
    uint32_t PN_Addr;
    uint8_t i, DiffErr, RespMemArea;
    uint8_t ActualVersion[5];
    uint8_t *data = (uint8_t *)(Received_Message.pucMsgData);

    if((*data & 0x0F) == 4)
    {
        PN_Addr = Info.BootPNAddr;
        RespMemArea = (MEMORY_AREA|0x04);
    }
    else
    {
        PN_Addr = APP_VERSION_ADDRESS;
        RespMemArea = (MEMORY_AREA|0x03);
    }

    DiffErr = 0;
    pVer = (uint16_t *)PN_Addr;

    ActualVersion[0] = (uint8_t)(*pVer >> 8);
    ActualVersion[1] = (uint8_t)(*pVer++);
    ActualVersion[2] = (uint8_t)(*pVer >> 8);
    ActualVersion[3] = (uint8_t)(*pVer++);
    ActualVersion[4] = (uint8_t)(*pVer >> 8);

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
        *Authorization = true;
    }
    else
    {
        *Authorization = false;
    }
    SendLogisticResponse(RespMemArea, ActualVersion, 5);
}

bool CheckWritingAddress(uint32_t Address, uint8_t MemoryArea, MyBootSys Info)
{
    bool ReturnValue = false;

    if ((MemoryArea & 0x0F) == 0)
    {
        /* Writing app */
        if (Address < MEM_APPCODE_START_ADDRESS)
        {
            /* Writing in Bootloader area*/
            ReturnValue = false;
            SendGenericResponse(MEMORY_AREA, WRITING_INVALID);
        }
        else if (Address > MEM_APPCODE_END_ADDRESS)
        {
            /* Writing in Configuration Page
             * No error sent : Ignore the writing*/
            ReturnValue = false;
        }
        else
        {
            ReturnValue = true;
        }
    }
    else if ((MemoryArea & 0x0F) == 4)
    {
        if (Address < Info.OppositBootStartAddr || Address > Info.OppositBootEndAddr)
        {
            ReturnValue = false;
            SendGenericResponse(MEMORY_AREA, WRITING_INVALID);
        }
        else
        {
            ReturnValue = true;
        }
    }
    else
    {
        /* Writing logistic information*/
        if (Address < FLAG_APPLI_ADDRESS || Address >= (FLAG_APPLI_ADDRESS + FLAG_TOTAL_LEN))
        {
            /*Writing before or after last page*/
            ReturnValue = false;
            SendGenericResponse(MEMORY_AREA, WRITING_INVALID);
        }
        else
        {
            /*Good area*/
            ReturnValue = true;
        }
    }

    return ReturnValue;
}

void CRCWrite(tCANMsgObject ReceivedMessage, MyBootSys BootStatus)
{
    uint16_t WriteBuf[2];
    uint16_t ReceivedCRC;
    uint32_t wCRCAddr;
    uint16_t CRCFlash;

    if ((*ReceivedMessage.pucMsgData & 0x0F) == 4)
    {
        wCRCAddr = BootStatus.OppositBootCRCAddr;
    }
    else
    {
        wCRCAddr = APP_CRC_ADDRESS;
    }

    u32UpdataFlag = 0;
    ReceivedCRC = ((uint16_t) ReceivedMessage.pucMsgData[1] << 8) + ReceivedMessage.pucMsgData[2];

    if ((ReceivedMessage.pucMsgData[0] & 0x0F) == 4)
    {
        /*Calculate CRC for boot memory area*/
        CRCFlash = CalcCRC_FLASH(0, BootStatus.OppositBootStartAddr, BOOT_TOTAL_LEN);
    }
    else
    {
        /*Calculate the CRC for app memory area*/
        CRCFlash = CalcCRC_FLASH(0, MEM_APPCODE_START_ADDRESS, APP_TOTAL_LEN);
    }

    if (CRCFlash == ReceivedCRC)
    {
        /*Send OK response */
        SendGenericResponse(MEMORY_AREA, NO_ERROR);
//        ClrWdt();
//        TMR3_Start();
        /*Wait for frame transmission*/
//        while (TMR3_SoftwareCounterGet() <= 5) {
//            TMR3_Tasks_16BitOperation();
//        }
        //TMR3_Stop();
        if ((ReceivedMessage.pucMsgData[0] & 0x0F) == 4)
        {
            WriteBuf[0] = (uint16_t) BootStatus.OppositBootValidCode;
            WriteBuf[1] = (uint16_t) (BootStatus.OppositBootValidCode >> 16);

            WriteFlash(BootStatus.OppositBootFlagValidAddr, WriteBuf, 2);
//            Erase_Flash(BootStatus.BootValidFlagAddr);
            ucAppMemoryErase = false;
            RESET();
        }
        else
        {
            WriteBuf[0] = (uint16_t) APP_VALID;
            WriteBuf[1] = (uint16_t) (APP_VALID >> 16);
            WriteFlash(FLAG_APPLI_ADDRESS, WriteBuf, 2);
            ucAppMemoryErase = false;
            RESET();
        }
    }
    else
    {
        /*Wrong CRC*/
        SendGenericResponse(MEMORY_AREA, WRONG_CRC);
    }

}

void LogisticCRCWrite(tCANMsgObject ReceivedMessage)
{
    uint16_t ReceivedCRC, HardwareVersionRead, HardwareSNRead[4], DataForCRC[5], CRCCalc;

    if((ReceivedMessage.pucMsgData[0] & 0x0F) == 1)
    {
        /* Hardware version case */
        /*Save received CRC*/
        ReceivedCRC = (ReceivedMessage.pucMsgData[1] << 8) + ReceivedMessage.pucMsgData[2];
        /*Read Saved data*/
        HardwareVersionRead = (uint16_t) Read_Data_Word(HW_VERSION_ADDRESS);
        /*Calculate CRC including received CRC*/
        DataForCRC[0] = HardwareVersionRead;
        DataForCRC[1] = 0xFFFF;
        DataForCRC[2] = 0xFFFF;
        DataForCRC[3] = 0xFFFF;
        DataForCRC[4] = ReceivedCRC;

        CRCCalc = CRC16(0, (uint8_t *)DataForCRC, HW_VERSION_SIZE);
        /*Add received CRC to calculated CRC*/
        CRCCalc = CRC16(CRCCalc, (uint8_t *)&DataForCRC[4], CRC_LENGTH);
        if(CRCCalc == 0xF0B8)
        {
            /*Correct CRC : save*/
            SeizeFlashPump_Bank1();
            WriteFlash(HW_VERSION_ADDRESS, DataForCRC, 5);
            SendGenericResponse(MEMORY_AREA, NO_ERROR);
        }
        else
        {
            /*Wrong CRC*/
            SendGenericResponse(MEMORY_AREA, WRONG_CRC);
        }
    }
    else if((ReceivedMessage.pucMsgData[0] & 0x0F) == 2)
    {
        /*Serial Number case*/
        /* Save in RAM received CRC*/
        ReceivedCRC = (ReceivedMessage.pucMsgData[1] << 8) + ReceivedMessage.pucMsgData[2];

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
        CRCCalc = CRC16(0, (uint8_t *)DataForCRC, HW_SERIAL_NUMBER_SIZE);
        /*Add received CRC to calculated CRC*/
        CRCCalc = CRC16(CRCCalc, (uint8_t *)&DataForCRC[4], CRC_LENGTH);

        if(CRCCalc == 0xF0B8)
        {
            /* Correct CRC : save CRC*/
            SeizeFlashPump_Bank1();
            WriteFlash(HW_SERIAL_NUMBER_ADDRESS, DataForCRC, 5);
            SendGenericResponse(MEMORY_AREA, NO_ERROR);
        }
        else
        {
            /*Wrong CRC*/
            SendGenericResponse(MEMORY_AREA, WRONG_CRC);
        }
    }
}

