/**
* @file   main.c
* @brief Last Modified in 20210427.
*/

/**
* \mainpage Bootloader
* \section intro_sec Introduction
* This bootloader is developed for CNH C sample.
* HW platform: TI DSP TMS320f28377s, secondary DSP, this bootloader communicate with gateway via internal CAN BUS.
* Communication Interface: CAN, speed: 500k.
*
* <A HREF="http://172.18.121.26/svn/eMobility/eM-C001/Firmware/Base2CNH/DSP/SecondaryDSP/Bootloader/Trunk">Project SVN path</a>
*
* \section Copyright_sec Copyright Notice
* All source code and data contained in this file is Proprietary and
* Confidential to Eaton, and must not be reproduced, transmitted, or
* disclosed; in whole or in part, without the express written permission of Eaton.
* Copyright 2021 - Eaton, All Rights Reserved.
*
*\section His_sec History
* Author      |        Date          |     Ver#    |    Description (CR#)
* ----------- | -------------------- | ----------- | -------------------------------------------------------------
* Dongdong Yang    |   20210225      |     00      |    Init for Base project 2nd DSP(28377s) Bootloader.
* Dongdong Yang    |   20210413      |     01      |    Optimize code according to coding guideline V1.1.
* Dongdong Yang    |   20210421      |     02      |    Optimize variable name according to coding guideline V1.1.
* Dongdong Yang    |   20210427      |     03      |    Added comments according to doxygen document rules.
*******************************************************************/

#include "main.h"

static BootMachineStates s_stState = State_TRANSITION;              /**< Boot StateMachine state */
static MyBootSys s_stBootStatus;                                    /**< Boot informations: polarity, self info, oppsite info */
uint16_t s_u16AddrCfg = 0;                                          /**< OBC address configuration, not used now */
St_BootFlag s_stBootFlag = {false, false, false, false};            /**< state flag of boot */
static uint8_t s_u8RecvBuf[MAX_BLOCK_SIZE + CRC_LENGTH] = {0};      /**< Data block receive buffer, used to receive data from CAN and copy to flash*/
static St_TransData s_stTransData = {0, s_u8RecvBuf, 0};            /**< Data struct used in CMD_TransferData*/
static St_TransDataInfo s_stTransDataInfo = {0, 0, 0, 0, 0, &s_stTransData};       /**< Data struct used in CMD_TransferInformation*/
static Uint16 pu16DataTmp[8] = {0};
static stTplMsg_t stTplRxMsg = {0, 0, (Uint16 *)pu16DataTmp};


/* function declaration */
static void TreatData(volatile uint8_t* Received_Message, St_TransDataInfo *pSt_TransDataInfo);
static void IdentifyBoot(void);
static void Init_TransParam(void);
static void Init_BootFlag(void);

/********************************************************
*   @brief define global Macro in different project build configuration
*  *Open CCS project propeties->Build->C2000 Compiler->predefined Symbols;
*  *Add macro in Pre-define NAME;
*********************************************************/
#ifndef __IS_STANDALONE
/* function declaration */
uint32_t MainBoot(void);

/**
* Preboot main function.
*
* First Bootloader software function entry point, branch to Boot0 or Boot1
* according to Boot area valid flag value.
*********************************************************/
#pragma CODE_SECTION(main,".preboot");
uint32_t main(void)
{//Pre boot sequence
   if(boot_even_flag == BOOT_EVEN_VALID)
   {
       if(0x0 == g_u40BootVersion[0]
          && 0x0 == g_u40BootVersion[1]
          && 0x0 == g_u40BootVersion[2])
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
   else if(boot_odd_flag == BOOT_ODD_VALID)
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
 * Bootloader main function.
 *
 * In system build configuration it is MainBoot;
 * In standalone build configuration it is main function for boot0 or boot1.
 */
#ifndef __IS_STANDALONE
uint32_t MainBoot(void)
#else
uint32_t main(void)
#endif
{
    /****************************************************
    * Step 1. Initialize System Control:
    * Enable Peripheral Clocks
    * This function is found in the F2837xS_SysCtrl.c file.
    ******************************************************/
    InitSysCtrl(); //PLL activates

    /***************************************************
    *  Unlock CSM
    *
    *  If the API functions are going to run in unsecured RAM
    *  then the CSM must be unlocked in order for the flash
    *  API functions to access the flash.
    *  If the flash API functions are executed from secure memory
    *  then this step is not required.
    **************************************************/
    CsmUnlock();

    /***************************************************
    * Step 2. Initialize GPIO:
    * This example function is found in the F2837xS_Gpio.c file and
    * illustrates how to set the GPIO to it's default state.
    ***************************************************/
    InitGpio();

#ifdef DEMOBOARD
    //LED init
    GPIO_SetupPinMux(BLINKY_LED_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(BLINKY_LED_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(CAN_LED_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(CAN_LED_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
#endif
    /***************************************************
    * Step 3. Clear all interrupts and initialize PIE vector table:
    * Disable CPU interrupts
    ***************************************************/
    DINT;

    /***************************************************
    * Initialize the PIE control registers to their default state.
    * The default state is all PIE interrupts disabled and flags
    * are cleared.
    * This function is found in the F2837xS_PieCtrl.c file.
    ***************************************************/
    InitPieCtrl();

    /***************************************************
    * Disable CPU interrupts and clear all CPU interrupt flags:
    ***************************************************/
    IER = 0x0000;
    IFR = 0x0000;

    /***************************************************
    * Initialize the PIE vector table with pointers to the shell Interrupt
    * Service Routines (ISR).
    ***************************************************/
    InitPieVectTable();

    SCI_Init();
    /* Initialize the CANA. */
    InitCana();

    /* Initialize the Timer. */
    Init_Timer();

    /* Initialize the Flash/FMC. */
    Init_Flash_Sectors();

    /* Enable CPU interrupts */
    EINT;

    /* Start CAN module A operations  */
    CANEnable(CANA_BASE);

    ServiceDog();

    Init_BootFlag();
    Init_TransParam();
    IdentifyBoot();

    /*  ||========================================================||
    *   ||                 Start of s_stBootStatus Machine                 ||
    *   ||========================================================||  */
    while (1)
    {
        SCI_TPL_ErrHandle();
        switch (s_stState)
        {
    /*  ||========================================================||
    *   ||                 STATE: TRANSITION                      ||
    *   ||========================================================||  */
            case State_TRANSITION:
                TMR1_Start();
                ServiceDog();
                if(UPDATE_APP_RQST == g_u32UpdataFlag)
                {/* received boot request from APP, jump to boot*/
                    TMR1_Start();
                    TMR1_SoftwareCounterClear();
                    SCI_Send_Cmd(SCI_ModeRequest, NULL, 8);
                    s_stState = State_BOOT; /* <====== GO BOOT   */
                }
                else if(TMR1_SoftwareCounterGet() < BOOT_DELAY)
                {   /** wait 2s to receive boot command in case of APP jumping failure*/
                    if(g_bCAN_RX_Flag)
                    {
                        Clr_CanRxFlag();
                        if(CMD_ModeRequest == g_enumCAN_Command)
                        {
                            if(BOOT_MODE == (g_stRXCANMsg.pu8MsgData[6] & 0x07))
                            {
                                g_u32UpdataFlag = UPDATE_APP_RQST;
                                SCI_Send_Cmd(SCI_ModeRequest, NULL, 8);
//                                SendDiagnosticResponse(BOOT_MODE, s_u16AddrCfg);
                                s_stState = State_BOOT; /* <====== GO BOOT  */
                            }
                        }
                    }
                }
                else if(APP_VALID ==  APP_VALID_FLAG)
                {/* FlagAppli is valid, jump to APP*/
                    TMR1_Stop();
                    TMR1_SoftwareCounterClear();
                    DisableDog();
                    DELAY_US(200000L);
                    StartApplication();
                }
                else
                {
                    TMR1_Stop();
                    TMR1_SoftwareCounterClear();
//                    SendDiagnosticResponse(DEFAULT_MODE, s_u16AddrCfg);
                    s_stState = State_DEFAULT; /* <====== GO DEFAULT  */
                }
                break;

    /*  ||========================================================||
    *   ||                 STATE: DEFAULT                         ||
    *   ||========================================================||  */
            case State_DEFAULT:
                ServiceDog();
                if(g_bCAN_RX_Flag)
                {
                    Clr_CanRxFlag();
                    if(CMD_ModeRequest == g_enumCAN_Command)
                    {
                        if(BOOT_MODE == (g_stRXCANMsg.pu8MsgData[6] & 0x07))
                        {
                            g_u32UpdataFlag = UPDATE_APP_RQST;
                            SCI_Send_Cmd(SCI_ModeRequest, NULL, 8);
//                            SendDiagnosticResponse(BOOT_MODE, s_u16AddrCfg);
                            s_stState = State_BOOT; /* <====== GO BOOT  */
                        }
                        else if(DEFAULT_MODE == (g_stRXCANMsg.pu8MsgData[6] & 0x07))
                        {
                            g_u32UpdataFlag = 0;
                            DEADLOOP();     /**< use dead loop trigger watch dog reset*/
                        }
                    }
                }
                break;

    /*  ||========================================================||
    *   ||                 STATE: BOOT                            ||
    *   ||========================================================||  */
            case State_BOOT:
                if(TMR1_SoftwareCounterGet() >= BOOT_TIMEOUT)
                {   /*500 * 10ms = 5000ms */
                    /*5s Timeout*/
                    g_u32UpdataFlag = 0;
                    Init_BootFlag();
                    TMR1_SoftwareCounterClear();
                    DEADLOOP();     /**< use dead loop trigger watch dog reset*/
                }

                ServiceDog();

                /*Wait for message*/
                if(g_bCAN_RX_Flag)
                {
                    Clr_CanRxFlag();
                    TMR1_SoftwareCounterClear();
                    uint8_t error = NO_ERROR;
                    switch(g_enumCAN_Command)
                    {
                        case CMD_ModeRequest:
                            /* Mode Request*/
                            error = IsRequestValid(g_stRXCANMsg);
                            if(error)
                            {
                                SendGenericResponse(SECD_MEMORY_AREA, error);
                            }
                            else
                            {
                                if(BOOT_MODE == (g_stRXCANMsg.pu8MsgData[6] & 0x07))
                                {/* Enter boot mode request : Send positive Response
                                 * No Action*/
                                    Init_TransParam();
                                    SCI_Send_Cmd(SCI_ModeRequest, NULL, 8);
                                }
                                else if(DEFAULT_MODE == (g_stRXCANMsg.pu8MsgData[6] & 0x07))
                                {
                                    /* Reset to preboot*/
                                    g_u32UpdataFlag = 0;
                                    Init_BootFlag();
                                    DEADLOOP();     /**< use dead loop trigger watch dog reset*/
                                }
                                else
                                {
                                    /* Do nothing */
                                }
                            }
                            break;

                        case CMD_LogisticRequest:
                            /* Logistic Request*/
                            error = IsLogisticValid(g_stRXCANMsg);
                            if (error != 0)
                            {
                                /* Error case */
                                SendGenericResponse(g_stRXCANMsg.pu8MsgData[0] & 0xE, error);
                            }
                            else
                            {
                                /* Send response to request */
                                LogiticRequestHandle(g_stRXCANMsg.pu8MsgData[0]);
                            }
                            break;

                        case CMD_SWVersionCheck:
                            error = IsSWVersionCheckValid(g_stRXCANMsg);
                            if (error)
                            {
                                SendGenericResponse(g_stRXCANMsg.pu8MsgData[0] & 0xF0, error);
                            }
                            else
                            {
                                SWVersionComparetHandle(g_stRXCANMsg, &s_stBootStatus, &s_stBootFlag);
                            }
                            break;

                        case CMD_SecurityAccess:
                            error = IsSecurityValid(g_stRXCANMsg);
                            if(SECD_MEMORY_AREA == (g_stRXCANMsg.pu8MsgData[0] & 0xF0) )
                            {
                                if(!error)
                                {
                                    /* Security unlocked successfully */
                                    s_stBootFlag.bSecurityUnlocked = true;
                                }
                                else
                                {
                                    /* Security unlocked fail         */
                                    s_stBootFlag.bSecurityUnlocked = false;
                                }
                                SendGenericResponse(SECD_MEMORY_AREA, error);
                            }
                            else if(PRIM_MEMORY_AREA == (g_stRXCANMsg.pu8MsgData[0] & 0xF0))
                            {
                                SCI_Send_Cmd(SCI_SecurityAccess, g_stRXCANMsg.pu8MsgData, 8);
                            }
                            break;

                        case CMD_EraseMemory:
                            error = IsEraseValid(g_stRXCANMsg, s_stBootFlag.bSecurityUnlocked);
                            if(SECD_MEMORY_AREA == (g_stRXCANMsg.pu8MsgData[0] & 0xF0))
                            {
                                if(error)
                                {
                                    /* Error case : send NOK*/
                                    SendGenericResponse(SECD_MEMORY_AREA, error);
                                }
                                else
                                {
                                    EraseFlash(g_stRXCANMsg.pu8MsgData[0], s_stBootStatus, &s_stBootFlag);
                                }
                            }
                            else if(PRIM_MEMORY_AREA == (g_stRXCANMsg.pu8MsgData[0] & 0xF0))
                            {
                                /* forward Erase primary DSP command to primary by SCI*/
                                SCI_Send_Cmd(SCI_EraseMemory, (uint8_t *)g_stRXCANMsg.pu8MsgData, 1);
                            }
                            break;

                        case CMD_TransferInformation:
                            /* Start timer for timeout*/
                            TMR2_Start();
                            /* Initialize counter*/
                            TMR2_SoftwareCounterClear();
                            error = IsTransferInfoValid(g_stRXCANMsg, &s_stTransDataInfo, &s_stBootFlag);
                            if (SECD_MEMORY_AREA == (g_stRXCANMsg.pu8MsgData[0] & 0xF0))
                            {
                                if (error)
                                {
                                    /* Send error message*/
                                    SendGenericResponse(SECD_MEMORY_AREA, error);
                                }
                                else
                                {
                                    /* Correct Memory area*/
                                    if (g_stRXCANMsg.pu8MsgData[1] != (uint8_t) ((s_stTransDataInfo.u8BSC + 1) & 0xFF))
                                    {
                                        /* Problem in u8BSC*/
                                        SendGenericResponse(SECD_MEMORY_AREA, WRONG_REQUEST_FORMAT);
                                    }
                                    else
                                    {
                                        /* Store information from frame, prepare for data reception*/
                                        s_stTransDataInfo.u8BSC++;
                                        s_stTransDataInfo.pst_Data->u8SN = 0;
                                        s_stTransDataInfo.pst_Data->u16RecvDataIdx = 0;
                                        s_stTransDataInfo.bValidInfo = true;
                                        s_stTransDataInfo.u8MemArea = g_stRXCANMsg.pu8MsgData[0];
                                    }
                                }
                            }
                            else if(PRIM_MEMORY_AREA == (g_stRXCANMsg.pu8MsgData[0] & 0xF0))
                            {
                                /* forward Erase primary DSP command to primary by SCI*/
                                s_stTransDataInfo.u8MemArea = g_stRXCANMsg.pu8MsgData[0];
                                SCI_Send_Cmd(SCI_TransferInformation, (uint8_t *)g_stRXCANMsg.pu8MsgData, 8);
                            }
                            break;

                        case CMD_TransferData:
                            if(SECD_MEMORY_AREA == (s_stTransDataInfo.u8MemArea & 0xF0))
                            {
                                /*Request for this board*/
                                error = IsTransferDataValid(g_stRXCANMsg, &s_stTransDataInfo);
                                if(error)
                                {
                                    if (SAME_SN == error)
                                    {
                                        /* Same sequence number as previous frame: ignore the frame*/
                                        TMR2_SoftwareCounterClear();
                                    }
                                    else
                                    {
                                        s_stTransDataInfo.bValidInfo = false;
                                        s_stTransDataInfo.pst_Data->u8SN = 0;
                                        SendGenericResponse(SECD_MEMORY_AREA, error);
                                    }
                                }
                                else if(SECD_MEMORY_AREA == s_stTransDataInfo.u8MemArea || 4 == (s_stTransDataInfo.u8MemArea & 0x0F))
                                {
                                    /* Reset timeout counter*/
                                    TMR2_SoftwareCounterClear();
                                    s_stTransDataInfo.pst_Data->u8SN++;
                                    /* Receive data*/
                                    TreatData(g_stRXCANMsg.pu8MsgData, &s_stTransDataInfo);
                                }
                                else
                                {
                                    /*Logistic information*/
                                    if (1 == (s_stTransDataInfo.u8MemArea & 0x0F) && g_stRXCANMsg.u16MsgLen != HW_VERSION_SIZE + 1)
                                    {
                                        /* Transfer Data for HW version is wrong*/
                                        SendGenericResponse(SECD_MEMORY_AREA, WRONG_REQUEST_FORMAT);
                                    }
                                    else if(2 == (s_stTransDataInfo.u8MemArea & 0x0F) && g_stRXCANMsg.u16MsgLen != HW_SERIAL_NUMBER_SIZE + 1)
                                    {
                                        /* Transfer Data for HW Serial Number is wrong*/
                                        SendGenericResponse(SECD_MEMORY_AREA, WRONG_REQUEST_FORMAT);
                                    }
                                    else
                                    {
                                        WriteLogisticInfo(g_stRXCANMsg.pu8MsgData, s_stTransDataInfo.u8MemArea);
                                    }
                                }
                            }
                            else if(PRIM_MEMORY_AREA == (s_stTransDataInfo.u8MemArea & 0xF0))
                            {
                                /* forward Erase primary DSP command to primary by SCI*/
                                SCI_Send_Cmd(SCI_TransferData, (uint8_t *)g_stRXCANMsg.pu8MsgData, 8);
                            }
                            break;

                        case CMD_CRCRequest:
                        {
                            TMR2_Stop();
                            TMR2_SoftwareCounterClear();
                            error = IsCRCRequestValid(g_stRXCANMsg);
                            if(error)
                            {
                                SendGenericResponse(SECD_MEMORY_AREA, error);
                            }
                            else
                            {
                                if(SECD_MEMORY_AREA == (g_stRXCANMsg.pu8MsgData[0] & 0xF0))
                                {
                                    if(0 == (g_stRXCANMsg.pu8MsgData[0] & 0x0F) || 4 == (g_stRXCANMsg.pu8MsgData[0] & 0x0F))
                                    {
                                        if(s_stBootFlag.bFlashAuthorization) // bFlashAuthorization
                                        {
                                            ServiceDog();
                                            CRCWrite(g_stRXCANMsg, &s_stBootStatus, &s_stBootFlag);
                                        }
                                    }
                                    else
                                    {
                                        LogisticCRCWrite(g_stRXCANMsg);
                                    }
                                }
                                else if(PRIM_MEMORY_AREA == (g_stRXCANMsg.pu8MsgData[0] & 0xF0))
                                {
                                    /* forward Erase primary DSP command to primary by SCI*/
                                    SCI_Send_Cmd(SCI_CRCRequest, (uint8_t *)g_stRXCANMsg.pu8MsgData, 3);
                                }
                            }
                        }
                            break;

                        default:
                            /* Do Nothing*/
                            break;
                    }
                }/* If CAN frame is received*/

                if(g_bSCI_TX_Flag)
                {/* If SCI frame is transmitted, then waiting reply*/

                    TMR0_Start();

                    switch(g_u16SCIMsgId)
                    {
                        case SCI_EraseMemory:
                            if(TMR0_SoftwareCounterGet() <= SCI_TIMOUT_EraseMemory)
                            {
                                if(SCI_TPL_Receive(&stTplRxMsg) == true)
                                {
                                    switch(stTplRxMsg.u16MsgId)
                                    {
                                    case SCI_GENERAL_RESP:
                                        /*received general_response*/
                                        g_bSCI_TX_Flag = false;         /* stop receiving, already received.*/
                                        TMR0_Stop();
                                        TMR0_SoftwareCounterClear();
                                        SendGenericResponse((uint8_t)(stTplRxMsg.pu16Data[0]), (uint8_t)stTplRxMsg.pu16Data[1]);
                                        break;
                                    default:
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                g_bSCI_TX_Flag = false;         /* stop receiving, timeout error occurred.*/
                                SendGenericResponse(PRIM_MEMORY_AREA, TIMEOUT);  /* send timeout error by CAN.*/
                                TMR0_Stop();
                                TMR0_SoftwareCounterClear();
                            }
                            break;

                        case SCI_TransferInformation:
                            if(TMR0_SoftwareCounterGet() <= SCI_TIMOUT_TransferInformation)
                            {
                                if(SCI_TPL_Receive(&stTplRxMsg) == true)
                                {
                                    switch(stTplRxMsg.u16MsgId)
                                    {
                                    case SCI_GENERAL_RESP:
                                        /*received negative general_response*/
                                        g_bSCI_TX_Flag = false;         /* stop receiving, already received.*/
                                        TMR0_Stop();
                                        TMR0_SoftwareCounterClear();
                                        SendGenericResponse((uint8_t)(stTplRxMsg.pu16Data[0]), (uint8_t)stTplRxMsg.pu16Data[1]);
                                        break;
                                    default:
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                g_bSCI_TX_Flag = false;         /* stop receiving, timeout error occurred.*/
//                                SendGenericResponse(PRIM_MEMORY_AREA, TIMEOUT);  /* send timeout error by CAN.*/
                                TMR0_Stop();
                                TMR0_SoftwareCounterClear();
                            }
                            break;

                        case SCI_TransferData:
                            if(TMR0_SoftwareCounterGet() <= SCI_TIMOUT_TransferData)
                            {
                                if(SCI_TPL_Receive(&stTplRxMsg) == true)
                                {
                                    switch(stTplRxMsg.u16MsgId)
                                    {
                                    case SCI_GENERAL_RESP:
                                        /*received general_response*/
                                        g_bSCI_TX_Flag = false;         /* stop receiving, already received.*/
                                        TMR0_Stop();
                                        TMR0_SoftwareCounterClear();
                                        SendGenericResponse((uint8_t)(stTplRxMsg.pu16Data[0]), (uint8_t)stTplRxMsg.pu16Data[1]);
                                        break;
                                    default:
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                g_bSCI_TX_Flag = false;         /* stop receiving, timeout error occurred.*/
//                                SendGenericResponse(PRIM_MEMORY_AREA, TIMEOUT);  /* send timeout error by CAN.*/
                                TMR0_Stop();
                                TMR0_SoftwareCounterClear();
                            }
                            break;

                        case SCI_CRCRequest:
                            if(TMR0_SoftwareCounterGet() <= SCI_TIMOUT_CRCRequest)
                            {
                                if(SCI_TPL_Receive(&stTplRxMsg) == true)
                                {
                                    switch(stTplRxMsg.u16MsgId)
                                    {
                                    case SCI_GENERAL_RESP:
                                        /*received general_response*/
                                        g_bSCI_TX_Flag = false;         /* stop receiving, already received.*/
                                        TMR0_Stop();
                                        TMR0_SoftwareCounterClear();
                                        SendGenericResponse((uint8_t)(stTplRxMsg.pu16Data[0]), (uint8_t)stTplRxMsg.pu16Data[1]);
                                        break;
                                    default:
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                g_bSCI_TX_Flag = false;         /* stop receiving, timeout error occurred.*/
                                SendGenericResponse(PRIM_MEMORY_AREA, TIMEOUT);  /* send timeout error by CAN.*/
                                TMR0_Stop();
                                TMR0_SoftwareCounterClear();
                            }
                            break;

                        case SCI_SecurityAccess:
                            if(TMR0_SoftwareCounterGet() <= SCI_TIMOUT_SecurityAccess)
                            {
                                if(SCI_TPL_Receive(&stTplRxMsg) == true)
                                {
                                    switch(stTplRxMsg.u16MsgId)
                                    {
                                    case SCI_GENERAL_RESP:
                                        /*received general_response*/
                                        g_bSCI_TX_Flag = false;         /* stop receiving, already received.*/
                                        TMR0_Stop();
                                        TMR0_SoftwareCounterClear();
                                        SendGenericResponse((uint8_t)(stTplRxMsg.pu16Data[0]), (uint8_t)stTplRxMsg.pu16Data[1]);
                                        break;
                                    default:
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                g_bSCI_TX_Flag = false;         /* stop receiving, timeout error occurred.*/
                                SendGenericResponse(PRIM_MEMORY_AREA, TIMEOUT);  /* send timeout error by CAN.*/
                                TMR0_Stop();
                                TMR0_SoftwareCounterClear();
                            }
                            break;

                        case SCI_LogisticRequest:
                            if(TMR0_SoftwareCounterGet() <= SCI_TIMOUT_LogisticRequest)
                            {
                                if(SCI_TPL_Receive(&stTplRxMsg) == true)
                                {/*received negative general_response*/
                                    ;
                                }
                            }
                            else
                            {
                                g_bSCI_TX_Flag = false;         /* stop receiving, timeout error occurred.*/
                                SendGenericResponse(PRIM_MEMORY_AREA, TIMEOUT);  /* send timeout error by CAN.*/
                                TMR0_Stop();
                                TMR0_SoftwareCounterClear();
                            }
                            break;

                        case SCI_SWVersionCheck:
                            if(TMR0_SoftwareCounterGet() <= SCI_TIMOUT_SWVersionCheck)
                            {
                                if(SCI_TPL_Receive(&stTplRxMsg) == true)
                                {
                                    switch(stTplRxMsg.u16MsgId)
                                    {
                                    case SCI_GENERAL_RESP:
                                        /*received negative general_response*/
                                        g_bSCI_TX_Flag = false;         /* stop receiving, already received.*/
                                        TMR0_Stop();
                                        TMR0_SoftwareCounterClear();
                                        SendGenericResponse((uint8_t)(stTplRxMsg.pu16Data[0]), (uint8_t)stTplRxMsg.pu16Data[1]);
                                        break;
                                    case SCI_LOGI_RESP:
                                    {
                                        /*received positive logistic_response*/
                                        uint8_t ActualVersion[5];
                                        g_bSCI_TX_Flag = false;         /* stop receiving, already received.*/
                                        TMR0_Stop();
                                        TMR0_SoftwareCounterClear();
                                        ActualVersion[0] = (uint8_t)(stTplRxMsg.pu16Data[1]);
                                        ActualVersion[1] = (uint8_t)(stTplRxMsg.pu16Data[2]);
                                        ActualVersion[2] = (uint8_t)(stTplRxMsg.pu16Data[3]);
                                        ActualVersion[3] = (uint8_t)(stTplRxMsg.pu16Data[4]);
                                        ActualVersion[4] = (uint8_t)(stTplRxMsg.pu16Data[5]);
                                        SendLogisticResponse((uint8_t)(stTplRxMsg.pu16Data[0]), ActualVersion, 5);
                                        break;
                                    }
                                    default:
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                g_bSCI_TX_Flag = false;         /* stop receiving, timeout error occurred.*/
                                SendGenericResponse(PRIM_MEMORY_AREA, TIMEOUT);  /* send timeout error by CAN.*/
                                TMR0_Stop();
                                TMR0_SoftwareCounterClear();
                            }
                            break;

                        case SCI_ModeRequest:
                            if(TMR0_SoftwareCounterGet() <= SCI_TIMOUT_ModeRequest)
                            {
                                if(SCI_TPL_Receive(&stTplRxMsg) == true)
                                {
                                    switch(stTplRxMsg.u16MsgId)
                                    {
                                    case SCI_DIAG_SESSION:
                                        g_bSCI_TX_Flag = false;         /* stop receiving, already received.*/
                                        TMR0_Stop();
                                        TMR0_SoftwareCounterClear();
                                        if(0xC0 == stTplRxMsg.pu16Data[4])
                                        {
                                            /*received positive general_response*/
                                            SendDiagnosticResponse(BOTH_BOOT_STATE, s_u16AddrCfg);
                                        }
                                        else
                                        {
                                            /*received negative general_response*/
                                            SendDiagnosticResponse(ONLY_SEC_BOOT_STATE, s_u16AddrCfg);
                                        }
                                        break;
                                    default:
                                        break;
                                    }
                                }
                                else
                                {/* SCI Stack RX buffer is empty now. */
                                    ;
                                }
                            }
                            else
                            {
                                g_bSCI_TX_Flag = false;         /* stop receiving, timeout error occurred.*/
                                TMR0_Stop();
                                TMR0_SoftwareCounterClear();
                                SendDiagnosticResponse(ONLY_SEC_BOOT_STATE, s_u16AddrCfg);
                            }
                            break;

                        default:
                            break;
                    }

                }

                break;

            default:
                s_stState = State_TRANSITION;
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

/**
* Treat received CAN message data frame. Be called in CMD_TransferData,
* it will parse TP and copy data to data buffer. Every time when data block
* is full, it will calculate block CRC, and will write block data into flash if CRC verification passed.
*
* @param[in] Received_Message pointer of received message.
* @param[in] pSt_TransDataInfo pointer of transfer data information struct.
*********************************************************/
static void TreatData(volatile uint8_t* Received_Message, St_TransDataInfo *pSt_TransDataInfo)
{
    int i, j;
    uint16_t DataToFlash[FLASH_WORDS_PER_ROW];

    /** Extract data from received frame*/
    for(j = 1; j < 8; j++)
    {
        if(pSt_TransDataInfo->pst_Data->u16RecvDataIdx < pSt_TransDataInfo->u16Size + CRC_LENGTH)
        { /*Data + CRC*/
            *(pSt_TransDataInfo->pst_Data->pu8RecvData + pSt_TransDataInfo->pst_Data->u16RecvDataIdx) = Received_Message[j];
            pSt_TransDataInfo->pst_Data->u16RecvDataIdx++;
        }
        else
        { /*i == FLASH_BYTES_PER_ROW_PHANTOM*/
            /* All data are received*/
            break;
        }
    }
    if(pSt_TransDataInfo->pst_Data->u16RecvDataIdx == pSt_TransDataInfo->u16Size + CRC_LENGTH)
    {
        if (s_stBootFlag.bFlashAuthorization == true)
        {
            /* All data received form consecutive frames
             * Extract CRC from last Frame */
            uint16_t CRC = ((uint16_t) *(pSt_TransDataInfo->pst_Data->pu8RecvData + pSt_TransDataInfo->u16Size) << 8) +
                    ((uint16_t) *(pSt_TransDataInfo->pst_Data->pu8RecvData + pSt_TransDataInfo->u16Size + 1));
            /* Re-calculate CRC*/
            uint16_t CRCCalc = CalcCRC_Bloc(pSt_TransDataInfo->u32Address, pSt_TransDataInfo->u16Size, pSt_TransDataInfo->u8MemArea, pSt_TransDataInfo->pst_Data->pu8RecvData);

            if(CRCCalc == CRC)
            {
                /*Write data to FLASH*/
                bool CorrectArea = CheckWritingAddress(pSt_TransDataInfo->u32Address, pSt_TransDataInfo->u8MemArea, &s_stBootStatus);
                ServiceDog();
                if(CorrectArea)
                {
                    /*Fill DataToCopy with 0xFF*/
                    for(i = pSt_TransDataInfo->u16Size; i < MAX_BLOCK_SIZE; i++)
                    {
                        *(pSt_TransDataInfo->pst_Data->pu8RecvData + i) = 0xFF;
                    }
                    /*Convert structure for FLASH Routines*/
                    for(i = 0, j = 0; i < FLASH_WORDS_PER_ROW; i++)
                    {
                        DataToFlash[i] = ((uint16_t)*(pSt_TransDataInfo->pst_Data->pu8RecvData + j) << 8) + (uint16_t)*(pSt_TransDataInfo->pst_Data->pu8RecvData + j + 1);
                        j += 2;
                    }

                    if(WriteFlash(pSt_TransDataInfo->u32Address, DataToFlash, pSt_TransDataInfo->u16Size/2 + pSt_TransDataInfo->u16Size%2))
                    {
                        /* error happened when write flash*/
                        if ((pSt_TransDataInfo->u8MemArea & 0x0F) == 0)
                        {
                            /* Problem in app Memory Area
                             * Reset indicators */
                            s_stBootFlag.bAppMemoryErase = false;
                            pSt_TransDataInfo->bValidInfo = false;
                        }
                        else
                        {
                            /*Problem in Logistic memory Area*/
                            s_stBootFlag.bLogMemoryErase = false;
                        }
                        /* Send error*/
                        SendGenericResponse(SECD_MEMORY_AREA, WRITING_INVALID);
                    }
                    else
                    {
                        /* No error happened when write flash, send positive ACK, Stop timeout Timer*/
                        pSt_TransDataInfo->bValidInfo = false;
                        SendGenericResponse(SECD_MEMORY_AREA, NO_ERROR);
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
                SendGenericResponse(SECD_MEMORY_AREA, WRONG_CRC);
            }/*(CRCRead == CRC)*/
        }
    }
}

/**
 * Identify Boot location, be called at every reset cycle.
 */
static void IdentifyBoot(void)
{ //Identify current boot
    if (boot_even_flag == BOOT_EVEN_VALID)
    {
        s_stBootStatus.BootPolarity              = 0; // current bootloader is Even
        s_stBootStatus.BootValidFlagAddr         = FLAG_BOOT0_ADDRESS;
        s_stBootStatus.BootPNAddr                = BOOT0_PN_ADDRESS;
        s_stBootStatus.OppositBootCRCAddr        = BOOT1_CRC_ADDRESS;
        s_stBootStatus.OppositBootStartAddr      = MEM_BOOT1_START_ADDRESS;
        s_stBootStatus.OppositBootFlagValidAddr  = FLAG_BOOT1_ADDRESS;
        s_stBootStatus.OppositBootEndAddr        = MEM_BOOT1_END_ADDRESS;
        s_stBootStatus.OppositBootValidCode      = BOOT_ODD_VALID;
    }
    else
    {
        s_stBootStatus.BootPolarity              = 1; // current bootloader is Odd
        s_stBootStatus.BootValidFlagAddr         = FLAG_BOOT1_ADDRESS;
        s_stBootStatus.BootPNAddr                = BOOT1_PN_ADDRESS;
        s_stBootStatus.OppositBootCRCAddr        = BOOT0_CRC_ADDRESS;
        s_stBootStatus.OppositBootStartAddr      = MEM_BOOT0_START_ADDRESS;
        s_stBootStatus.OppositBootFlagValidAddr  = FLAG_BOOT0_ADDRESS;
        s_stBootStatus.OppositBootEndAddr        = MEM_BOOT0_END_ADDRESS;
        s_stBootStatus.OppositBootValidCode      = BOOT_EVEN_VALID;
    }
}

/**
 * Initiate transfer data struct, be called at start or when data transfer break.
 */
static void Init_TransParam(void)
{
    s_stTransDataInfo.u32Address        = 0;
    s_stTransDataInfo.u8BSC             = 0;
    s_stTransDataInfo.u8MemArea         = 0;
    s_stTransDataInfo.u16Size           = 0;
    s_stTransDataInfo.bValidInfo        = false;
    s_stTransData.u16RecvDataIdx        = 0;
    s_stTransData.u8SN                  = 0;
}

#if 0
static uint8_t ReadConfigurationPin(void)
{

    bool AddressL = GPIO_ReadPin();
    bool AddressH = GPIO_ReadPin();
    uint8_t Address = (AddressH << 1) | AddressL;

    uint8_t Address = 0;

    return Address;
}
#endif

/**
 * Initiate Boot flag struct, be called at start or when data transfer break.
 */
static void Init_BootFlag(void)
{
    s_stBootFlag.bSecurityUnlocked      = false;
    s_stBootFlag.bFlashAuthorization    = false;
    s_stBootFlag.bAppMemoryErase        = false;
    s_stBootFlag.bLogMemoryErase        = false;
#if 0
    s_u16AddrCfg = ReadConfigurationPin();
#endif
}

//
// End of file
//
