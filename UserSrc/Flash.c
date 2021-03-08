/*
 * Flash.c
 *
 *  Created on: 2021Äê3ÔÂ1ÈÕ
 *      Author: E9981231
 */
#include "Flash.h"
#include "F28x_Project.h"
#include "F021_F2837xS_C28x.h"

//
// Example_Error - Error function that will halt debugger
//
#pragma CODE_SECTION(Example_Error,".TI.ramfunc");
void Example_Error(Fapi_StatusType status)
{
    //
    // Error code will be in the status parameter
    //
    __asm("    ESTOP0");
}

//
// Init_Flash_Sectors - Initialize flash sectors
//
void Init_Flash_Sectors(void)
{
    EALLOW;
    Flash0EccRegs.ECC_ENABLE.bit.ENABLE = 0x0;
//    Flash1EccRegs.ECC_ENABLE.bit.ENABLE = 0x0;

    Fapi_StatusType oReturnCheck;
    oReturnCheck = Fapi_initializeAPI(F021_CPU0_W0_BASE_ADDRESS, 150);
    if(oReturnCheck != Fapi_Status_Success)
    {
        Example_Error(oReturnCheck);
    }

    oReturnCheck = Fapi_setActiveFlashBank(Fapi_FlashBank0);
    if(oReturnCheck != Fapi_Status_Success)
    {
        Example_Error(oReturnCheck);
    }

    Flash0EccRegs.ECC_ENABLE.bit.ENABLE = 0xA;
    EDIS;

    //
    // Gain pump semaphore for Bank0.
    // User may need to do this for Bank1 if programming Bank1.
    //
    SeizeFlashPump_Bank0();
}

#pragma CODE_SECTION(WriteLogisticInfo,".TI.ramfunc");
void WriteLogisticInfo(void)
{
    Fapi_StatusType oReturnCheck;
//    Fapi_FlashStatusWordType oFlashStatusWord;
    uint16 miniBuffer[5] = {0xAA11, 0xBB22, 0xCC33, 0xDD44, 0xFF55};
    EALLOW;

    if( true )
    {
        oReturnCheck = Fapi_issueAsyncCommandWithAddress(Fapi_EraseSector,
                (uint32 *)HW_VERSION_ADDRESS);
        while(Fapi_checkFsmForReady() == Fapi_Status_FsmBusy) ;
        if(oReturnCheck != Fapi_Status_Success)
        {
            Example_Error(oReturnCheck);
        }

        //program 4 words at once, 64-bits
        oReturnCheck = Fapi_issueProgrammingCommand((uint32 *)HW_VERSION_ADDRESS,
                miniBuffer,
                sizeof(miniBuffer),
                0,
                0,
                Fapi_AutoEccGeneration);
        while(Fapi_checkFsmForReady() == Fapi_Status_FsmBusy) ;
        if(oReturnCheck != Fapi_Status_Success)
        {
            Example_Error(oReturnCheck);
        }
    }

    EDIS;
    return;
}
