
#include "main.h"

//
// Function Prototypes
//
void Example_Error(Fapi_StatusType status);
void Init_Flash_Sectors(void);
void Init_Timer(void);

BootMachineStates State = State_TRANSITION;


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
//    InitFlash_Bank1();
//    InitFlash_Bank0();
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
                    StartApplication();
                }
                else if(UPDATE_APP_RQST == u32UpdataFlag)
                {/* received boot request from APP, jump to boot*/
                    State = State_BOOT; // <====== GO BOOT
                }
                else
                {
                    State = State_DEFAULT; // <====== GO DEFAULT
                }
                break;
    //  ||========================================================||
    //  ||                 STATE: DEFAULT                         ||
    //  ||========================================================||
            case State_DEFAULT:
#ifdef DEMOBOARD
                //toggle LED
                if(CAN_RX_Flag)
                {
                    CAN_RX_Flag = 0;
                    SendModeResponse(5, 0);
                }
#endif
                //ClrWdt();
                break;
    //  ||========================================================||
    //  ||                 STATE: BOOT                            ||
    //  ||========================================================||
            case State_BOOT:
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
