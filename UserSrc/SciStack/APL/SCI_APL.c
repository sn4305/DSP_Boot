/*********************************************************************
File name:       SCI_APL.h
Purpose :        SCI application layer module
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
#include <SciStack/APL/SCI_APL.h>
#include <SciStack/ERR/SCI_ERR.h>

/*************************************************************************************************************
 *  internal variable
 *************************************************************************************************************/
static const stSAplMsg_t gs_stSAplRegTxMsg[] = CNF_SCI_CYC_STX;
static const stSAplMsg_t gs_stSAplRegRxMsg[] = CNF_SCI_CYC_SRX;

static const stDAplTxMsg_t gs_stDAplRegTxMsg_backup[] = CNF_SCI_CYC_DTX;
static const stDAplRxMsg_t gs_stDAplRegRxMsg_backup[] = CNF_SCI_CYC_DRX;

static Uint16 gs_u16TxErrNum = 0;
static Uint16 gs_u16RxErrNum = 0;

static Uint16 gs_au16AplRxData[APL_BUFF_SIZE];
static stSAplMsg_t gs_stAplRxMsgTemp;

static Uint32 gs_u32TimerCounter = 0;
/*************************************************************************************************************
 * Global variable
 *************************************************************************************************************/
stDAplTxMsg_t g_stDAplRegTxMsg[sizeof(gs_stDAplRegTxMsg_backup)/sizeof(stDAplTxMsg_t)];
stDAplRxMsg_t g_stDAplRegRxMsg[sizeof(gs_stDAplRegRxMsg_backup)/sizeof(stDAplRxMsg_t)];
/*************************************************************************************************************
 *  internal function
 *************************************************************************************************************/
static void SCI_APL_Receive(void);
static void SCI_APL_SendMsg(const stSAplMsg_t *pstAplMsg);
static void SCI_APL_RxMsgHandle(Uint16 u16MsgId, Uint16 u16Len, Uint16 *pu16Data);
static void SCI_APL_Send(void);
static void SCI_APL_ErrHandler(void);
static void SCI_APL_RxMsgMonitor(void);

/*************************************************************************************************************
 * Function Definitions
 *************************************************************************************************************/

/******************************************************************************************
 * Function Name: SCI_APL_Init
 * Description  : SCI stack initialization
 * Parameters   :
 * Returns      :
 * Remark       :
 *****************************************************************************************/
void BSW_SVC_SCI_APL_Init(void)
{
    gs_stAplRxMsgTemp.u16MsgId   = 0;
    gs_stAplRxMsgTemp.u16MsgLen  = 0;
    gs_stAplRxMsgTemp.pu16Data   = gs_au16AplRxData;

    memcpy (g_stDAplRegTxMsg,(void *)gs_stDAplRegTxMsg_backup, sizeof(gs_stDAplRegTxMsg_backup));
    memcpy (g_stDAplRegRxMsg,(void *)gs_stDAplRegRxMsg_backup, sizeof(gs_stDAplRegRxMsg_backup));

    SCI_ERR_Init();

    SCI_TPL_Init(SCI_PRISEC_CONTROLLER);
}

/******************************************************************************************
 * Function Name: SCI_Main_Task
 * Description  : SCI stack main task ,the function should be executed every 2 ms
 * Parameters   :
 * Returns      :
 * Remark       :
 *****************************************************************************************/
void BSW_SVC_SCI_Main_Task2ms(void)
{
    SCI_APL_Send();

    SCI_APL_Receive();

    SCI_APL_RxMsgMonitor();

    SCI_APL_ErrHandler();
}

/******************************************************************************************
 * Function Name: SCI_APL_Send
 * Description  : Send data to SCI
 * Parameters   : i16SciHandle_t i16Handle,  SCI handle
 *                stTplMsg_t *pstTPLmsg
 * Returns      : true=success, false =fail
 * Remark       :
 *****************************************************************************************/
static void SCI_APL_SendMsg(const stSAplMsg_t *pstAplMsg)
{

    if (false == SCI_TPL_Send((stTplMsg_t*)pstAplMsg))
    {
        if (50 < gs_u16TxErrNum)
        {
            SCI_ERR_SetErr(SCI_APL_TX_ERR);
        }
        else
        {
            gs_u16TxErrNum ++;
        }
    }
    else
    {
        if (0 != gs_u16TxErrNum)
        {
            gs_u16TxErrNum --;
        }
    }
}

/******************************************************************************************
 * Function Name: SCI_APL_Receive
 * Description  : Receive one message from SCI
 * Parameters   : i16SciHandle_t i16Handle,  SCI handle
 *                stTplMsg_t *pstTPLmsg
 * Returns      : true=success, false =fail
 * Remark       :
 *****************************************************************************************/
static void SCI_APL_Receive(void)
{
    if (true == SCI_TPL_Receive((stTplMsg_t*)&gs_stAplRxMsgTemp))
    {
        SCI_APL_RxMsgHandle(gs_stAplRxMsgTemp.u16MsgId, gs_stAplRxMsgTemp.u16MsgLen, gs_stAplRxMsgTemp.pu16Data);
    }
}

/******************************************************************************************
 * Function Name: void SCI_APL_RxCallback(Uint16 u16MsgId, Uint16 u16Len, Uint16 *pu16Data)
 * Description  : the function will be called when TPL received the  messages
 * Parameters   :
 * Uint16 u16MsgId,  message id
 * Uint16 u16Len,    message length
 * Uint16 *pu16Data, message address
 * Returns      :
 * Remark       :
 *****************************************************************************************/
static void SCI_APL_RxMsgHandle(Uint16 u16MsgId, Uint16 u16Len, Uint16 *pu16Data)
{
    Uint16 i, j;

    for (i=0; i<sizeof(gs_stSAplRegRxMsg)/sizeof(stSAplMsg_t); i++)
    {
       if (gs_stSAplRegRxMsg[i].u16MsgId == u16MsgId) // message id
       {
           // message received, the correspondence timer set to 0
           g_stDAplRegRxMsg[i].u32Timer = 0;

           // set the flag that the data had received
           g_stDAplRegRxMsg[i].bMsgRecvived = true;

           for (j = 0; j<gs_stSAplRegRxMsg[i].u16MsgLen; j++)
           {
               // received from SCI is byte, but the application use data with 2 bytes format
               gs_stSAplRegRxMsg[i].pu16Data[j] = (pu16Data[j*2]<< 8) | (pu16Data[j*2+1]);
           }

           if (gs_stSAplRegRxMsg[i].u16MsgId & MSG_TYPE_EVENT_MUSK)
           {
#ifdef APP_RX_EVENT_CALLBACK
               APP_RX_CALLBACK(s_stSAplRegRxMsg[j].u16MsgId, s_stSAplRegRxMsg[i].u16MsgLen, s_stSAplRegRxMsg[i].pu16Data);
#endif
           }
       }
    }
}


/******************************************************************************************
 * Function Name: SCI_APL_ErrHandler
 * Description  : APL error handler
 * Parameters   : i16SciHandle_t i16Handle,  SCI handle
 * Returns      : true=success, false =fail
 * Remark       :
 *****************************************************************************************/
static void SCI_APL_ErrHandler(void)
{
    Uint16 u16Err = SCI_ERR_GetErrStatus();

    if (u16Err & SCI_APL_TX_ERR)
    {
        gs_u32TimerCounter  = 0;
        gs_u16TxErrNum      = 0;

        memcpy (g_stDAplRegTxMsg,(void *)gs_stDAplRegTxMsg_backup, sizeof(gs_stDAplRegTxMsg_backup));

        SCI_ERR_ClearErrStatus(SCI_APL_TX_ERR);
    }

    if (u16Err & SCI_APL_RX_PERIOD_ERR)
    {
        gs_u16RxErrNum      = 0;

        memcpy (g_stDAplRegRxMsg,(void *)gs_stDAplRegRxMsg_backup, sizeof(gs_stDAplRegRxMsg_backup));

        SCI_ERR_ClearErrStatus(SCI_APL_RX_PERIOD_ERR);
    }

    SCI_TPL_ErrHandle();
}

/******************************************************************************************
 * Function Name: SCI_APL_ErrHandler
 * Description  : APL error handler
 * Parameters   : i16SciHandle_t i16Handle,  SCI handle
 * Returns      : true=success, false =fail
 * Remark       :
 *****************************************************************************************/
static void SCI_APL_RxMsgMonitor(void)
{
    Uint16 i=0;

    /* Monitor the RX message period */
    for (i=0; i< sizeof(gs_stSAplRegRxMsg)/sizeof(stSAplMsg_t); i++)
    {
        if ((gs_stSAplRegRxMsg[i].u16MsgId & MSG_TYPE_EVENT_MUSK) == 0) // periodic message
        {
            g_stDAplRegRxMsg[i].u32Timer += 2;

           if ((g_stDAplRegRxMsg[i].u32Period << 2) < g_stDAplRegRxMsg[i].u32Timer)
           {
               if (50 < gs_u16RxErrNum)
               {
                   SCI_ERR_SetErr(SCI_APL_RX_PERIOD_ERR);
               }
               else
               {
                   gs_u16RxErrNum++;
               }
           }
           else
           {
               if (0 != gs_u16RxErrNum)
               {
                   gs_u16RxErrNum--;
               }
           }
        }
    }
}

/******************************************************************************************
 * Function Name: SCI_APL_Send
 * Description  : APL send
 * Parameters   :
 * Returns      :
 * Remark       :
 *****************************************************************************************/
static void SCI_APL_Send(void)
{
    Uint16 i = 0;

    gs_u32TimerCounter = gs_u32TimerCounter +2;

    /* Send  the TX message Periodic */
    for (i=0; i < sizeof(gs_stSAplRegTxMsg)/sizeof(stSAplMsg_t); i++)
    {
        if (g_stDAplRegTxMsg[i].bEnable) // send the enable message
        {
            if (gs_u32TimerCounter >= g_stDAplRegTxMsg[i].u32Timer) // the message timer counter is reach
            {
                SCI_APL_SendMsg(&gs_stSAplRegTxMsg[i]);

                if (gs_stSAplRegTxMsg[i].u16MsgId & MSG_TYPE_EVENT_MUSK) // event message, sent one time.
                {
                    g_stDAplRegTxMsg[i].bEnable = false;
                }
                else  // periodic message
                {
                    g_stDAplRegTxMsg[i].u32Timer += g_stDAplRegTxMsg[i].u32Period;
                }

                return;
            }
        }
    }
}
