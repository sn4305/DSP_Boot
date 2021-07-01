/**
 * @file Timer.h
 *
 *  Created on: 20210301
 *      Author: E9981231
 */

#ifndef USERINC_TIMER_H_
#define USERINC_TIMER_H_

#include "F28x_Project.h"

/** Timer object. */
typedef struct _TMR_OBJ_STRUCT
{
    /** Timer Elapsed */
    bool         Start_Flag;
    /** Software Counter value*/
    volatile uint16_t     count;
} TMR_OBJ;

/** Initiate Timer. Include clock, Timer INT, ISR.
 * Be called in MainBoot.
* @see MainBoot()
*********************************************************/
void Init_Timer(void);

/** Get system tick.
* @return Tick value.
*********************************************************/
uint16_t Get_SysTick(void);

/** Clear system tick.*/
void Clr_SysTick(void);

void TMR0_Start(void);
void TMR0_Stop(void);
void TMR0_SoftwareCounterClear(void);
uint16_t TMR0_SoftwareCounterGet(void);

void TMR1_Start(void);
void TMR1_Stop(void);
void TMR1_SoftwareCounterClear(void);
uint16_t TMR1_SoftwareCounterGet(void);

void TMR2_Start(void);
void TMR2_Stop(void);
void TMR2_SoftwareCounterClear(void);
uint16_t TMR2_SoftwareCounterGet(void);


#endif /* USERINC_TIMER_H_ */
