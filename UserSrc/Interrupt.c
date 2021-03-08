/*
 * Interrupt.c
 *
 *  Created on: 2021Äê3ÔÂ1ÈÕ
 *      Author: E9981231
 */

#include "interrupt.h"


/* global variable, only be modified in CAN rx interrupt function and Timer.c */
volatile uint16_t u16Tick = 0;
volatile uint16_t u16TimTick = 0;
bool Timer_Start_Flag = 0;

/* global variable, only be modified in CAN rx interrupt function, and read by main state machine */
volatile bool CAN_RX_Flag = 0;
volatile CAN_CMD g_enumCAN_Command = CMD_ModeRequest;
volatile uint16_t  Can_u16errFlag = 0;
volatile uint8_t g_u8rxMsgData[8] = {0};
volatile tCANMsgObject g_RXCANMsg;

#pragma CODE_SECTION(cpu_timer0_isr,".TI.ramfunc");
#pragma CODE_SECTION(canaISR,".TI.ramfunc");

//
// cpu_timer0_isr - CPU Timer0 ISR with interrupt counter
//
__interrupt void cpu_timer0_isr(void)
{
    DINT;

    u16Tick++;  /* for test */

    if(Timer_Start_Flag)
    {/* need consider u16TimTick overflow when use this*/
        u16TimTick++;
    }
   //
   // Acknowledge this interrupt to receive more interrupts from group 1
   //
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
   EINT;
}

//
// CAN A ISR - The interrupt service routine called when a CAN interrupt is
//             triggered on CAN module A.
//
__interrupt void
canaISR(void)
{
    uint32_t status0, status1;
    uint8_t idx = 0;

    //Enable cpu_Timer0 interrupt nest
   PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
   IER |= M_INT1;
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
   EINT;

    //
    // Read the CAN-A interrupt status to find the cause of the interrupt
    //
    status0 = CANIntStatus(CANA_BASE, CAN_INT_STS_CAUSE);

    //
    // If the cause is a controller status interrupt, then get the status
    //
    if(status0 == CAN_INT_INT0ID_STATUS)
    {
        //
        // Read the controller status.  This will return a field of status
        // error bits that can indicate various errors.  Error processing
        // is not done in this example for simplicity.  Refer to the
        // API documentation for details about the error status bits.
        // The act of reading this status will clear the interrupt.
        //
        status1 = CANStatusGet(CANA_BASE, CAN_STS_CONTROL);

        //
        // Check to see if an error occurred.
        //
        if(((status1  & ~(CAN_ES_TXOK | CAN_ES_RXOK)) != 7) &&
           ((status1  & ~(CAN_ES_TXOK | CAN_ES_RXOK)) != 0))
        {
            //
            // Set a flag to indicate some errors may have occurred.
            //
            Can_u16errFlag = 1;
        }
    }
    //
    // Check if the cause is the CAN-A receive message object 1
    //
    else if((status0 >= ID_RX_OBJ_START) && (status0 <= ID_RX_OBJ_END))
    {
        CAN_RX_Flag = 1;
        //
        // Get the received message
        //
        CANMessageGet(CANA_BASE, status0, &sRXCANMessage, true);
        /* copy CAN rxmsg to front end buffer which is used in main state machine*/
        for(idx = 0; idx < 8; idx++)
        {
            g_u8rxMsgData[idx] = rxMsgData[idx];
        }
        memcpy((void *)&g_RXCANMsg, (const void *)&sRXCANMessage, sizeof(tCANMsgObject));

        switch(status0) //Message Obj ID
        {  /* judge CANid to see which CAN Command is received*/
           case ID_RX_ModeRequest:
               g_enumCAN_Command = CMD_ModeRequest;
               break;
           case ID_RX_LogisticRequest:
               g_enumCAN_Command = CMD_LogisticRequest;
               break;
           case ID_RX_SecurityAccess:
               g_enumCAN_Command = CMD_SecurityAccess;
               break;
           case ID_RX_SWVersionCheck:
               g_enumCAN_Command = CMD_SWVersionCheck;
               break;
           case ID_RX_EraseMemory:
               g_enumCAN_Command = CMD_EraseMemory;
               break;
           case ID_RX_TransferInformation:
               g_enumCAN_Command = CMD_TransferInformation;
               break;
           case ID_RX_TransferData:
               g_enumCAN_Command = CMD_TransferData;
               break;
           case ID_RX_CRCRequest:
               g_enumCAN_Command = CMD_CRCRequest;
               break;
        }

#ifdef DEMOBOARD
        if(0 == rxMsgData[0])
            //turn off BLUE LED
            GPIO_WritePin(13, 1);
        else if(1 == rxMsgData[0])
            //turn on BLUE LED
            GPIO_WritePin(13, 0);

#endif

        //
        // Getting to this point means that the RX interrupt occurred on
        // message object 1, and the message RX is complete.  Clear the
        // message object interrupt.
        //
        CANIntClear(CANA_BASE, status0);

        //
        // Since the message was received, clear any error flags.
        //
        Can_u16errFlag = 0;
    }
    //
    // If something unexpected caused the interrupt, this would handle it.
    //
    else
    {
        //
        // Spurious interrupt handling can go here.
        //
    }

    //
    // Clear the global interrupt flag for the CAN interrupt line
    //
    CANGlobalIntClear(CANA_BASE, CAN_GLB_INT_CANINT0);

    //
    // Acknowledge this interrupt located in group 9
    //
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
}
