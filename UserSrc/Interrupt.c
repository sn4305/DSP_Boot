/*
 * Interrupt.c
 *
 *  Created on: 2021Äê3ÔÂ1ÈÕ
 *      Author: E9981231
 */

#include "interrupt.h"
#include "cancom.h"


static volatile uint16_t u16Tick = 0;
volatile bool CAN_RX_Flag = 0;
volatile uint16_t  Can_u16errFlag = 0;

#pragma CODE_SECTION(cpu_timer0_isr,".TI.ramfunc");
#pragma CODE_SECTION(Get_SysTick,".TI.ramfunc");
#pragma CODE_SECTION(Clr_SysTick,".TI.ramfunc");
#pragma CODE_SECTION(canaISR,".TI.ramfunc");

//
// cpu_timer0_isr - CPU Timer0 ISR with interrupt counter
//
__interrupt void cpu_timer0_isr(void)
{
    DINT;

    u16Tick++;
   //
   // Acknowledge this interrupt to receive more interrupts from group 1
   //
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
   EINT;
}

uint16_t Get_SysTick(void)
{
    return u16Tick;
}

void Clr_SysTick(void)
{
    u16Tick = 0;
}

//
// CAN A ISR - The interrupt service routine called when a CAN interrupt is
//             triggered on CAN module A.
//
__interrupt void
canaISR(void)
{
    __asm("    ESTOP0");

    uint32_t status0, status1;

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

        switch(status0) //Message Obj ID
        {
           case ID_RX_ModeRequest:

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
