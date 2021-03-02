/*
 * Timer.h
 *
 *  Created on: 2021Äê3ÔÂ1ÈÕ
 *      Author: E9981231
 */

#ifndef USERINC_TIMER_H_
#define USERINC_TIMER_H_

#include "F28x_Project.h"

void Init_Timer(void);
uint16_t Get_SysTick(void);
void Clr_SysTick(void);

void TMR_Start(void);
void TMR_Stop(void);
void TMR_SoftwareCounterClear(void);
uint16_t TMR_SoftwareCounterGet(void);

void TMR3_Start(void);
void TMR3_Stop(void);
void TMR3_SoftwareCounterClear(void);
uint16_t TMR3_SoftwareCounterGet(void);

#endif /* USERINC_TIMER_H_ */
