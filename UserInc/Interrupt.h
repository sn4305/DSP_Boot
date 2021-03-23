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
#include "ErrHandler.h"
#include "Timer.h"

extern volatile bool CAN_RX_Flag;
extern volatile CAN_CMD g_enumCAN_Command;
extern volatile tCANMsgObject g_RXCANMsg;
extern volatile uint8_t g_u8rxMsgData[8];

__interrupt void cpu_timer0_isr(void);
__interrupt void canaISR(void);



#endif /* USERINC_INTERRUPT_H_ */
