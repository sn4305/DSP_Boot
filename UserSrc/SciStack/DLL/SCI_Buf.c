/*********************************************************************
File name:       SCI_Buf.c
Purpose :        Ring buffer for SCI receive/transmit message
Copyright Notice:
All source code and data contained in this file is Proprietary and
Confidential to Eaton, and must not be reproduced, transmitted, or
disclosed; in whole or in part, without the express written permission of Eaton.

Copyright 2021 - Eaton, All Rights Reserved.

Author                Date                      Ver#        Description
Jason.Chen         2021/05/20                   1.0          Initialization version


append log after baseline, especially for milestone or release version, no log is allowed for minor modification
**********************************************************************/
/*************************************************************************************************************
 * Include Files
 *************************************************************************************************************/
#include <SciStack/DLL/SCI_Buf.h>
#include <SciStack/SCI_Stack_Conf.h>
/*************************************************************************************************************
 * Macro Definitions
 *************************************************************************************************************/
#define DRV_RINGBUFF_NUM        2


/*************************************************************************************************************
 * Internal variable definition
 *************************************************************************************************************/
stRingBuff_t g_astRingBuf[DRV_RINGBUFF_NUM];


/*************************************************************************************************************
 * Function Definitions
 *************************************************************************************************************/
/******************************************************************************************
 * Function Name: RingBuff_Init
 * Description  : ring buffer initialization
 * Parameters   : void
 * Returns      : void
 * Remark       :
 *****************************************************************************************/
void SCI_RingBuff_Init(void)
{
    Uint16 u16Num = 0;

    // init all number ring buffers
    for (u16Num = 0; u16Num<DRV_RINGBUFF_NUM; u16Num++)
    {
        // init the content of each ring buffer
        g_astRingBuf[u16Num].u16Fornt = 0;
        g_astRingBuf[u16Num].u16Rear = 0;
        g_astRingBuf[u16Num].u16RemainNum = DRV_RINGBUFF_SIZE;
    }
}

/******************************************************************************************
 * Function Name: RingBuff_Reset
 * Description  : ring buffer reset
 * Parameters   : SCI buffer id
 * Returns      : err_t, error id, 0=No error, others=error,
 * Remark       :
 *****************************************************************************************/
enBufErr_t SCI_RingBuff_Reset(enSciBuffId_t enId)
{
    if (enId >= DRV_RINGBUFF_NUM)
    {
        return ERROR_WRONG_PARAM;
    }

    g_astRingBuf[enId].u16Fornt = 0;
    g_astRingBuf[enId].u16Rear = 0;
    g_astRingBuf[enId].u16RemainNum = DRV_RINGBUFF_SIZE;

    return BUF_NO_ERROR;
}



/******************************************************************************************
 * Function Name: RingBuff_Write
 * Description  : write the data to ring buffer
 * Parameters   : enSciBuffId_t enId, ring buffer id
 *                Uint16* pu16Data,   the address of data  which to be write
 *                Uint16 u16Num,      Data length
 * Returns      : err_t, error id, 0=No error, others=error,
 * Remark       :
 *****************************************************************************************/
enBufErr_t SCI_RingBuff_Write(enSciBuffId_t enId, Uint16* pu16Data, Uint16 u16Num)
{
    Uint16 i = 0;

    if ((enId>=DRV_RINGBUFF_NUM) || (NULL == pu16Data) || (u16Num > DRV_RINGBUFF_SIZE))
    {
        return ERROR_WRONG_PARAM;
    }

    // no enough space in buffer to write
    if (u16Num > g_astRingBuf[enId].u16RemainNum)
    {
        return ERROR_NO_SPACE;
    }

    for (i=0; i<u16Num; i++)
    {
        // write the  data to the front of  the correspond ring buffer(RX/TX)
        g_astRingBuf[enId].au16Buf[g_astRingBuf[enId].u16Fornt] = pu16Data[i];
        g_astRingBuf[enId].u16Fornt ++;
        g_astRingBuf[enId].u16RemainNum --;

        if (g_astRingBuf[enId].u16Fornt == DRV_RINGBUFF_SIZE)
        {
            g_astRingBuf[enId].u16Fornt = 0;
        }
    }

    return BUF_NO_ERROR;
}

/******************************************************************************************
 * Function Name: RingBuff_Read
 * Description  : read the data to ring buffer
 * Parameters   : enSciBuffId_t enId, ring buffer id
 *                Uint16* pu16Data,   the address of data  which to be read
 *                Uint16 u16Num,      Data length
 * Returns      : err_t, error id, 0=No error, others=error,
 * Remark       :
 *****************************************************************************************/
enBufErr_t SCI_RingBuff_Read(enSciBuffId_t enId, Uint16* pu16Data, Uint16 u16Num)
{
    Uint16 u16Idx = 0;

    if ((enId>=DRV_RINGBUFF_NUM) || (NULL == pu16Data) || (u16Num > DRV_RINGBUFF_SIZE))
    {
        return ERROR_WRONG_PARAM;
    }

    // buffer no data or no enough data to read
    if ((DRV_RINGBUFF_SIZE == g_astRingBuf[enId].u16RemainNum) || ((DRV_RINGBUFF_SIZE - g_astRingBuf[enId].u16RemainNum) < u16Num))
    {
        return ERROR_NO_DATA;
    }

    for (u16Idx=0; u16Idx<u16Num; u16Idx++)
    {
        // get data from rear of  the correspond ring buffer(RX/TX)
        pu16Data[u16Idx] = g_astRingBuf[enId].au16Buf[g_astRingBuf[enId].u16Rear];
        g_astRingBuf[enId].u16Rear ++;
        g_astRingBuf[enId].u16RemainNum ++;

        if (DRV_RINGBUFF_SIZE == g_astRingBuf[enId].u16Rear)
        {
            g_astRingBuf[enId].u16Rear = 0;
        }
    }

    return BUF_NO_ERROR;
}
