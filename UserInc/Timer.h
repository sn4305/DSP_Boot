/*
 * Timer.h
 *
 *  Created on: 2021Äê3ÔÂ1ÈÕ
 *      Author: E9981231
 */

#ifndef USERINC_TIMER_H_
#define USERINC_TIMER_H_

#include "F28x_Project.h"

typedef struct _TMR_OBJ_STRUCT
{
    /* Timer Elapsed */
    bool         Start_Flag;
    /*Software Counter value*/
    uint16_t     count;

} TMR_OBJ;

void Init_Timer(void);
uint16_t Get_SysTick(void);
void Clr_SysTick(void);

void TMR1_Start(void);
void TMR1_Stop(void);
void TMR1_SoftwareCounterClear(void);
uint16_t TMR1_SoftwareCounterGet(void);

void TMR2_Start(void);
void TMR2_Stop(void);
void TMR2_SoftwareCounterClear(void);
uint16_t TMR2_SoftwareCounterGet(void);


#endif /* USERINC_TIMER_H_ */
