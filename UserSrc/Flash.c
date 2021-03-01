/*
 * Flash.c
 *
 *  Created on: 2021��3��1��
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
//    oReturnCheck = Fapi_initializeAPI(F021_CPU0_W1_BASE_ADDRESS, 150);
//    if(oReturnCheck != Fapi_Status_Success)
//    {
//        Example_Error(oReturnCheck);
//    }

    oReturnCheck = Fapi_setActiveFlashBank(Fapi_FlashBank0);
    if(oReturnCheck != Fapi_Status_Success)
    {
        Example_Error(oReturnCheck);
    }
//    oReturnCheck = Fapi_setActiveFlashBank(Fapi_FlashBank1);
//    if(oReturnCheck != Fapi_Status_Success)
//    {
//        Example_Error(oReturnCheck);
//    }

    Flash0EccRegs.ECC_ENABLE.bit.ENABLE = 0xA;
//    Flash1EccRegs.ECC_ENABLE.bit.ENABLE = 0xA;
    EDIS;
}
