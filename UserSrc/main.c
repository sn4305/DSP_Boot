/*******************************************************************
File name:   <main.c>
Purpose :    <main file>
Copyright Notice:
All source code and data contained in this file is Proprietary and
Confidential to Eaton, and must not be reproduced, transmitted, or
disclosed; in whole or in part, without the express written permission of Eaton.
Copyright 2011 - Eaton, All Rights Reserved.

Author              Date               Ver#        Description (CR#)
Dongdong Yang       20210225           00          Init for Base project 2nd DSP(28377s) Bootloader
 append log after baseline, especially for milestone or release version, no log is allowed for minor modification
******************************************************************
(***).
*/

#include "main.h"

//
// Function Prototypes
//
void Example_Error(Fapi_StatusType status);
void Init_Flash_Sectors(void);
void Init_Timer(void);

BootMachineStates State = State_TRANSITION;
uint16_t s_u16Configuration = 0; //OBC address configuration

/**
 * main.c
 */
void main(void)
{
//    uint32 FlagAppli;
    //
    // Step 1. Initialize System Control:
    // Enable Peripheral Clocks
    // This function is found in the F2837xS_SysCtrl.c file.
    //
    InitSysCtrl(); //PLL activates

    //
    // Step 2. Initialize GPIO:
    // This example function is found in the F2837xS_Gpio.c file and
    // illustrates how to set the GPIO to it's default state.
    //
    InitGpio();

#ifdef DEMOBOARD
    //LED init
    GPIO_SetupPinMux(BLINKY_LED_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(BLINKY_LED_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(CAN_LED_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(CAN_LED_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
#endif
    //
    // Step 3. Clear all interrupts and initialize PIE vector table:
    // Disable CPU interrupts
    //
    DINT;

    //
    // Initialize the PIE control registers to their default state.
    // The default state is all PIE interrupts disabled and flags
    // are cleared.
    // This function is found in the F2837xS_PieCtrl.c file.
    //
    InitPieCtrl();

    //
    // Disable CPU interrupts and clear all CPU interrupt flags:
    //
    IER = 0x0000;
    IFR = 0x0000;

    //
    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    // This will populate the entire table, even if the interrupt
    // is not used in this example.  This is useful for debug purposes.
    //
    InitPieVectTable();

    //
    // Initialize the CANA.
    //
    InitCana();

    Init_Timer();

    //
    // Initialize the Flash.
    //
    Init_Flash_Sectors();

    //
    // Gain pump semaphore for Bank0.
    // User may need to do this for Bank1 if programming Bank1.
    //
    SeizeFlashPump_Bank0();

    EINT;
//    ERTM;  // Enable Global realtime interrupt DBGM

    //
    // Start CAN module A operations
    //
    CANEnable(CANA_BASE);

//    u32UpdataFlag = 0xC0DEFEED;

    State = State_TRANSITION;

#ifdef DEMOBOARD
    State = State_BOOT;
#endif
    //  ||========================================================||
    //  ||                 Start of State Machine                 ||
    //  ||========================================================||
    while (1) {
        switch (State) {

    //  ||========================================================||
    //  ||                 STATE: TRANSITION                      ||
    //  ||========================================================||
            case State_TRANSITION:
                if( UPDATE_APP_RQST != u32UpdataFlag && APP_VALID ==  APP_VALID_FLAG)
                {/* FlagAppli is valid, jump to APP*/
                    TMR_Stop();
                    TMR_SoftwareCounterClear();
                    StartApplication();
                }
                else if(UPDATE_APP_RQST == u32UpdataFlag)
                {/* received boot request from APP, jump to boot*/
                    TMR_Start();
                    TMR_SoftwareCounterClear();
                    SendDiagnosticResponse(BOOT_MODE, s_u16Configuration);
                    State = State_BOOT; // <====== GO BOOT
                }
                else
                {
                    SendDiagnosticResponse(DEFAULT_MODE, s_u16Configuration);
                    State = State_DEFAULT; // <====== GO DEFAULT
                }
                break;
    //  ||========================================================||
    //  ||                 STATE: DEFAULT                         ||
    //  ||========================================================||
            case State_DEFAULT:
                //toggle LED
                if(CAN_RX_Flag)
                {
                    Clr_CanRxFlag();
                    if(g_enumCAN_Command == CMD_ModeRequest )
                    {
                        if((g_u8rxMsgData[4] & 0x07) == BOOT_MODE)
                        {
                            u32UpdataFlag = UPDATE_APP_RQST;
                            SendDiagnosticResponse(BOOT_MODE, s_u16Configuration);
                            State = State_BOOT; // <====== GO BOOT
                        }
                        else if((g_u8rxMsgData[4] & 0x07) == DEFAULT_MODE)
                        {
                            u32UpdataFlag = 0;
                            RESET();
                        }
                    }
                }

                //ClrWdt();
                break;
    //  ||========================================================||
    //  ||                 STATE: BOOT                            ||
    //  ||========================================================||
            case State_BOOT:
                if (TMR_SoftwareCounterGet() >= 500)
                { /*500 * 10ms = 5000ms */
                    /*5s Timeout*/
                    u32UpdataFlag = 0;
                    TMR_SoftwareCounterClear();
                    RESET();
                }
                /*Wait for message*/
                if(CAN_RX_Flag)
                {
                    Clr_CanRxFlag();
                    TMR_SoftwareCounterClear();
                    uint8_t error = NO_ERROR;
                    switch(g_enumCAN_Command)
                    {
                        case CMD_ModeRequest:
                            if((g_u8rxMsgData[4] & 0x07) == BOOT_MODE)
                            {/* Enter boot mode request : Send positive Response
                             * No Action*/
                                SendDiagnosticResponse(BOOT_MODE, s_u16Configuration);
                            }
                            else if ((g_u8rxMsgData[4] & 0x07) == DEFAULT_MODE)
                            {
                                /* Go to Default Mode : send response and Reset*/
                                SendDiagnosticResponse(DEFAULT_MODE, s_u16Configuration);
                                RESET();
                            }
                            else
                            {
                                /* Do nothing */
                            }
                            break;
                        case CMD_LogisticRequest:
                            /* Logistic Request*/
                            error = IsLogisticValid(g_RXCANMsg);
                            if (error != 0)
                            {
                                /* Error case */
                                SendGenericResponse(g_u8rxMsgData[0] & 0xE, error);
                            }
                            else
                            {
                                /* Send response to request */
                                LogiticRequestHandle(g_u8rxMsgData[0]);
                            }
                            break;
                        case CMD_SecurityAccess:
                            break;
                        case CMD_EraseMemory:
#ifdef DEMOBOARD
                            WriteLogisticInfo();

#endif
                            break;
                        case CMD_TransferInformation:
                            break;
                        case CMD_TransferData:
                            break;
                        case CMD_CRCRequest:
                            break;
                        default:
                            /* Do Nothing*/
                            break;
                    }
                }/* If CAN frame is received*/
                break;

            default:
                State = State_TRANSITION;
                break;
        } // switch (MyCurrentState)

#ifdef DEMOBOARD
        //toggle LED
        if(0 == Get_SysTick()%8)
            GPIO_WritePin(BLINKY_LED_GPIO, 0);
        else
            GPIO_WritePin(BLINKY_LED_GPIO, 1);



#endif


    //  ||========================================================||
    //  ||                   End of State Machine                 ||
    //  ||========================================================||

    } // while (1)

}

//
// End of file
//
