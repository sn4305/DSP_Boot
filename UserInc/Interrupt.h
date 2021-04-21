/*
 * interrupt.h
 *
 *  Created on: 2021Äê3ÔÂ1ÈÕ
 *      Author: E9981231
 */

#ifndef USERINC_INTERRUPT_H_
#define USERINC_INTERRUPT_H_

#include "F28x_Project.h"
#include "cancom.h"
#include "Timer.h"

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
extern volatile stCanMsgObj g_stRXCANMsg;  /*global CAN message used in main.c*/

__interrupt void cpu_timer0_isr(void);
__interrupt void canaISR(void);



#endif /* USERINC_INTERRUPT_H_ */
