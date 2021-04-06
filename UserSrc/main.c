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

BootMachineStates State = State_TRANSITION;
static MyBootSys BootStatus;
uint16_t s_u16Configuration = 0; //OBC address configuration
St_BootFlag st_BootFlag = {false, false, false, false};
pSt_BootFlag ptr_st_BootFlag = &st_BootFlag;
uint8_t RecvBuf[MAX_BLOCK_SIZE + CRC_LENGTH] = {0};
St_TransData st_TransData = {0, RecvBuf, 0};
St_TransDataInfo st_TransDataInfo = {0, 0, 0, 0, 0, &st_TransData};

/* function declaration */
static void TreatData(uint8_t* Received_Message, St_TransDataInfo *pSt_TransDataInfo);
static void IdentifyBoot(MyBootSys* Info);
static void Init_TransParam(St_TransDataInfo *pTran);
static void Init_BootFlag(void);

/* ******************************************************
  ##define global Macro in different project build configuration
 *Open CCS project propeties->Build->C2000 Compiler->predefined Symbols;
 *Add macro in Pre-define NAME;
*********************************************************/
#ifndef __IS_STANDALONE
/* function declaration */
uint32_t MainBoot(void);

#pragma CODE_SECTION(main,".preboot");
uint32_t main(void)
{//Pre boot sequence
   if (boot_even_flag == BootEvenValid)
   {
       if (u40BootVersion[0] == 0x0101 &&
           u40BootVersion[1] == 0x0301 &&
           u40BootVersion[2] == 0x00FF )
       {
           /* if it is init Boot0, then jump to Mainboot*/
           MainBoot();
       }
       else
       {
           /* if it is reProgrammed Boot0, then jump to fix Boot0 start address*/
           StartBootEven();
       }
   }
   else if(boot_odd_flag == BootOddValid)
   {
       StartBootOdd();
   }
   else
   {
       RESET();
   }
   return 1;

}
#endif



/**
 * main.c
 */
#ifndef __IS_STANDALONE
uint32_t MainBoot(void)
#else
uint32_t main(void)
#endif
{
//    uint32 FlagAppli;
    //
    // Step 1. Initialize System Control:
    // Enable Peripheral Clocks
    // This function is found in the F2837xS_SysCtrl.c file.
    //
    InitSysCtrl(); //PLL activates

    //
    //  Unlock CSM
    //
    //  If the API functions are going to run in unsecured RAM
    //  then the CSM must be unlocked in order for the flash
    //  API functions to access the flash.
    //  If the flash API functions are executed from secure memory
    //  then this step is not required.
    //
    CsmUnlock();

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

    EINT;
//    ERTM;  // Enable Global realtime interrupt DBGM

    //
    // Start CAN module A operations
    //
    CANEnable(CANA_BASE);

    ServiceDog();

    State = State_TRANSITION;
    Init_BootFlag();
    Init_TransParam(&st_TransDataInfo);
    IdentifyBoot(&BootStatus);

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
#ifndef __IS_DEBUG
                    TMR1_Stop();
                    TMR1_SoftwareCounterClear();
#endif
                    DisableDog();
                    DELAY_US(200000L);
                    StartApplication();
                }
                else if(UPDATE_APP_RQST == u32UpdataFlag)
                {/* received boot request from APP, jump to boot*/
                    TMR1_Start();
                    TMR1_SoftwareCounterClear();
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
                ServiceDog();
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
                            DisableDog();
                            DELAY_US(200000L);
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
                if (TMR1_SoftwareCounterGet() >= 500)
                { /*500 * 10ms = 5000ms */
                    /*5s Timeout*/
                    u32UpdataFlag = 0;
                    Init_BootFlag();
                    DINT;
                    TMR1_SoftwareCounterClear();
                    DisableDog();
                    DELAY_US(200000L);
                    RESET();
                }
                ServiceDog();

                /*Wait for message*/
                if(CAN_RX_Flag)
                {
                    Clr_CanRxFlag();
                    TMR1_SoftwareCounterClear();
                    uint8_t error = NO_ERROR;
                    switch(g_enumCAN_Command)
                    {
                        case CMD_ModeRequest:
                            /* Mode Request*/
                            error = IsRequestValid(g_RXCANMsg);
                            if (error)
                            {
                                SendGenericResponse(MEMORY_AREA, error);
                            }
                            else
                            {
                                if((g_u8rxMsgData[4] & 0x07) == BOOT_MODE)
                                {/* Enter boot mode request : Send positive Response
                                 * No Action*/
                                    Init_TransParam(&st_TransDataInfo);
                                    SendDiagnosticResponse(BOOT_MODE, s_u16Configuration);
                                }
                                else if((g_u8rxMsgData[4] & 0x07) == DEFAULT_MODE)
                                {
                                    /* Reset*/
                                    u32UpdataFlag = 0;
                                    Init_BootFlag();
                                    DINT;
                                    DisableDog();
                                    DELAY_US(200000L);
                                    RESET();
                                }
                                else
                                {
                                    /* Do nothing */
                                }
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

                        case CMD_SWVersionCheck:
                            error = IsSWVersionCheckValid(g_RXCANMsg);
                            if (error)
                            {
                                SendGenericResponse(MEMORY_AREA, error);
                            }
                            else
                            {
                                SWVersionComparetHandle(g_RXCANMsg, BootStatus, ptr_st_BootFlag);
                            }
                            break;

                        case CMD_SecurityAccess:
                            error = IsSecurityValid(g_RXCANMsg);
                            if ((g_u8rxMsgData[0] & 0xF0) == MEMORY_AREA || g_u8rxMsgData[0] == (MEMORY_AREA | 4))
                            {
                                if(!error)
                                {
                                    /* Security successfully unlocked*/
                                    st_BootFlag.ucSecurityUnlocked = true;
                                }
                                else
                                {
                                    /* Security successfully unlocked*/
                                    st_BootFlag.ucSecurityUnlocked = false;
                                }
                                SendGenericResponse(MEMORY_AREA, error);
                            }
                            else
                            {
                                /* Message for another OBC/DCDC device */
                            }
                            break;

                        case CMD_EraseMemory:
                            error = IsEraseValid(g_RXCANMsg, st_BootFlag.ucSecurityUnlocked);
                            if(error)
                            {
                                /* Error case : send NOK*/
                                SendGenericResponse(MEMORY_AREA, error);
                            }
                            else
                            {
                                if((g_u8rxMsgData[0] & 0xF0) == MEMORY_AREA)
                                {
                                    EraseFlash(g_u8rxMsgData[0], BootStatus, ptr_st_BootFlag);
                                }
                                else
                                {
                                    /* Message for another component : do nothing*/
                                }
                            }
                            break;

                        case CMD_TransferInformation:
#ifdef __IS_DEBUG
                            /* Start timer for timeout*/
                            TMR2_Start();
                            /* Initialize counter*/
                            TMR2_SoftwareCounterClear();
#endif
                            error = IsTransferInfoValid(g_RXCANMsg, &st_TransDataInfo);
                            if (error)
                            {
                                /* Send error message*/
                                SendGenericResponse(MEMORY_AREA, error);
                            }
                            else
                            {
                                if (MEMORY_AREA == (g_u8rxMsgData[0] & 0xF0))
                                {
                                    /* Correct Memory area*/
                                    if (g_u8rxMsgData[1] != (uint8_t) (st_TransDataInfo.BSC + 1))
                                    {
                                        /* Problem in BSC*/
                                        SendGenericResponse(MEMORY_AREA, WRONG_REQUEST_FORMAT);
                                    }
                                    else
                                    {
                                        /* Store information from frame, prepare for data reception*/
                                        st_TransDataInfo.BSC++;
                                        st_TransDataInfo.ptr_St_Data->SN = 0;
                                        st_TransDataInfo.ptr_St_Data->RecvDataIdx = 0;
                                        st_TransDataInfo.ValidInfo = true;
                                        st_TransDataInfo.MemArea = g_u8rxMsgData[0];
                                    }
                                }
                                else
                                {
                                    /* Request for other hardware
                                     * Do Nothing */
                                }
                            }
                            break;

                        case CMD_TransferData:
                            if (MEMORY_AREA == (st_TransDataInfo.MemArea & 0xF0))
                            {
                                /*Request for this board*/
                                error = IsTransferDataValid(g_RXCANMsg, &st_TransDataInfo);
                                if (error)
                                {
                                    if (error == SAME_SN)
                                    {
                                        /* Same sequence number as previous frame: ignore the frame*/
                                        TMR2_SoftwareCounterClear();
                                    }
                                    else
                                    {
                                        st_TransDataInfo.ValidInfo = false;
                                        st_TransDataInfo.ptr_St_Data->SN = 0;
                                        SendGenericResponse(MEMORY_AREA, error);
                                    }
                                }
                                else if(MEMORY_AREA == st_TransDataInfo.MemArea || (st_TransDataInfo.MemArea & 0x0F) == 4)
                                {
                                    /* Reset timeout counter*/
                                    TMR2_SoftwareCounterClear();
                                    st_TransDataInfo.ptr_St_Data->SN++;
                                    /* Receive data*/
                                    TreatData(g_u8rxMsgData, &st_TransDataInfo);
                                }
                                else
                                {
                                    /*Logistic information*/
                                    if ((st_TransDataInfo.MemArea & 0x0F) == 1 && g_RXCANMsg.ui32MsgLen != HW_VERSION_SIZE + 1)
                                    {
                                        /* Transfer Data for HW version is wrong*/
                                        SendGenericResponse(MEMORY_AREA, WRONG_REQUEST_FORMAT);
                                    }
                                    else if((st_TransDataInfo.MemArea & 0x0F) == 2 && g_RXCANMsg.ui32MsgLen != HW_SERIAL_NUMBER_SIZE + 1)
                                    {
                                        /* Transfer Data for HW Serial Number is wrong*/
                                        SendGenericResponse(MEMORY_AREA, WRONG_REQUEST_FORMAT);
                                    }
                                    else
                                    {
                                        WriteLogisticInfo(g_u8rxMsgData, st_TransDataInfo.MemArea);
                                    }
                                }
                            }
                            else
                            {
                                /* Request for other hardware
                                 * Do Nothing */
                            }
                            break;

                        case CMD_CRCRequest:
                        {
                            TMR2_Stop();
                            TMR2_SoftwareCounterClear();
                            error = IsCRCRequestValid(g_RXCANMsg);
                            if(error)
                            {
                                SendGenericResponse(MEMORY_AREA, error);
                            }
                            else
                            {
                                if((g_u8rxMsgData[0] & 0xF0) == MEMORY_AREA)
                                {
                                    if(((g_u8rxMsgData[0] & 0x0F) == 0 || (g_u8rxMsgData[0] & 0x0F) == 4) )//&& ucAppMemoryErase)
                                    {
                                        if(st_BootFlag.FlashAuthorization) // FlashAuthorization
                                        {
                                            ServiceDog();
                                            CRCWrite(g_RXCANMsg, BootStatus);
                                        }
                                    }
                                    else
                                    {
                                        LogisticCRCWrite(g_RXCANMsg);
                                    }
                                }
                                else
                                {
                                    /* Message for other Hardware
                                     * Do Nothing */
                                }
                            }
                        }
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
#ifndef __IS_STANDALONE
        //toggle LED for debug
        if(0 == Get_SysTick()%40) //system boot, blink faster
#else
        if(0 == Get_SysTick()%100) //Stand alone boot, blink slower


#endif
            GPIO_WritePin(BLINKY_LED_GPIO, 0);
        else
            GPIO_WritePin(BLINKY_LED_GPIO, 1);
#endif


    //  ||========================================================||
    //  ||                   End of State Machine                 ||
    //  ||========================================================||

    } // while (1)

}

static void TreatData(uint8_t* Received_Message, St_TransDataInfo *pSt_TransDataInfo)
{
    int i, j;
    uint16_t DataToFlash[FLASH_WORDS_PER_ROW];

    /* Extract data from received frame*/
    for(j = 1; j < 8; j++)
    {
        if(pSt_TransDataInfo->ptr_St_Data->RecvDataIdx < pSt_TransDataInfo->Size + CRC_LENGTH)
        { /*Data + CRC*/
            *(pSt_TransDataInfo->ptr_St_Data->pRecvData + pSt_TransDataInfo->ptr_St_Data->RecvDataIdx) = Received_Message[j];
            pSt_TransDataInfo->ptr_St_Data->RecvDataIdx++;
        }
        else
        { /*i == FLASH_BYTES_PER_ROW_PHANTOM*/
            /* All data are received*/
            break;
        }
    }
    if(pSt_TransDataInfo->ptr_St_Data->RecvDataIdx == pSt_TransDataInfo->Size + CRC_LENGTH)
    {
        if (st_BootFlag.FlashAuthorization == true)
        {
            /* All data received form consecutive frames
             * Extract CRC from last Frame */
            uint16_t CRC = ((uint16_t) *(pSt_TransDataInfo->ptr_St_Data->pRecvData + pSt_TransDataInfo->Size) << 8) +
                    ((uint16_t) *(pSt_TransDataInfo->ptr_St_Data->pRecvData + pSt_TransDataInfo->Size + 1));
            /* Re-calculate CRC*/
            uint16_t CRCCalc = CalcCRC_Bloc(pSt_TransDataInfo->Address, pSt_TransDataInfo->Size, pSt_TransDataInfo->MemArea, pSt_TransDataInfo->ptr_St_Data->pRecvData);

            if(CRCCalc == CRC)
            {
                /*Write data to FLASH*/
                bool CorrectArea = CheckWritingAddress(pSt_TransDataInfo->Address, pSt_TransDataInfo->MemArea, BootStatus);
                ServiceDog();
                if(CorrectArea)
                {
                    /*Fill DataToCopy with 0xFF*/
                    for (i = pSt_TransDataInfo->Size; i < MAX_BLOCK_SIZE; i++)
                    {
                        *(pSt_TransDataInfo->ptr_St_Data->pRecvData + i) = 0xFF;
                    }
                    /*Convert structure for FLASH Routines*/
                    for (i = 0, j = 0; i < FLASH_WORDS_PER_ROW; i++)
                    {
                        DataToFlash[i] = ((uint16_t)*(pSt_TransDataInfo->ptr_St_Data->pRecvData + j) << 8) + (uint16_t)*(pSt_TransDataInfo->ptr_St_Data->pRecvData + j + 1);
                        j += 2;
                    }

                    if(WriteFlash(pSt_TransDataInfo->Address, DataToFlash, pSt_TransDataInfo->Size/2 + pSt_TransDataInfo->Size%2))
                    {
                        /* error happened when write flash*/
                        if ((pSt_TransDataInfo->MemArea & 0x0F) == 0)
                        {
                            /* Problem in app Memory Area
                             * Reset indicators */
                            st_BootFlag.ucAppMemoryErase = false;
                            pSt_TransDataInfo->ValidInfo = false;
                        }
                        else
                        {
                            /*Problem in Logistic memory Area*/
                            st_BootFlag.ucLogMemoryErase = false;
                        }
                        /* Send error*/
                        SendGenericResponse(MEMORY_AREA, WRITING_INVALID);
                    }
                    else
                    {
                        /* No error happened when write flash, send positive ACK, Stop timeout Timer*/
                        pSt_TransDataInfo->ValidInfo = false;
                        SendGenericResponse(MEMORY_AREA, NO_ERROR);
                    }
                    ReleaseFlashPump();

                }
                else
                {
                    /* Ignore Writing in configuration area
                     * Throw error otherwise
                     * Handled in CheckWritingAddress function*/
                }

            }
            else
            {
                /*Send Wrong CRC*/
                SendGenericResponse(MEMORY_AREA, WRONG_CRC);
            }/*(CRCRead == CRC)*/
        }
    }
}

static void IdentifyBoot(MyBootSys* Info)
{ //Identify current boot
    if (boot_even_flag == BootEvenValid)
    {
        Info->BootPolarity              = 0; // current bootloader is Even
        Info->BootValidFlagAddr         = FLAG_BOOT0_ADDRESS;
        Info->BootPNAddr                = BOOT0_PN_ADDRESS;
        Info->OppositBootCRCAddr        = BOOT1_CRC_ADDRESS;
        Info->OppositBootStartAddr      = MEM_BOOT1_START_ADDRESS;
        Info->OppositBootFlagValidAddr  = FLAG_BOOT1_ADDRESS;
        Info->OppositBootEndAddr        = MEM_BOOT1_END_ADDRESS;
        Info->OppositBootValidCode      = BootOddValid;
    }
    else
    {
        Info->BootPolarity              = 1; // current bootloader is Odd
        Info->BootValidFlagAddr         = FLAG_BOOT1_ADDRESS;
        Info->BootPNAddr                = BOOT1_PN_ADDRESS;
        Info->OppositBootCRCAddr        = BOOT0_CRC_ADDRESS;
        Info->OppositBootStartAddr      = MEM_BOOT0_START_ADDRESS;
        Info->OppositBootFlagValidAddr  = FLAG_BOOT0_ADDRESS;
        Info->OppositBootEndAddr        = MEM_BOOT0_END_ADDRESS;
        Info->OppositBootValidCode      = BootEvenValid;
    }
}

static void Init_TransParam(St_TransDataInfo *pTran)
{
    pTran->Address      = 0;
    pTran->BSC          = 0;
    pTran->MemArea      = 0;
    pTran->Size         = 0;
    pTran->ValidInfo    = false;
}

static void Init_BootFlag(void)
{
    ptr_st_BootFlag->ucSecurityUnlocked      = false;
    ptr_st_BootFlag->FlashAuthorization      = false;
    ptr_st_BootFlag->ucAppMemoryErase        = false;
    ptr_st_BootFlag->ucLogMemoryErase        = false;
}

//
// End of file
//
