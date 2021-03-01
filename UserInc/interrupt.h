/*
 * interrupt.h
 *
 *  Created on: 2021Äê3ÔÂ1ÈÕ
 *      Author: E9981231
 */

#ifndef USERINC_INTERRUPT_H_
#define USERINC_INTERRUPT_H_

#include "F28x_Project.h"



extern volatile bool CAN_RX_Flag;

__interrupt void cpu_timer0_isr(void);
__interrupt void canaISR(void);

uint16_t Get_SysTick(void);
void Clr_SysTick(void);

#endif /* USERINC_INTERRUPT_H_ */
