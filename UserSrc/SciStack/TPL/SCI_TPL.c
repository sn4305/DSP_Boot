/*********************************************************************
File name:       SCI_TPL.c
Purpose :        Pack/Unpack SCI message
Copyright Notice:
All source code and data contained in this file is Proprietary and
Confidential to Eaton, and must not be reproduced, transmitted, or
disclosed; in whole or in part, without the express written permission of Eaton.

Copyright 2021 - Eaton, All Rights Reserved.

Author                Date                      Ver#        Description
Jason.Chen         2021/05/20                   1.0          Initialization version


append log after baseline, especially for milestone or release version, no log is allowed for minor modification
**********************************************************************/
#include <SciStack/ERR/SCI_ERR.h>
#include <SciStack/TPL/SCI_TPL.h>
#include <SciStack/SCI_Stack_Conf.h>

static Uint16 Checksum_Calc(Uint16 *pu16Data, Uint16 u16Length);
static Uint16 s_au16TplTxData[TPL_BUFF_SIZE]; // define as static variable , it is to avoid stack over flow.


typedef enum
{
    enHeader_0,
    enHeader_1,
    enHeader_2,
    enLength,
    enMsgId,
    enData,
    enChecksum,
}enRxProtoPhase_t;


/******************************************************************************************
 * Function Name: SCI_TPL_Init
 * Description  : SCI controller and ring buffer initialization
 * Parameters   : enSciCtrler_t enController, SCI controller which need to be initialization
 * Returns      : i16SciHandle_t, SCI handle, if handle <0, initialization fail.
 * Remark       :
 *****************************************************************************************/
void SCI_TPL_Init(Uint16 u16Controller)
{
    stSciConfig_t stSciCfg;

    stSciCfg.u16Stopbit = 1;
    stSciCfg.enParity   = enNo_Parity;
    stSciCfg.u16CharLen = 8;
    stSciCfg.enBaud     = SCI_BAUDRATE;

     SCI_DLL_Init( u16Controller, &stSciCfg);
}

/******************************************************************************************
 * Function Name: SCI_TPL_Send
 * Description  : Send data to SCI
 * Parameters   : i16SciHandle_t i16Handle,  SCI handle
 *                stTplMsg_t *pstTPLmsg
 * Returns      : true=success, false =fail
 * Remark       :
 *****************************************************************************************/
bool SCI_TPL_Send(stTplMsg_t *pstTplMsg)
{
    Uint16 u16Index;
    Uint16 u16Number;

    if ((NULL == pstTplMsg)
        || (NULL == pstTplMsg->pu16Data))
    {
        return false;
    }

    // init TPL message header
    s_au16TplTxData[0] = 0xAA;
    s_au16TplTxData[1] = 0x55;
    s_au16TplTxData[2] = 0xAA;
    // the length include: flag+Data[]+CRC
    s_au16TplTxData[3] = pstTplMsg->u16MsgLen*2+2;

    s_au16TplTxData[4] = pstTplMsg->u16MsgId;

    /*SCI only can transmit 8bit data, so need the seperate the 16bit data to 2 8bit*/
    for (u16Index=0; u16Index < pstTplMsg->u16MsgLen; u16Index++)
    {
        s_au16TplTxData[5+u16Index*2]   = (pstTplMsg->pu16Data[u16Index] & 0xff00)>>8;
        s_au16TplTxData[5+u16Index*2+1] = pstTplMsg->pu16Data[u16Index] & 0x00ff;
        u16Number                       = 5+u16Index*2+1+1; // total data number in buffer
    }

    s_au16TplTxData[u16Number] = Checksum_Calc(s_au16TplTxData, u16Number);

    return SCI_DLL_Send(s_au16TplTxData, u16Number+1);
}

/******************************************************************************************
 * Function Name: SCI_TPL_Receive
 * Description  : Receive one message from SCI
 * Parameters   : i16SciHandle_t i16Handle,  SCI handle
 * Returns      : true=success, false =fail
 * Remark       :
 *****************************************************************************************/
bool SCI_TPL_Receive(stTplMsg_t *pstTplMsg)
{
    Uint16 u16RxData;
    bool bRet, bEndOfFrame = false;

    static enRxProtoPhase_t s_enRxProPhase = enHeader_0;
    static Uint16 s_len = 0, s_u16Checksum = 0;


    while (true == SCI_DLL_Receive(&u16RxData, 1))
    {
        switch (s_enRxProPhase)
        {
        case enHeader_0:
            if (0xAA == u16RxData)
            {
                s_u16Checksum    = u16RxData;
                s_enRxProPhase     = enHeader_1;
            }
            break;
        case enHeader_1:
            if (0x55 == u16RxData)
            {
                s_u16Checksum   += u16RxData;
                s_enRxProPhase     = enHeader_2;
            }
            else if (0xAA == u16RxData)
            {
                s_enRxProPhase = enHeader_1;
            }
            else
            {
                s_enRxProPhase = enHeader_0;
            }
            break;
        case enHeader_2:
            if (0xAA == u16RxData)
            {
                s_u16Checksum   += u16RxData;
                s_enRxProPhase     = enLength;
            }
            else
            {
                s_enRxProPhase = enHeader_0;
            }
            break;
        case enLength:
            if (u16RxData <= TPL_BUFF_SIZE-6) // the message length should less than 32
            {
               // the u16RxData length include: flag+Data[]+CRC, the message length = Data[]
                pstTplMsg->u16MsgLen = u16RxData - 2; // save the message length
                s_u16Checksum       += u16RxData;
                s_enRxProPhase         = enMsgId;
            }
            else
            {
                s_enRxProPhase = enHeader_0;
                SCI_ERR_SetErr(SCI_TPL_WRONGLENGTH_ERR);
            }
            break;
        case enMsgId:
            pstTplMsg->u16MsgId  = u16RxData;
            s_u16Checksum       += u16RxData;
            s_enRxProPhase         = enData;
            break;
        case enData:
            pstTplMsg->pu16Data[s_len] = u16RxData;
            s_u16Checksum += u16RxData;
            s_len++;
            if (pstTplMsg->u16MsgLen == s_len) // all the data() received
            {
                s_enRxProPhase = enChecksum;
                s_len        = 0;
            }
            break;
        case enChecksum:
            s_enRxProPhase = enHeader_0;
            bEndOfFrame = true;
            if (u16RxData == (s_u16Checksum & 0x00ff))
            {
                bRet =  true;
            }
            else
            {
                SCI_ERR_SetErr(SCI_TPL_CHECKSUM_ERR);
                bRet = false;
            }
            break;
        default:
            s_enRxProPhase = enHeader_0;
            break;
        }

        if (bEndOfFrame)
        {
            return bRet;
        }
    }

    return false;
}

/******************************************************************************************
 * Function Name: SCI_TPL_ErrHandler
 * Description  : TPL error handler
 * Parameters   : i16SciHandle_t i16Handle,  SCI handle
 * Returns      : true=success, false =fail
 * Remark       :
 *****************************************************************************************/
void SCI_TPL_ErrHandle(void)
{
    Uint16 u16Err = SCI_ERR_GetErrStatus();

    if (u16Err & SCI_TPL_CHECKSUM_ERR)
    {
       // SCI_ERR_ClearErrStatus(SCI_TPL_CHECKSUM_ERR);
    }

    if (u16Err&SCI_TPL_WRONGLENGTH_ERR)
    {
       // SCI_ERR_ClearErrStatus(SCI_TPL_WRONGLENGTH_ERR);
    }

    SCI_DLL_ErrHandle();
}
/******************************************************************************************
 * Function Name:
 * Description  :
 * Parameters   :
 * Returns      :
 * Remark       :
 *****************************************************************************************/
static Uint16 Checksum_Calc(Uint16 *pu16Data, Uint16 u16Length)
{
    Uint16 i = 0;
    Uint16 u16Temp = 0;

    for (i=0; i<u16Length; i++)
    {
        u16Temp += pu16Data[i];
    }

    return (u16Temp&0x00ff);

}



