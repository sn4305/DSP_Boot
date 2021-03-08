/*
 * ErrHandler.c
 *
 *  Created on: 2021Äê3ÔÂ2ÈÕ
 *      Author: E9981231
 */

#include "ErrHandler.h"

extern const uint16_t u40BootVersion[3];

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
            data0 != 0x41 && data0 != 0x42)
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

uint8_t IsTransferInfoValid(tCANMsgObject Received_Message, bool ucSecurityUnlocked, bool ucAppMemoryErase, bool ucLogMemoryErase)
{
    uint8_t error = NO_ERROR;
    uint8_t data0 = *(Received_Message.pucMsgData);
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
            data0 != 0x41 && data0 != 0x42)
    {
        error = ID_NOT_SUPPORTED;
    }
    else if (((data0 & 0x0F) == 0) && ucAppMemoryErase  != true)
    {
        error = MEMORY_NOT_BLANK;
    }
    else
    {
        error = NO_ERROR;
    }
    return error;
}

uint8_t IsTransferDataValid(tCANMsgObject Received_Message, bool InfoReceived, uint8_t SN, uint16_t CopiedData, uint16_t LenDataToCopy)
{
    uint8_t error = NO_ERROR;
    uint8_t data0 = *(Received_Message.pucMsgData);

    if(TMR3_SoftwareCounterGet() >= 10){ /*50ms*/
        /*Timeout*/
        error = TIMEOUT;
        TMR3_SoftwareCounterClear();
    }

    if(!InfoReceived)
    {
        error = WRONG_REQUEST_FORMAT;
    }
    else if(data0 == (SN - 1))
    {
        /* Same sequence number as previous frame: ignore the frame*/
        error = 20;
    }
    else if(data0 != (SN + 1) || data0 > MAX_SN)
    {
        error = WRONG_REQUEST_FORMAT;
    }
    else if(Received_Message.ui32MsgLen != 8)
    {
        if((CopiedData + Received_Message.ui32MsgLen - CRC_LENGTH) != LenDataToCopy)
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
            data0 != 0x41 && data0 != 0x42)
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
        if (ActualVersion[i] != *data)
        {
            DiffErr ++;
        }
        data++;
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
