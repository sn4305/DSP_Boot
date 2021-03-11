/*
 * Flash.c
 *
 *  Created on: 2021Äê3ÔÂ1ÈÕ
 *      Author: E9981231
 */
#include "Flash.h"
#include "F28x_Project.h"
#include "F021_F2837xS_C28x.h"

extern bool ucLogMemoryErase, ucAppMemoryErase;

static const Uint32 sectAddress[FLASH_SECTOR_NUM] =
{
 /*bank0*/
 Bzero_SectorA_start,
 Bzero_SectorB_start,
 Bzero_SectorC_start,
 Bzero_SectorD_start,
 Bzero_SectorE_start,
 Bzero_SectorF_start,
 Bzero_SectorG_start,   //FLASH_SIZE 0x6
 Bzero_SectorH_start,
 Bzero_SectorI_start,
 Bzero_SectorJ_start,
 Bzero_SectorK_start,
 Bzero_SectorL_start,
 Bzero_SectorM_start,
 Bzero_SectorN_start,
 /*bank1*/
 Bzero_SectorO_start,
 Bzero_SectorP_start,
 Bzero_SectorQ_start,
 Bzero_SectorR_start,
 Bzero_SectorS_start,
 Bzero_SectorT_start,
 Bzero_SectorU_start,
 Bzero_SectorV_start,
 Bzero_SectorW_start,
 Bzero_SectorX_start,
 Bzero_SectorY_start,
 Bzero_SectorZ_start,
 Bzero_SectorAA_start,
 Bzero_SectorAB_start,
};

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
    Flash1EccRegs.ECC_ENABLE.bit.ENABLE = 0x0;

    Fapi_StatusType oReturnCheck;
    oReturnCheck = Fapi_initializeAPI(F021_CPU0_W0_BASE_ADDRESS, 200);
    if(oReturnCheck != Fapi_Status_Success)
    {
        Example_Error(oReturnCheck);
    }

    oReturnCheck = Fapi_initializeAPI(F021_CPU0_W1_BASE_ADDRESS, 200);
    if(oReturnCheck != Fapi_Status_Success)
    {
        Example_Error(oReturnCheck);
    }

    oReturnCheck = Fapi_setActiveFlashBank(Fapi_FlashBank0);
    if(oReturnCheck != Fapi_Status_Success)
    {
        Example_Error(oReturnCheck);
    }

    oReturnCheck = Fapi_setActiveFlashBank(Fapi_FlashBank1);
    if(oReturnCheck != Fapi_Status_Success)
    {
        Example_Error(oReturnCheck);
    }

    Flash0EccRegs.ECC_ENABLE.bit.ENABLE = 0xA;
    Flash1EccRegs.ECC_ENABLE.bit.ENABLE = 0xA;
    EDIS;

    //
    // Gain pump semaphore for Bank0.
    // User may need to do this for Bank1 if programming Bank1.
    //
    SeizeFlashPump_Bank0();
}

/* Clear HW info inside flag sector, this will keep App valid flag*/
#pragma CODE_SECTION(prv_EraseLogisticFlash,".TI.ramfunc");
static void prv_EraseLogisticFlash(void)
{
    Fapi_StatusType oReturnCheck;
    Un_FLAG Flag;

    EALLOW;
    SeizeFlashPump_Bank1();

    /*read back AppValid flag first*/
    Flag.flag.App_Valid_Flag = APP_VALID_FLAG;

    /*Erase flag page*/
    oReturnCheck = Fapi_issueAsyncCommandWithAddress(Fapi_EraseSector,
                                                     (uint32 *)FLAG_APPLI_ADDRESS);
    while(Fapi_checkFsmForReady() == Fapi_Status_FsmBusy) ;
    if(oReturnCheck != Fapi_Status_Success)
    {
        /* Error while erasing*/
        SendGenericResponse(MEMORY_AREA, MEMORY_NOT_BLANK);
    }
    else
    {
        /* No Error, send positive response*/
        ucLogMemoryErase = true;
        SendGenericResponse(MEMORY_AREA, NO_ERROR);
    } /* if Error erasing*/

    /*Write back AppValid flag, program 4 words at once, 64-bits */
    oReturnCheck = Fapi_issueProgrammingCommand((uint32 *)FLAG_APPLI_ADDRESS,
                                                (uint16_t *)(Flag.data),
                                                2,
                                                0,
                                                0,
                                                Fapi_AutoEccGeneration);
    while(Fapi_checkFsmForReady() == Fapi_Status_FsmBusy) ;
    if(oReturnCheck != Fapi_Status_Success)
    {
        //            SendGenericResponse(MEMORY_AREA, WRITING_INVALID);
    }
    EDIS;
}

/* Clear Application Valid Flag inside flag sector, this will keep HW info*/
#pragma CODE_SECTION(prv_ClearAppFlag,".TI.ramfunc");
static void prv_ClearAppFlag(void)
{
    Un_FLAG Flag;
    uint16_t *pFlagAddr, len, i;
    Fapi_StatusType oReturnCheck;

    EALLOW;
    SeizeFlashPump_Bank1();

    /*read back HW flag first*/
    pFlagAddr = (uint16_t *)HW_VERSION_ADDRESS;
    for(i = 0; i < FLAG_TOTAL_LEN; i++)
    {
        if(i < 4)
        {
            /* fill zero in app_valid_flag to disable it*/
            Flag.data[i] = 0;
        }
        else
        {
            /* copy HW flag to buffer*/
            Flag.data[i] = *pFlagAddr++;
        }
    }

    /*Erase flag page*/
    oReturnCheck = Fapi_issueAsyncCommandWithAddress(Fapi_EraseSector,
                                                     (uint32 *)FLAG_APPLI_ADDRESS);
    while(Fapi_checkFsmForReady() == Fapi_Status_FsmBusy) ;
    if(oReturnCheck != Fapi_Status_Success)
    {
        /* Error while erasing*/
        SendGenericResponse(MEMORY_AREA, MEMORY_NOT_BLANK);
    }
    else
    {
        /* No Error, send positive response*/
        ucLogMemoryErase = true;
        SendGenericResponse(MEMORY_AREA, NO_ERROR);
    } /* if Error erasing*/

    for(i = 0; i <= FLAG_TOTAL_LEN/4; i++)
    {
        if(i == (FLAG_TOTAL_LEN/4))
        {
            len = FLAG_TOTAL_LEN - 4*i;
        }
        else
        {
            len = 4;
        }
        /*Write back AppValid flag, program 4 words at once, 64-bits */
        oReturnCheck = Fapi_issueProgrammingCommand((uint32 *)FLAG_APPLI_ADDRESS,
                                                    (uint16_t *)&(Flag.data[4*i]),
                                                    len,
                                                    0,
                                                    0,
                                                    Fapi_AutoEccGeneration);
        while(Fapi_checkFsmForReady() == Fapi_Status_FsmBusy) ;
        if(oReturnCheck != Fapi_Status_Success)
        {
            ;
        }
    }
    EDIS;
}

/* Flash Sector Erase
 * param:
 *  sectors: bit0 refers to sector A, bit1 refers to sector B
 *  if bitx == 1, means erase corresponded sector*/
#pragma CODE_SECTION(prv_Sector_Erase,".TI.ramfunc");
static int prv_Sector_Erase(uint32_t sectors)
{

    uint16_t i = 0;
    Fapi_StatusType oReturnCheck;
    uint16_t fail = 0;

    EALLOW;
    for(i = 0; i < FLASH_SECTOR_NUM; i++)
    {
        if( (sectors & 0x00000001) == 0x00000001 )
        {
            oReturnCheck = Fapi_issueAsyncCommandWithAddress(Fapi_EraseSector,
                    (uint32 *)(sectAddress[i]));

            if(oReturnCheck != Fapi_Status_Success)
            {
                if(fail == 0) //first fail
                {
//                    SendGenericResponse(MEMORY_AREA, MEMORY_NOT_BLANK);
                }
                fail++;
            }

            while(Fapi_checkFsmForReady() == Fapi_Status_FsmBusy)
            {
            }
        }
        sectors = sectors >> 1;
    }
    EDIS;
    return fail;
}

#pragma CODE_SECTION(EraseFlash,".TI.ramfunc");
void EraseFlash(uint8_t MemoryArea, bool Authorization, MyBootSys Info)
{
    uint32_t EraseSector;
    bool Erase_Err = 0;

    if ((MemoryArea & 0x0F) == 1 || (MemoryArea & 0x0F) == 2)
    {
        prv_EraseLogisticFlash();               /* clear HW info with 0xFFFF */
    }
    else
    {
        if (Authorization)
        {
            if ((MemoryArea & 0x0F) == 0)
            {
                prv_ClearAppFlag();             /* clear application valid flag with 0x0000*/
                EraseSector = APP0_SECTOR;
            }
            else
            {
                if(MEM_BOOT1_START_ADDRESS == Info.OppositBootStartAddr)
                {
                    EraseSector = BOOT1_SECTOR;
                }
                else if(MEM_BOOT0_START_ADDRESS == Info.OppositBootStartAddr)
                {
                    EraseSector = BOOT0_SECTOR;
                }
            }
            /*Erase all applicative Flash */
            Erase_Err = prv_Sector_Erase(EraseSector);
            if(Erase_Err)
            {
                /* Send error message*/
                SendGenericResponse(MEMORY_AREA, MEMORY_NOT_BLANK);
            }
            else
            {
                /*Send OK*/
                ucAppMemoryErase = true;
//                BSC = 0;
                SendGenericResponse(MEMORY_AREA, NO_ERROR);
            }

        }/* If Authorized */

    }/* If logistic area */

}

/* len must be divisible by 4, Address must be aligned with 4*/
#pragma CODE_SECTION(WriteFlash,".TI.ramfunc");
uint16_t WriteFlash(uint32_t Address, uint16_t* Data, uint16_t len)
{
    bool Write_Err = true;
    Fapi_StatusType oReturnCheck;
    uint16_t k, j, fail, miniLen;
    uint16_t miniBuffer[4]; //useful for 4-word access to flash with

    for(k = 0; k <= (len / 4); k++)
    {
        if(k == (len/4))
        {
            miniLen = len - 4 * k;
        }
        else
        {
            miniLen = 4;
        }
        if(miniLen)
        {
            for(j = 0; j < miniLen; j ++)
            {
                miniBuffer[j] = Data[k * 4 + j];
            }
        }
        //check that miniBuffer is not already all erased data
        if(!((miniBuffer[0] == 0xFFFF) && (miniBuffer[1] == 0xFFFF) && (miniBuffer[2] == 0xFFFF)
                && (miniBuffer[3] == 0xFFFF)))
        {
            //program 4 words at once, 64-bits
            oReturnCheck = Fapi_issueProgrammingCommand((uint32 *)(Address + k * 4),
                                                        miniBuffer,
                                                        miniLen,
                                                        0,
                                                        0,
                                                        Fapi_AutoEccGeneration);
            while(Fapi_checkFsmForReady() == Fapi_Status_FsmBusy);
            if(oReturnCheck != Fapi_Status_Success)
            {
                fail++;
            }
#ifdef FLASH_VERIFY
            for(j = 0; j < 4; j += 2)
            {
                uint32_t toVerify = miniBuffer[j+1];
                toVerify = toVerify << 16;
                toVerify |= miniBuffer[j];
                oReturnCheck = Fapi_doVerify((uint32 *)(Address + k * 4 + j),
                                             1,
                                             (uint32 *)(&toVerify),
                                             &oFlashStatusWord);
                if(oReturnCheck != Fapi_Status_Success)
                {
                    Write_Err = true;
                }
            } //for j; for Fapi_doVerify
#endif
        } //check if miniBuffer does not contain all already erased data
    } //for(int k); loads miniBuffer with Buffer elements

    if(NO_ERROR == fail)
    {
        Write_Err = false;
    }

    return Write_Err;
}

#pragma CODE_SECTION(WriteLogisticInfo,".TI.ramfunc");
void WriteLogisticInfo(void)
{
    Fapi_StatusType oReturnCheck;
    uint16 miniBuffer[5] = {0xAA11, 0xBB22, 0xCC33, 0xDD44, 0xFF55};
    EALLOW;

    if( true )
    {
        SeizeFlashPump_Bank1();
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
