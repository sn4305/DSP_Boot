/*
 * Timer.c
 *
 *  Created on: 2021Äê3ÔÂ1ÈÕ
 *      Author: E9981231
 */

#include "Timer.h"
#include "F28x_Project.h"
#include "interrupt.h"

/* defined in interrupt.c */
extern volatile uint16_t u16Tick;
extern volatile uint16_t u16TimTick;
extern bool Timer_Start_Flag;
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

#pragma CODE_SECTION(TMR_Start,".TI.ramfunc");
void TMR_Start(void)
{
    Timer_Start_Flag = 1;
}

#pragma CODE_SECTION(TMR_Stop,".TI.ramfunc");
void TMR_Stop(void)
{
    Timer_Start_Flag = 0;
}

#pragma CODE_SECTION(TMR_SoftwareCounterClear,".TI.ramfunc");
void TMR_SoftwareCounterClear(void)
{
    u16TimTick = 0;
}

#pragma CODE_SECTION(TMR_SoftwareCounterGet,".TI.ramfunc");
uint16_t TMR_SoftwareCounterGet(void)
{
    return u16TimTick;
}

#pragma CODE_SECTION(TMR3_Start,".TI.ramfunc");
void TMR3_Start(void)
{
//    Timer3_Start_Flag = 1;
}

#pragma CODE_SECTION(TMR3_Stop,".TI.ramfunc");
void TMR3_Stop(void)
{
//    Timer3_Start_Flag = 0;
}

#pragma CODE_SECTION(TMR3_SoftwareCounterGet,".TI.ramfunc");
uint16_t TMR3_SoftwareCounterGet(void)
{
//    return u16Tim3Tick;
}

#pragma CODE_SECTION(TMR3_SoftwareCounterClear,".TI.ramfunc");
void TMR3_SoftwareCounterClear(void)
{
//    u16Tim3Tick = 0;
}
