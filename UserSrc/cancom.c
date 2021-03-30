/*
 * can.c
 *
 *  Created on: 2021Äê2ÔÂ26ÈÕ
 *      Author: E9981231
 */

#include "cancom.h"

//
// Function Prototypes
//
void InitCanaGpio(void);
void InitCanaMbox(void);

unsigned char txMsgData[8];
unsigned char rxMsgData[8];
tCANMsgObject sTXCANMessage;
tCANMsgObject sRXCANMessage;

extern __interrupt void canaISR(void);

void InitCana(void)
{
    InitCanaGpio();

    //
    // Initialize the CAN controller
    //
    CANInit(CANA_BASE);

    //
    // Setup CAN to be clocked off the PLL output clock
    //
    CANClkSourceSelect(CANA_BASE, 0);   // 500kHz CAN-Clock

    //
    // Set up the CAN bus bit rate to 500kHz for CANA module
    // This function sets up the CAN bus timing for a nominal configuration.
    // You can achieve more control over the CAN bus timing by using the
    // function CANBitTimingSet() instead of this one, if needed.
    // Additionally, consult the device data sheet for more information about
    // the CAN module clocking.
    //
    CANBitRateSet(CANA_BASE, 200000000, 500000);

    //
    // Enable interrupts on the CAN A peripheral.
    //
    CANIntEnable(CANA_BASE, CAN_INT_MASTER | CAN_INT_ERROR | CAN_INT_STATUS);

    //
    // Interrupts that are used in this example are re-mapped to
    // ISR functions found within this file.
    // This registers the interrupt handler in PIE vector table.
    //
    EALLOW;
    PieVectTable.CANA0_INT = canaISR;
    EDIS;

    //
    // Enable the CAN-A0 interrupt on the processor (PIE).
    //
    PieCtrlRegs.PIEIER9.bit.INTx5 = 1;
    IER |= M_INT9;

    InitCanaMbox();

    //
    // Enable the CAN-A interrupt signal
    //
    CANGlobalIntEnable(CANA_BASE, CAN_GLB_INT_CANINT0);

    //
    // Clear the global interrupt flag for the CAN interrupt line
    //
    CANGlobalIntClear(CANA_BASE, CAN_GLB_INT_CANINT0);

    //
    // Acknowledge this interrupt located in group 9
    //
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
    //
    // Start CAN module A operations
    //
//    CANEnable(CANA_BASE);
}

void InitCanaGpio(void)
{
#ifdef DEMOBOARD
    GPIO_SetupPinMux(70, GPIO_MUX_CPU1, 5); //GPIO70 -  CANRXA
    GPIO_SetupPinOptions(70, GPIO_INPUT, GPIO_ASYNC);
    GPIO_SetupPinMux(71, GPIO_MUX_CPU1, 5); //GPIO71 - CANTXA
    GPIO_SetupPinOptions(71, GPIO_OUTPUT, GPIO_PUSHPULL);
#else

#endif
}

void InitCanaMbox(void)
{
    //
    // Initialize the receive message object used for receiving CAN messages.
    // Message Object Parameters:
    //      Message Identifier: 0x5555
    //      Message ID Mask: 0x0
    //      Message Object Flags: Receive Interrupt
    //      Message Data Length: 4 Bytes
    //      Message Receive data: rxMsgData
    //
    sRXCANMessage.ui32MsgID = ModeRequest;
    sRXCANMessage.ui32MsgIDMask = 0;
    sRXCANMessage.ui32Flags = MSG_OBJ_RX_INT_ENABLE;
    sRXCANMessage.ui32MsgLen = 8;
    sRXCANMessage.pucMsgData = rxMsgData;

    CANMessageSet(CANA_BASE, ID_RX_ModeRequest, &sRXCANMessage,
                  MSG_OBJ_TYPE_RX);

    sRXCANMessage.ui32MsgID = LogisticRequest;
    sRXCANMessage.ui32MsgLen = 1;
    CANMessageSet(CANA_BASE, ID_RX_LogisticRequest, &sRXCANMessage, MSG_OBJ_TYPE_RX);

    sRXCANMessage.ui32MsgID = SecurityAccess;
    sRXCANMessage.ui32MsgLen = 8;
    CANMessageSet(CANA_BASE, ID_RX_SecurityAccess, &sRXCANMessage, MSG_OBJ_TYPE_RX);

    sRXCANMessage.ui32MsgID = EraseMemory;
    sRXCANMessage.ui32MsgLen = 1;
    CANMessageSet(CANA_BASE, ID_RX_EraseMemory, &sRXCANMessage, MSG_OBJ_TYPE_RX);

    sRXCANMessage.ui32MsgID = TransferInformation;
    sRXCANMessage.ui32MsgLen = 8;
    CANMessageSet(CANA_BASE, ID_RX_TransferInformation, &sRXCANMessage, MSG_OBJ_TYPE_RX);

    sRXCANMessage.ui32MsgID = TransferData;
    sRXCANMessage.ui32MsgLen = 8;
    CANMessageSet(CANA_BASE, ID_RX_TransferData, &sRXCANMessage, MSG_OBJ_TYPE_RX);

    sRXCANMessage.ui32MsgID = CRCRequest;
    sRXCANMessage.ui32MsgLen = 1;
    CANMessageSet(CANA_BASE, ID_RX_CRCRequest, &sRXCANMessage, MSG_OBJ_TYPE_RX);

    sRXCANMessage.ui32MsgID = SWVersionCheck;
    sRXCANMessage.ui32MsgLen = 6;
    CANMessageSet(CANA_BASE, ID_RX_SWVersionCheck, &sRXCANMessage, MSG_OBJ_TYPE_RX);

    sTXCANMessage.ui32MsgID = DIAG_SESSION;
    sTXCANMessage.ui32MsgIDMask = 0;
    sTXCANMessage.ui32Flags = 0;
    sTXCANMessage.ui32MsgLen = 8;
    sTXCANMessage.pucMsgData = txMsgData;
}

void SendDiagnosticResponse(uint8_t MemoryArea, uint8_t Config)
{
    sTXCANMessage.ui32MsgID = DIAG_SESSION;
    sTXCANMessage.ui32MsgLen = 8;

    txMsgData[0] = MemoryArea + (Config << 5);
    txMsgData[4] = (DIAGNOSTICSESSION << 6);

    CANMessageSet(CANA_BASE, ID_TX_DIAG_SESSION, &sTXCANMessage, MSG_OBJ_TYPE_TX);
}

void SendGenericResponse(uint8_t MemoryArea, uint8_t error)
{
    sTXCANMessage.ui32MsgID = GENERAL_RESP;
    sTXCANMessage.ui32MsgLen = 2;

    txMsgData[0] = MemoryArea;
    txMsgData[1] = error;

    CANMessageSet(CANA_BASE, ID_TX_GENERAL_RESP, &sTXCANMessage, MSG_OBJ_TYPE_TX);
}

void SendLogisticResponse(uint8_t MemoryArea, uint8_t* Config, uint8_t DataSize)
{
    uint8_t i;
    sTXCANMessage.ui32MsgID = LOGI_RESP;
    sTXCANMessage.ui32MsgLen = 8;

    /* clear data buffer*/
    txMsgData[0] = MemoryArea;
    for (i = 0; i < 7; i++)
    {
        if(i < DataSize)
        {
            txMsgData[i+1] = Config[i];
        }
        else
        {
            txMsgData[i+1] = 0;
        }
    }

    CANMessageSet(CANA_BASE, ID_TX_LOGI_RESP, &sTXCANMessage, MSG_OBJ_TYPE_TX);
}



