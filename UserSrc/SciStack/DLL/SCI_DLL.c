/*********************************************************************
File name:       SCI_DLL.c
Purpose :        Send/Received message through SCI
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
#include <SciStack/DLL/SCI_DLL.h>
#include <SciStack/DLL/SCI_Buf.h>
#include <SciStack/ERR/SCI_ERR.h>

extern stRingBuff_t g_astRingBuf[];

#define SCI_RXERROR     (1<<7)
#define SCI_RXRDY       (1<<6)
#define SCI_BBRKDT      (1<<5)
#define SCI_FE          (1<<4)
#define SCI_OE          (1<<3)
#define SCI_PE          (1<<2)

/*************************************************************************************************************
 * Global Variables
 *************************************************************************************************************/

/*************************************************************************************************************
 * Internal function declaration
 *************************************************************************************************************/
volatile struct SCI_REGS *g_pstSciReg = NULL;

/*************************************************************************************************************
 * Internal variable declaration
 *************************************************************************************************************/
static volatile bool s_bInTransmiting = false;
static stSciConfig_t gs_SciConfig_Backup;

/*************************************************************************************************************
 * Function Definitions
 *************************************************************************************************************/

/******************************************************************************************
 * Function Name: SCI_DLL_Init
 * Description  : SCI controller and ring buffer initialization
 * Parameters   : enSciCtrler_t enController, SCI controller which need to be initialization
 *                stSciConfig_t *pstSciCfg, SCI communication configuration parameters
 * Returns      : i16SciHandle_t, SCI handle, if handle <0, initialization fail.
 * Remark       :
 *****************************************************************************************/
int16 SCI_DLL_Init(Uint16 u16Controller, stSciConfig_t *pstSciCfg)
{
    Uint16 u16BRR;

    Uint64 u64LSPCLK = 200E6/14; // LSPCLK = CPUfreq / lspdiv

    s_bInTransmiting = false;

    SCI_RingBuff_Init();

    gs_SciConfig_Backup.enBaud     = pstSciCfg->enBaud;
    gs_SciConfig_Backup.enParity   = pstSciCfg->enParity;
    gs_SciConfig_Backup.u16CharLen = pstSciCfg->u16CharLen;
    gs_SciConfig_Backup.u16Stopbit = pstSciCfg->u16Stopbit;

    switch(u16Controller)
    {
    case SCI_CON_A:
        g_pstSciReg = &SciaRegs;
        break;
    case SCI_CON_B:
        g_pstSciReg = &ScibRegs;
        break;
#if (SUPPORT_SCI_C == 1)
    case SCI_C:
        g_pstSciReg = &ScicRegs;
        break;
#endif
    default:
        return -1; // parameter invalid
    }

    g_pstSciReg->SCICTL1.bit.SWRESET        = 0;    // write a 0 to initializes the SCI state machine and operating flags

    if ((1 != pstSciCfg->u16Stopbit)
        && (2 != pstSciCfg->u16Stopbit))
    {
        return -1; // parameter invalid
    }
    g_pstSciReg->SCICCR.bit.STOPBITS        = pstSciCfg->u16Stopbit -1;

    switch (pstSciCfg->enParity)
    {
    case enNo_Parity:
        g_pstSciReg->SCICCR.bit.PARITYENA       = 0;    // Disable parity
        break;
    case enOdd_Parity:
        g_pstSciReg->SCICCR.bit.PARITYENA       = 1;    // Enable parity
        g_pstSciReg->SCICCR.bit.PARITY          = 0;    // Odd parity
        break;
    case enEven_Parity:
        g_pstSciReg->SCICCR.bit.PARITYENA       = 1;    // Enable parity
        g_pstSciReg->SCICCR.bit.PARITY          = 1;    // Even parity
        break;
    default:
        return -1; // parameter invalid
    }

    if (pstSciCfg->u16CharLen>8)
    {
        return -1; // parameter invalid
    }
    g_pstSciReg->SCICCR.bit.SCICHAR         = pstSciCfg->u16CharLen - 1;

    g_pstSciReg->SCICCR.bit.LOOPBKENA       = 0;    // Loop back test mode disable
    g_pstSciReg->SCICCR.bit.ADDRIDLE_MODE   = 0;    // idle-line mode protocol selected
    g_pstSciReg->SCICTL1.bit.RXERRINTENA    = 0;    // receive error interrupt enable
    g_pstSciReg->SCICTL1.bit.TXWAKE         = 0;    // SCI transmitter wake-up method select
    g_pstSciReg->SCICTL1.bit.SLEEP          = 0;    // sleep mode disabled
    g_pstSciReg->SCICTL1.bit.TXENA          = 1;    // transmitter enabled
    g_pstSciReg->SCICTL1.bit.RXENA          = 1;    // receiver enable

    /*
    CPU_FREQ        200E6
    LSPCLK_FREQ     CPU_FREQ/14
    SCI_BRR         ((LSPCLK_FREQ/(SCI_baud*8))-1)
    SCI_BAUD        LSPCLK_FREQ/(SCI_BRR+1)/8
    */

    switch(pstSciCfg->enBaud)
    {
    case enBaud_1200:
        u16BRR = (Uint16)(u64LSPCLK/8/1200-1);
        break;
    case enBaud_9600:
        u16BRR = (Uint16)(u64LSPCLK/8/8600-1);
        break;
    case enBaud_19200:
        u16BRR = (Uint16)(u64LSPCLK/8/19200-1);
        break;
    case enBaud_38400:
        u16BRR = (Uint16)(u64LSPCLK/8/38400-1);
        break;
    case enBaud_56000:
        u16BRR = (Uint16)(u64LSPCLK/8/56000-1);
        break;
    case enBaud_57600:
        u16BRR = (Uint16)(u64LSPCLK/8/57600-1);
        break;
    case enBaud_115200:
        u16BRR = (Uint16)(u64LSPCLK/8/115200-1);
        break;
    case enBaud_256000:
        u16BRR = (Uint16)(u64LSPCLK/8/256000-1);
        break;
    default:
        u16BRR = (Uint16)(u64LSPCLK/8/38400-1);
        break;
    }

    g_pstSciReg->SCIHBAUD.bit.BAUD          = (u16BRR & 0xff00) >>8;    // 19200 bit/s
    g_pstSciReg->SCILBAUD.bit.BAUD          = u16BRR & 0x00ff;

    g_pstSciReg->SCICTL2.bit.RXBKINTENA     = 1;    // enable RXRDY/BRKDT interrupt
    g_pstSciReg->SCICTL2.bit.TXINTENA       = 1;    // enable TXRDY interrupt

    g_pstSciReg->SCIFFTX.bit.SCIFFENA       = 0;    // disable tx FIFO
    g_pstSciReg->SCIFFRX.bit.RXFFIENA       = 0;    // disable rx FIFO

    g_pstSciReg->SCICTL1.bit.SWRESET        = 1;    //

    return 0;
}

/******************************************************************************************
 * Function Name: SCI_DLL_Send
 * Description  : Send data to SCI
 * Parameters   : i16SciHandle_t i16Handle,  SCI handle
 *                Uint16 * pu16Data, data address which need to be sent
 *                 Uint16 u16Len, Data length to be sent
 * Returns      : true=success, false =fail
 * Remark       :
 *****************************************************************************************/
bool SCI_DLL_Send(Uint16 * pu16Data, Uint16 u16Len)
{
    if (NULL == g_pstSciReg)
    {
        return false;
    }

    if (s_bInTransmiting == true)
    {
        if (BUF_NO_ERROR != SCI_RingBuff_Write(enSCI_TX_Buf_ID, &pu16Data[0], u16Len))
        {
            SCI_ERR_SetErr(SCI_DLL_TX_BUFOVER_ERR);
            SCI_RingBuff_Reset(enSCI_TX_Buf_ID);

            return false;
        }
    }
    else
    {
        s_bInTransmiting = true;

        if (BUF_NO_ERROR != SCI_RingBuff_Write(enSCI_TX_Buf_ID, &pu16Data[1], u16Len-1))
        {
            SCI_RingBuff_Reset(enSCI_TX_Buf_ID);

            return false;
        }
        else
        {
            // start transmit, this operation will trigger the TX interrupt and the TX ISR will get data from ring buffer and send
            g_pstSciReg->SCITXBUF.all = pu16Data[0] & 0x00ff;
        }
    }

    return true;
}


/******************************************************************************************
 * Function Name: SCI_DLL_Receive
 * Description  : Receive one 16bit data from SCI
 * Parameters   : i16SciHandle_t i16Handle,  SCI handle
 *                Uint16 * pu16Data, data address which to save the message.
 * Returns      : true=success, false =fail
 * Remark       :
 *****************************************************************************************/
bool SCI_DLL_Receive(Uint16* pu16Data, Uint16 u16Len )
{
    if (BUF_NO_ERROR == SCI_RingBuff_Read(enSCI_RX_Buf_ID, pu16Data, u16Len))
    {
        return true;
    }
    else
    {
        return false;
    }
}


/******************************************************************************************
 * Function Name: SCI_RX_InterruptServiceRoutine
 * Description  : SCI Data receive interrupt Service Routine
 * Parameters   : enSciCtrler_t enController, SCI controller
 * Returns      : true=success, false =fail
 * Remark       :
 *****************************************************************************************/
#if (SCI_PRISEC_CONTROLLER == SCI_CON_A)
#pragma CODE_SECTION(SCIA_RX_InterruptServiceRoutine, ".TI.ramfunc");
void SCIA_RX_InterruptServiceRoutine(void)
{
    Uint16 u16Data;

    enSciBuffId_t enBufId = enSCI_RX_Buf_ID;
    Uint16 u16RxSts = SciaRegs.SCIRXST.all;

    if (u16RxSts & (SCI_RXERROR|SCI_BBRKDT|SCI_FE|SCI_OE|SCI_PE))
    {
        if (u16RxSts & SCI_BBRKDT)
        {
            SCI_ERR_SetErr(SCI_DLL_DISCONNECT_ERR);
        }
        else
        {
            SCI_ERR_SetErr(SCI_DLL_COM_ERR);
        }
    }
    else
    {
        if (u16RxSts & SCI_RXRDY)
        {
            /* get data and buff status from register. the register is read-clear register */
            u16Data = SciaRegs.SCIRXBUF.all;

            /*
            * check SCI controller internal buffer error flag
            * SCIRXBUF.bit.SCIFFFE
            * SCIRXBUF.bit.SCIFFPE
            */
            if (u16Data & 0xff00)
            {
                SCI_ERR_SetErr(SCI_DLL_COM_ERR);
            }
            else
            {
                u16Data &= 0x00ff; // get SCIRXBUF.bit.SAR

                /* below segment implement these 2 functions
                if (BUF_NO_ERROR != SCI_RingBuff_Write(enBufId, &u16Data, 1))
                    SCI_RingBuff_Reset(enBufId);
                 */

                // no enough space in buffer to write
                 if (0 < g_astRingBuf[enBufId].u16RemainNum)  // use global variable to void function, and shorten the ISR execute time
                 {
                     // write the  data to the front of  the correspond ring buffer(RX/TX)
                     g_astRingBuf[enBufId].au16Buf[g_astRingBuf[enBufId].u16Fornt] = u16Data;
                     g_astRingBuf[enBufId].u16Fornt ++;
                     g_astRingBuf[enBufId].u16RemainNum --;

                     if (g_astRingBuf[enBufId].u16Fornt == DRV_RINGBUFF_SIZE)
                     {
                         g_astRingBuf[enBufId].u16Fornt = 0;
                     }
                 }
                 else
                 {
                     SCI_ERR_SetErr(SCI_DLL_RX_BUFOVER_ERR);
                 }
            }
        }
    }
}
#endif

/******************************************************************************************
 * Function Name: SCI_RX_InterruptServiceRoutine
 * Description  : SCI Data receive interrupt Service Routine
 * Parameters   : enSciCtrler_t enController, SCI controller
 * Returns      : true=success, false =fail
 * Remark       :
 *****************************************************************************************/
#if (SCI_PRISEC_CONTROLLER == SCI_CON_B)
#pragma CODE_SECTION(SCIB_RX_InterruptServiceRoutine, ".TI.ramfunc");
void SCIB_RX_InterruptServiceRoutine(void)
{
    Uint16 u16Data;

    enSciBuffId_t enBufId = enSCI_RX_Buf_ID;
    Uint16 u16RxSts = ScibRegs.SCIRXST.all;

    if (u16RxSts & (SCI_RXERROR|SCI_BBRKDT|SCI_FE|SCI_OE|SCI_PE))
    {
        if (u16RxSts & SCI_BBRKDT)
        {
            SCI_ERR_SetErr(SCI_DLL_DISCONNECT_ERR);
        }
        else
        {
            SCI_ERR_SetErr(SCI_DLL_COM_ERR);
        }
    }
    else
    {
        if (u16RxSts & SCI_RXRDY)
        {
            /* get data and buff status from register. the register is read-clear register */
            u16Data = ScibRegs.SCIRXBUF.all;

            /*
            * check SCI controller internal buffer error flag
            * SCIRXBUF.bit.SCIFFFE
            * SCIRXBUF.bit.SCIFFPE
            */
            if (u16Data & 0xff00)
            {
                SCI_ERR_SetErr(SCI_DLL_COM_ERR);
            }
            else
            {
                u16Data &= 0x00ff; // get SCIRXBUF.bit.SAR

                /* below segment implement these 2 functions
                if (BUF_NO_ERROR != SCI_RingBuff_Write(enBufId, &u16Data, 1))
                    SCI_RingBuff_Reset(enBufId);
                */

                // no enough space in buffer to write
                 if (0 < g_astRingBuf[enBufId].u16RemainNum) // use global variable to void function, and shorten the ISR execute time
                 {
                     // write the  data to the front of  the correspond ring buffer(RX/TX)
                     g_astRingBuf[enBufId].au16Buf[g_astRingBuf[enBufId].u16Fornt] = u16Data;
                     g_astRingBuf[enBufId].u16Fornt ++;
                     g_astRingBuf[enBufId].u16RemainNum --;

                     if (g_astRingBuf[enBufId].u16Fornt == DRV_RINGBUFF_SIZE)
                     {
                         g_astRingBuf[enBufId].u16Fornt = 0;
                     }
                 }
                 else
                 {  // reset ring buffer
                     SCI_ERR_SetErr(SCI_DLL_RX_BUFOVER_ERR);
                 }
            }
        }
    }
}
#endif

/******************************************************************************************
 * Function Name: SCI_TX_InterruptServiceRoutine
 * Description  : SCI Data transmit interrupt Service Routine
 * Parameters   : enSciCtrler_t enController, SCI controller
 * Returns      : true=success, false =fail
 * Remark       :
 *****************************************************************************************/
#if (SCI_PRISEC_CONTROLLER == SCI_CON_B)
#pragma CODE_SECTION(SCIB_TX_InterruptServiceRoutine, ".TI.ramfunc");
void SCIB_TX_InterruptServiceRoutine(void)
{
    Uint16 enBufId   = enSCI_TX_Buf_ID;
    Uint16 u16TxData = 0;

     // get data from rear of  the correspond ring buffer(RX/TX)
     //SCI_RingBuff_Read(enBufId, &u16TxData, 1)
     if (DRV_RINGBUFF_SIZE > g_astRingBuf[enBufId].u16RemainNum)
     {
         u16TxData = g_astRingBuf[enBufId].au16Buf[g_astRingBuf[enBufId].u16Rear];
         g_astRingBuf[enBufId].u16Rear ++;
         g_astRingBuf[enBufId].u16RemainNum ++;

         if (DRV_RINGBUFF_SIZE == g_astRingBuf[enBufId].u16Rear)
         {
             g_astRingBuf[enBufId].u16Rear = 0;
         }

         ScibRegs.SCITXBUF.all = u16TxData&0x00ff;
     }
     else
     {
         s_bInTransmiting = false;
     }
}
#endif

/******************************************************************************************
 * Function Name: SCI_TX_InterruptServiceRoutine
 * Description  : SCI Data transmit interrupt Service Routine
 * Parameters   : enSciCtrler_t enController, SCI controller
 * Returns      : true=success, false =fail
 * Remark       :
 *****************************************************************************************/
#if (SCI_PRISEC_CONTROLLER == SCI_CON_A)
#pragma CODE_SECTION(SCIA_TX_InterruptServiceRoutine, ".TI.ramfunc");
void SCIA_TX_InterruptServiceRoutine(void)
{
    Uint16 enBufId   = enSCI_TX_Buf_ID;
    Uint16 u16TxData = 0;

     // get data from rear of  the correspond ring buffer(RX/TX)
     //SCI_RingBuff_Read(enBufId, &u16TxData, 1)
     if (DRV_RINGBUFF_SIZE > g_astRingBuf[enBufId].u16RemainNum)
     {
         u16TxData = g_astRingBuf[enBufId].au16Buf[g_astRingBuf[enBufId].u16Rear];
         g_astRingBuf[enBufId].u16Rear ++;
         g_astRingBuf[enBufId].u16RemainNum ++;

         if (DRV_RINGBUFF_SIZE == g_astRingBuf[enBufId].u16Rear)
         {
             g_astRingBuf[enBufId].u16Rear = 0;
         }

         SciaRegs.SCITXBUF.all = u16TxData&0x00ff;
     }
     else
     {
         s_bInTransmiting = false;
     }
}
#endif

/******************************************************************************************
 * Function Name: SCI_DLL_ErrHandle
 * Description  : Reset DLL
 * Parameters   : enSciCtrler_t enController,
 * Returns      :
 * Remark       :
 *****************************************************************************************/
void SCI_DLL_ErrHandle(void)
{
    Uint16 u16ErrFlag = SCI_ERR_GetErrStatus();

    if (u16ErrFlag & SCI_DLL_TX_BUFOVER_ERR)
    {
        SCI_RingBuff_Reset(enSCI_TX_Buf_ID);
        SCI_ERR_ClearErrStatus(SCI_DLL_TX_BUFOVER_ERR);
    }

    if (u16ErrFlag & SCI_DLL_RX_BUFOVER_ERR)
    {
        SCI_RingBuff_Reset(enSCI_RX_Buf_ID);
        SCI_ERR_ClearErrStatus(SCI_DLL_RX_BUFOVER_ERR);
    }

    if ((u16ErrFlag & SCI_DLL_DISCONNECT_ERR)
        || (u16ErrFlag & SCI_DLL_COM_ERR))
    {
        SCI_DLL_Reset();

        if (u16ErrFlag & SCI_DLL_DISCONNECT_ERR)
        {
            SCI_ERR_ClearErrStatus(SCI_DLL_DISCONNECT_ERR);
        }
        else
        {
            SCI_ERR_ClearErrStatus(SCI_DLL_COM_ERR);
        }
    }
}

/******************************************************************************************
 * Function Name: SCI_DLL_Reset
 * Description  : Reset DLL
 * Parameters   : enSciCtrler_t enController,
 * Returns      :
 * Remark       :
 *****************************************************************************************/
void SCI_DLL_Reset()
{
    Uint16 u16BRR;
    stSciConfig_t *pstSciCfg = &gs_SciConfig_Backup;
    Uint64 u64LSPCLK = 200E6/14; // LSPCLK = CPUfreq / lspdiv

    if (g_pstSciReg == NULL)
    {
        return;
    }

    s_bInTransmiting = false;

    SCI_RingBuff_Init();

    g_pstSciReg->SCICTL1.bit.SWRESET        = 0;    // write a 0 to initializes the SCI state machine and operating flags

    g_pstSciReg->SCICCR.bit.STOPBITS        = pstSciCfg->u16Stopbit -1;

    switch (pstSciCfg->enParity)
    {
    case enNo_Parity:
        g_pstSciReg->SCICCR.bit.PARITYENA       = 0;    // Disable parity
        break;
    case enOdd_Parity:
        g_pstSciReg->SCICCR.bit.PARITYENA       = 1;    // Enable parity
        g_pstSciReg->SCICCR.bit.PARITY          = 0;    // Odd parity
        break;
    case enEven_Parity:
        g_pstSciReg->SCICCR.bit.PARITYENA       = 1;    // Enable parity
        g_pstSciReg->SCICCR.bit.PARITY          = 1;    // Even parity
        break;
    default:
        return; // parameter invalid
    }

    g_pstSciReg->SCICCR.bit.SCICHAR         = pstSciCfg->u16CharLen - 1;

    g_pstSciReg->SCICCR.bit.LOOPBKENA       = 0;    // Loop back test mode disable
    g_pstSciReg->SCICCR.bit.ADDRIDLE_MODE   = 0;    // idle-line mode protocol selected
    g_pstSciReg->SCICTL1.bit.RXERRINTENA    = 0;    // receive error interrupt enable
    g_pstSciReg->SCICTL1.bit.TXWAKE         = 0;    // SCI transmitter wake-up method select
    g_pstSciReg->SCICTL1.bit.SLEEP          = 0;    // sleep mode disabled
    g_pstSciReg->SCICTL1.bit.TXENA          = 1;    // transmitter enabled
    g_pstSciReg->SCICTL1.bit.RXENA          = 1;    // receiver enable

    switch(pstSciCfg->enBaud)
    {
    case enBaud_1200:
        u16BRR = (Uint16)(u64LSPCLK/8/1200-1);
        break;
    case enBaud_9600:
        u16BRR = (Uint16)(u64LSPCLK/8/8600-1);
        break;
    case enBaud_19200:
        u16BRR = (Uint16)(u64LSPCLK/8/19200-1);
        break;
    case enBaud_38400:
        u16BRR = (Uint16)(u64LSPCLK/8/38400-1);
        break;
    case enBaud_56000:
        u16BRR = (Uint16)(u64LSPCLK/8/56000-1);
        break;
    case enBaud_57600:
        u16BRR = (Uint16)(u64LSPCLK/8/57600-1);
        break;
    case enBaud_115200:
        u16BRR = (Uint16)(u64LSPCLK/8/115200-1);
        break;
    case enBaud_256000:
        u16BRR = (Uint16)(u64LSPCLK/8/256000-1);
        break;
    default:
        u16BRR = (Uint16)(u64LSPCLK/8/38400-1);
        break;
    }

    g_pstSciReg->SCIHBAUD.bit.BAUD          = (u16BRR & 0xff00) >>8;    // 19200 bit/s
    g_pstSciReg->SCILBAUD.bit.BAUD          = u16BRR & 0x00ff;

    g_pstSciReg->SCICTL2.bit.RXBKINTENA     = 1;    // enable RXRDY/BRKDT interrupt
    g_pstSciReg->SCICTL2.bit.TXINTENA       = 1;    // enable TXRDY interrupt

    g_pstSciReg->SCIFFTX.bit.SCIFFENA       = 0;    // disable tx FIFO
    g_pstSciReg->SCIFFRX.bit.RXFFIENA       = 0;    // disable rx FIFO

    g_pstSciReg->SCICTL1.bit.SWRESET        = 1;    //

}
