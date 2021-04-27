/**
 * @file interrupt.h
 *
 *  Created on: 20210301
 *      Author: E9981231
 */

#ifndef USERINC_INTERRUPT_H_
#define USERINC_INTERRUPT_H_

#include "F28x_Project.h"
#include "cancom.h"
#include "Timer.h"

/** Object used to restore CAN frame */
typedef struct
{
    //! The CAN message identifier used for 11 or 29 bit identifiers.
    uint32_t u32MsgID;


    //! This value is the number of bytes of data in the message object.
    uint16_t u16MsgLen;

    //! This is a pointer to the message object's data.
    volatile uint8_t *pu8MsgData;
}stCanMsgObj;

extern volatile bool g_bCAN_RX_Flag;
extern volatile CAN_CMD g_enumCAN_Command;
/* global CAN message object used to restore temporal CAN frame from canaISR() and read by MainBoot()*/
extern volatile stCanMsgObj g_stRXCANMsg;

/** Timer0 interrupt server routine*/
__interrupt void cpu_timer0_isr(void);

/** Can A interrupt server routine*/
__interrupt void canaISR(void);



#endif /* USERINC_INTERRUPT_H_ */
