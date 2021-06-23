/*
 * scicom.c
 *
 *  Created on: 2021Äê6ÔÂ16ÈÕ
 *      Author: E9981231
 */

#include "scicom.h"
#include "cancom.h"
#include "Timer.h"
#include <SciStack/APL/SCI_APL.h>


/*************************************************************************
**    global functions
*************************************************************************/
ST_SEC2PRI_COM_TX_OBJ g_stSec2PriTxObj;
ST_PRI2SEC_COM_RX_OBJ g_stPri2SecRxObj;
bool g_bSCI_TX_Flag = false;
Uint16 g_u16SCIMsgId;

extern __interrupt void ISR_SCIB_RX(void);      /**< Defined in interrupt.c*/
extern __interrupt void ISR_SCIB_TX(void);      /**< Defined in interrupt.c*/

void SCI_Init(void)
{
#ifdef _LAUNCHXL_F28377S
    GPIO_SetupPinMux(18, GPIO_MUX_CPU1, 2); // 18 SCI TXDB
    GPIO_SetupPinOptions(18, GPIO_OUTPUT, GPIO_ASYNC);

    GPIO_SetupPinMux(19, GPIO_MUX_CPU1, 2);// 19 SCI RXDB
    GPIO_SetupPinOptions(19, GPIO_INPUT, GPIO_PUSHPULL);
#else
    GPIO_SetupPinMux(54, GPIO_MUX_CPU1, 6); // 54 SCI TXDB
    GPIO_SetupPinOptions(54, GPIO_OUTPUT, GPIO_PULLUP);

    GPIO_SetupPinMux(55, GPIO_MUX_CPU1, 6);// 55 SCI RXDB
    GPIO_SetupPinOptions(55, GPIO_INPUT, GPIO_PULLUP);
#endif
    BSW_SVC_SCI_APL_Init();
#if (SCI_PRISEC_CONTROLLER == SCI_CON_B)
    EALLOW;
    PieVectTable.SCIB_RX_INT =  &ISR_SCIB_RX;
    PieVectTable.SCIB_TX_INT =  &ISR_SCIB_TX;
    PieCtrlRegs.PIEIER9.bit.INTx3 = 1;    // enable SCI B RX interrupt
    PieCtrlRegs.PIEIER9.bit.INTx4 = 1;    // enable SCI B TX interrupt
    EDIS;
#endif
}

void SCI_Send_Cmd(Uint16 u16MsgId, uint8_t *data, uint8_t len)
{
    g_u16SCIMsgId = u16MsgId;
    stTplMsg_t stTplTxMsg;
    Uint16 pu16DataTmp[8] = {0};
    stTplTxMsg.pu16Data = pu16DataTmp;
    stTplTxMsg.u16MsgId = u16MsgId;

    TMR0_SoftwareCounterClear();
    switch(u16MsgId)
    {
    case SCI_EraseMemory:
        pu16DataTmp[0] = (uint16_t)data[0]<<8;
        stTplTxMsg.u16MsgLen = 1;
        g_bSCI_TX_Flag = true;
        SCI_TPL_Send(&stTplTxMsg);
        break;
    case SCI_SecurityAccess:
    case SCI_TransferInformation:
    case SCI_TransferData:
        if(data != NULL && len == 8)
        {
            pu16DataTmp[0] = (uint16_t)data[0]<<8 | data[1];
            pu16DataTmp[1] = (uint16_t)data[2]<<8 | data[3];
            pu16DataTmp[2] = (uint16_t)data[4]<<8 | data[5];
            pu16DataTmp[3] = (uint16_t)data[6]<<8 | data[7];
            stTplTxMsg.u16MsgLen = 4;
            g_bSCI_TX_Flag = true;
            SCI_TPL_Send(&stTplTxMsg);
        }
        break;
    case SCI_CRCRequest:
        if(data != NULL && len == 3)
        {
            pu16DataTmp[0] = (uint16_t)data[0]<<8 | data[1];
            pu16DataTmp[1] = (uint16_t)data[2]<<8;
            stTplTxMsg.u16MsgLen = 2;
            g_bSCI_TX_Flag = true;
            SCI_TPL_Send(&stTplTxMsg);
        }
        break;
    case SCI_LogisticRequest:

//        g_bSCI_TX_Flag = true;

        break;
    case SCI_SWVersionCheck:
        if(data != NULL && len == 6)
        {
            pu16DataTmp[0] = (uint16_t)data[0]<<8 | data[1];
            pu16DataTmp[1] = (uint16_t)data[2]<<8 | data[3];
            pu16DataTmp[2] = (uint16_t)data[4]<<8 | data[5];
            stTplTxMsg.u16MsgLen = 3;
            g_bSCI_TX_Flag = true;
            SCI_TPL_Send(&stTplTxMsg);
        }
        break;
    case SCI_ModeRequest:
        pu16DataTmp[2] = (uint16_t)BOOT_MODE<<8;
        stTplTxMsg.u16MsgLen = 4;
        g_bSCI_TX_Flag = true;
        SCI_TPL_Send(&stTplTxMsg);
        break;
    default:
        break;
    }


}
