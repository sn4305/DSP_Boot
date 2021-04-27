/**
 * @file cancom.c
 *
 *  Created on: 20210226
 *      Author: E9981231
 */

#include "cancom.h"

//
// Function Prototypes
//
void InitCanaGpio(void);
void InitCanaMbox(void);

static unsigned char s_u8txMsgData[8];      /**< CAN transmit buffer*/
static unsigned char s_u8rxMsgData[8];      /**< CAN receive buffer*/
tCANMsgObject s_stTXCANMessage;             /**< CAN transmit object*/
tCANMsgObject g_stRXCANMessage;             /**< CAN receive object*/

extern __interrupt void canaISR(void);      /**< Defined in interrupt.c*/

void InitCana(void)
{
    /* Initialize the CAN TX|RX IO */
    InitCanaGpio();
    /* Initialize the CAN controller */
    CANInit(CANA_BASE);
    /* Setup CAN to be clocked off the PLL output clock */
    CANClkSourceSelect(CANA_BASE, 0);
    /********************************************************************
    // Set up the CAN bus bit rate to 500kHz for CANA module
    // This function sets up the CAN bus timing for a nominal configuration.
    // You can achieve more control over the CAN bus timing by using the
    // function CANBitTimingSet() instead of this one, if needed.
    // Additionally, consult the device data sheet for more information about
    // the CAN module clocking.
    *********************************************************************/
    CANBitRateSet(CANA_BASE, 200000000, 500000);
    /* Enable interrupts on the CAN A peripheral. */
    CANIntEnable(CANA_BASE, CAN_INT_MASTER | CAN_INT_ERROR | CAN_INT_STATUS);

    /********************************************************************
    // Interrupts that are used in this example are re-mapped to
    // ISR functions found within this file.
    // This registers the interrupt handler in PIE vector table.
    ********************************************************************/
    EALLOW;
    PieVectTable.CANA0_INT = canaISR;
    EDIS;
    /* Enable the CAN-A0 interrupt on the processor (PIE). */
    PieCtrlRegs.PIEIER9.bit.INTx5 = 1;
    IER |= M_INT9;
    /* Initialize the CAN-A0 message box. */
    InitCanaMbox();
    /* Enable the CAN-A interrupt signal  */
    CANGlobalIntEnable(CANA_BASE, CAN_GLB_INT_CANINT0);
    /* Clear the global interrupt flag for the CAN interrupt line */
    CANGlobalIntClear(CANA_BASE, CAN_GLB_INT_CANINT0);
    /* Acknowledge this interrupt located in group 9 */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
}

/**
*  Configure CANa GPIO.
*  GPIO70 -  CANRXA; GPIO71 - CANTXA.
*  Calls: InitCana()
*************************************************/
void InitCanaGpio(void)
{
    GPIO_SetupPinMux(70, GPIO_MUX_CPU1, 5); /*GPIO70 -  CANRXA*/
    GPIO_SetupPinOptions(70, GPIO_INPUT, GPIO_ASYNC);
    GPIO_SetupPinMux(71, GPIO_MUX_CPU1, 5); /*GPIO71 - CANTXA*/
    GPIO_SetupPinOptions(71, GPIO_OUTPUT, GPIO_PUSHPULL);
}

void InitCanaMbox(void)
{
    /* Initialize the receive message object used for receiving CAN messages.*/
    g_stRXCANMessage.ui32MsgID = ModeRequest;
    g_stRXCANMessage.ui32MsgIDMask = 0;
    g_stRXCANMessage.ui32Flags = MSG_OBJ_RX_INT_ENABLE;
    g_stRXCANMessage.ui32MsgLen = 8;
    g_stRXCANMessage.pucMsgData = s_u8rxMsgData;

    CANMessageSet(CANA_BASE, ID_RX_ModeRequest, &g_stRXCANMessage,
                  MSG_OBJ_TYPE_RX);

    g_stRXCANMessage.ui32MsgID = LogisticRequest;
    g_stRXCANMessage.ui32MsgLen = 1;
    CANMessageSet(CANA_BASE, ID_RX_LogisticRequest, &g_stRXCANMessage, MSG_OBJ_TYPE_RX);

    g_stRXCANMessage.ui32MsgID = SecurityAccess;
    g_stRXCANMessage.ui32MsgLen = 8;
    CANMessageSet(CANA_BASE, ID_RX_SecurityAccess, &g_stRXCANMessage, MSG_OBJ_TYPE_RX);

    g_stRXCANMessage.ui32MsgID = EraseMemory;
    g_stRXCANMessage.ui32MsgLen = 1;
    CANMessageSet(CANA_BASE, ID_RX_EraseMemory, &g_stRXCANMessage, MSG_OBJ_TYPE_RX);

    g_stRXCANMessage.ui32MsgID = TransferInformation;
    g_stRXCANMessage.ui32MsgLen = 8;
    CANMessageSet(CANA_BASE, ID_RX_TransferInformation, &g_stRXCANMessage, MSG_OBJ_TYPE_RX);

    g_stRXCANMessage.ui32MsgID = TransferData;
    g_stRXCANMessage.ui32MsgLen = 8;
    CANMessageSet(CANA_BASE, ID_RX_TransferData, &g_stRXCANMessage, MSG_OBJ_TYPE_RX);

    g_stRXCANMessage.ui32MsgID = CRCRequest;
    g_stRXCANMessage.ui32MsgLen = 1;
    CANMessageSet(CANA_BASE, ID_RX_CRCRequest, &g_stRXCANMessage, MSG_OBJ_TYPE_RX);

    g_stRXCANMessage.ui32MsgID = SWVersionCheck;
    g_stRXCANMessage.ui32MsgLen = 6;
    CANMessageSet(CANA_BASE, ID_RX_SWVersionCheck, &g_stRXCANMessage, MSG_OBJ_TYPE_RX);

    s_stTXCANMessage.ui32MsgID = DIAG_SESSION;
    s_stTXCANMessage.ui32MsgIDMask = 0;
    s_stTXCANMessage.ui32Flags = 0;
    s_stTXCANMessage.ui32MsgLen = 8;
    s_stTXCANMessage.pucMsgData = s_u8txMsgData;
}

void SendDiagnosticResponse(uint8_t MemoryArea, uint8_t Config)
{
    s_stTXCANMessage.ui32MsgID = DIAG_SESSION;
    s_stTXCANMessage.ui32MsgLen = 8;

    s_u8txMsgData[0] = MemoryArea + (Config << 5);
    s_u8txMsgData[4] = (DIAGNOSTICSESSION << 6);

    CANMessageSet(CANA_BASE, ID_TX_DIAG_SESSION, &s_stTXCANMessage, MSG_OBJ_TYPE_TX);
}

void SendGenericResponse(uint8_t MemoryArea, uint8_t error)
{
    s_stTXCANMessage.ui32MsgID = GENERAL_RESP;
    s_stTXCANMessage.ui32MsgLen = 2;

    s_u8txMsgData[0] = MemoryArea;
    s_u8txMsgData[1] = error;

    CANMessageSet(CANA_BASE, ID_TX_GENERAL_RESP, &s_stTXCANMessage, MSG_OBJ_TYPE_TX);
}

void SendLogisticResponse(uint8_t MemoryArea, uint8_t* Config, uint8_t DataSize)
{
    uint8_t i;
    s_stTXCANMessage.ui32MsgID = LOGI_RESP;
    s_stTXCANMessage.ui32MsgLen = 8;

    /* clear data buffer*/
    s_u8txMsgData[0] = MemoryArea;
    for(i = 0; i < 7; i++)
    {
        if(i < DataSize)
        {
            s_u8txMsgData[i+1] = Config[i];
        }
        else
        {
            s_u8txMsgData[i+1] = 0;
        }
    }

    CANMessageSet(CANA_BASE, ID_TX_LOGI_RESP, &s_stTXCANMessage, MSG_OBJ_TYPE_TX);
}



