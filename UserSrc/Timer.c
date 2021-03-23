/*
 * Timer.c
 *
 *  Created on: 2021Äê3ÔÂ1ÈÕ
 *      Author: E9981231
 */

#include <Interrupt.h>
#include "Timer.h"
#include "F28x_Project.h"

/* defined in interrupt.c */
extern volatile uint16_t u16Tick;

/* used for 5s boot overtime */
TMR_OBJ tmr1_obj;
/* used for transData and flash overtime */
TMR_OBJ tmr2_obj;

static void EnableDog(void);

//
// Init_Timer - Initialize cpu timer
//
void Init_Timer(void)
{
    EALLOW;  // This is needed to write to EALLOW protected registers
    PieVectTable.TIMER0_INT = &cpu_timer0_isr;
//    PieVectTable.TIMER1_INT = &cpu_timer1_isr;
//    PieVectTable.TIMER2_INT = &cpu_timer2_isr;
    EDIS;    // This is needed to disable write to EALLOW protected registers

    InitCpuTimers();

    //
    // Configure CPU-Timer 0 to interrupt every 10ms:
    // 200MHz CPU Freq, 10ms period.
    //
    ConfigCpuTimer(&CpuTimer0, 200, 10000);
//    ConfigCpuTimer(&CpuTimer1, 200, 1000000);
//    ConfigCpuTimer(&CpuTimer2, 200, 1000000);

    //
    // To ensure precise timing, use write-only instructions to write to the
    // entire register. Therefore, if any of the configuration bits are changed in
    // ConfigCpuTimer and InitCpuTimers (in F2837xS_cputimervars.h), the below
    // settings must also be updated.
    //
    CpuTimer0Regs.TCR.all = 0x4000;
//        CpuTimer1Regs.TCR.all = 0x4000;
//        CpuTimer2Regs.TCR.all = 0x4000;
//
// Enable CPU int1 which is connected to CPU-Timer 0, CPU int13
// which is connected to CPU-Timer 1, and CPU int 14, which is connected
// to CPU-Timer 2:
//
    IER |= M_INT1;
//    IER |= M_INT13;
//    IER |= M_INT14;

//
// Enable TINT0 in the PIE: Group 1 interrupt 7
//
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

#ifndef __IS_DEBUG
    /*enable watchdog*/
    EnableDog();
#endif
}

#pragma CODE_SECTION(Get_SysTick,".TI.ramfunc");
uint16_t Get_SysTick(void)
{
    return u16Tick;
}

#pragma CODE_SECTION(Clr_SysTick,".TI.ramfunc");
void Clr_SysTick(void)
{
    u16Tick = 0;
}

#pragma CODE_SECTION(TMR1_Start,".TI.ramfunc");
void TMR1_Start(void)
{
    tmr1_obj.Start_Flag = true;
}

#pragma CODE_SECTION(TMR1_Stop,".TI.ramfunc");
void TMR1_Stop(void)
{
    tmr1_obj.Start_Flag = false;
}

#pragma CODE_SECTION(TMR1_SoftwareCounterClear,".TI.ramfunc");
void TMR1_SoftwareCounterClear(void)
{
    tmr1_obj.count = 0;
}

#pragma CODE_SECTION(TMR1_SoftwareCounterGet,".TI.ramfunc");
uint16_t TMR1_SoftwareCounterGet(void)
{
    return tmr1_obj.count;
}

#pragma CODE_SECTION(TMR2_Start,".TI.ramfunc");
void TMR2_Start(void)
{
    tmr2_obj.Start_Flag = true;
}

#pragma CODE_SECTION(TMR2_Stop,".TI.ramfunc");
void TMR2_Stop(void)
{
    tmr2_obj.Start_Flag = false;
}

#pragma CODE_SECTION(TMR2_SoftwareCounterGet,".TI.ramfunc");
uint16_t TMR2_SoftwareCounterGet(void)
{
    return tmr2_obj.count;
}

#pragma CODE_SECTION(TMR2_SoftwareCounterClear,".TI.ramfunc");
void TMR2_SoftwareCounterClear(void)
{
    tmr2_obj.count = 0;
}

static inline void EnableDog(void)
{
//
// Enable the watchdog, 000 WDCLK = INTOSC1/512/1, 1tick period: 51.2us, overtime:51.2us*256 = 13ms
//
    EALLOW;
    WdRegs.WDCR.all = 0x0028;
    EDIS;
}
