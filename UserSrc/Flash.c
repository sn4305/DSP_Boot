/**
 * @file Flash.c
 *
 *  Created on: 20210301
 *      Author: E9981231
 */
#include "Flash.h"
#include "F28x_Project.h"
#include "F021_F2837xS_C28x.h"

extern St_BootFlag s_stBootFlag;

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

/** Example_Error - Error function that will halt debugger */
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
    SeizeFlashPump_Bank0();
    oReturnCheck = Fapi_initializeAPI(F021_CPU0_W0_BASE_ADDRESS, 200);
    if(oReturnCheck != Fapi_Status_Success)
    {
        Example_Error(oReturnCheck);
    }
    oReturnCheck = Fapi_setActiveFlashBank(Fapi_FlashBank0);
    if(oReturnCheck != Fapi_Status_Success)
    {
        Example_Error(oReturnCheck);
    }

    SeizeFlashPump_Bank1();
    oReturnCheck = Fapi_initializeAPI(F021_CPU0_W1_BASE_ADDRESS, 200);
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
}

#pragma CODE_SECTION(SwitchBank,".TI.ramfunc");
uint16_t SwitchBank(uint16_t BankIdx)
{
    uint16_t Err_Cnt = 0;
    Fapi_StatusType oReturnCheck;

    if(0 == BankIdx)
    {
        SeizeFlashPump_Bank0();
        oReturnCheck = Fapi_initializeAPI(F021_CPU0_W0_BASE_ADDRESS, 200);
        if(oReturnCheck != Fapi_Status_Success)
        {
            Err_Cnt++;
        }
        oReturnCheck = Fapi_setActiveFlashBank(Fapi_FlashBank0);
        if(oReturnCheck != Fapi_Status_Success)
        {
            Err_Cnt++;
        }
    }
    else if(1 == BankIdx)
    {
        SeizeFlashPump_Bank1();
        oReturnCheck = Fapi_initializeAPI(F021_CPU0_W1_BASE_ADDRESS, 200);
        if(oReturnCheck != Fapi_Status_Success)
        {
            Err_Cnt++;
        }
        oReturnCheck = Fapi_setActiveFlashBank(Fapi_FlashBank1);
        if(oReturnCheck != Fapi_Status_Success)
        {
            Err_Cnt++;
        }
    }

    return Err_Cnt;
}

/** Clear HW info inside flag sector, this will keep App valid flag*/
#pragma CODE_SECTION(prv_EraseLogisticFlash,".TI.ramfunc");
static void prv_EraseLogisticFlash(void)
{
    Fapi_StatusType oReturnCheck;
    Un_FLAG Flag;

    EALLOW;
    SwitchBank(0);

    /*read back AppValid flag first*/
    Flag.flag.App_Valid_Flag = APP_VALID_FLAG;

    /*Erase flag page*/
    oReturnCheck = Fapi_issueAsyncCommandWithAddress(Fapi_EraseSector,
                                                     (uint32 *)FLAG_APPLI_ADDRESS);
    while(Fapi_checkFsmForReady() == Fapi_Status_FsmBusy) ;
    if(oReturnCheck != Fapi_Status_Success)
    {
        /* Error while erasing*/
        SendGenericResponse(SECD_MEMORY_AREA, MEMORY_NOT_BLANK);
    }
    else
    {
        /* No Error, send positive response*/
        s_stBootFlag.bLogMemoryErase = true;
        SendGenericResponse(SECD_MEMORY_AREA, NO_ERROR);
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
        //            SendGenericResponse(SECD_MEMORY_AREA, WRITING_INVALID);
    }
    ReleaseFlashPump();
    EDIS;
}

/** Clear Application Valid Flag inside flag sector, this will keep HW info*/
#pragma CODE_SECTION(prv_ClearAppFlag,".TI.ramfunc");
static uint16_t prv_ClearAppFlag(void)
{
    Un_FLAG Flag;
    uint16_t fail = 0;
    uint16_t *pFlagAddr, i;
    Fapi_StatusType oReturnCheck;

    EALLOW;
    SwitchBank(1);

    /*read back HW flag first*/
    pFlagAddr = (uint16_t *)HW_VERSION_ADDRESS;
    for(i = 0; i < FLAG_TOTAL_LEN; i++)
    {
        if(i < 4)
        {
            /* fill zero in app_valid_flag to disable it*/
            Flag.data[i] = 0xFFFF;
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
        SendGenericResponse(SECD_MEMORY_AREA, MEMORY_NOT_BLANK);
        fail = 1;
    }
    else
    {
        /* No Error, send positive response*/
        s_stBootFlag.bLogMemoryErase = true;
        fail = 0;
//        SendGenericResponse(SECD_MEMORY_AREA, NO_ERROR);
    } /* if Error erasing*/

    fail += WriteFlash(FLAG_APPLI_ADDRESS, Flag.data, FLAG_TOTAL_LEN);
    ReleaseFlashPump();
    EDIS;

    return fail;
}

/** Flash Sector Erase
 * param[in]
 *   sectors bit0 refers to sector A, bit1 refers to sector B and so on,
 *  if bitx == 1, means erase corresponded sector */
#pragma CODE_SECTION(prv_Sector_Erase,".TI.ramfunc");
int prv_Sector_Erase(uint32_t sectors)
{

    uint16_t i = 0;
    Fapi_StatusType oReturnCheck;
//    Fapi_FlashStatusType FlashStatus;
    uint16_t fail = 0;

    EALLOW;
    for(i = 0; i < FLASH_SECTOR_NUM; i++)
    {
        if( (sectors & 0x00000001) == 0x00000001 )
        {
            ServiceDog();
            oReturnCheck = Fapi_issueAsyncCommandWithAddress(Fapi_EraseSector,
                    (uint32 *)(sectAddress[i]));

            if(oReturnCheck != Fapi_Status_Success)
            {
                if(fail == 0) //first fail
                {
//                    SendGenericResponse(SECD_MEMORY_AREA, MEMORY_NOT_BLANK);
                }
                fail++;
            }

            while(Fapi_checkFsmForReady() == Fapi_Status_FsmBusy)
            {
            }
//            FlashStatus = Fapi_getFsmStatus();
        }
        sectors = sectors >> 1;
    }
    EDIS;
    return fail;
}

#pragma CODE_SECTION(EraseFlash,".TI.ramfunc");
void EraseFlash(uint8_t MemoryArea, MyBootSys Info, pSt_BootFlag ptr_st_BootFlag)
{
    uint32_t EraseSector;
    uint16_t Erase_Err = 0;

    if ((MemoryArea & 0x0F) == 1 || (MemoryArea & 0x0F) == 2)
    {
        prv_EraseLogisticFlash();               /* clear HW info with 0xFFFF */
    }
    else
    {
        if (ptr_st_BootFlag->bFlashAuthorization)
        {
            if ((MemoryArea & 0x0F) == 0)
            {
                Erase_Err = prv_ClearAppFlag();             /* clear application valid flag with 0x0000*/
                EraseSector = APP0_SECTOR;
                Erase_Err += SwitchBank(0);
            }
            else
            {
                if(MEM_BOOT1_START_ADDRESS == Info.OppositBootStartAddr)
                {
                    EraseSector = BOOT1_SECTOR;
                    Erase_Err += SwitchBank(1);

                }
                else if(MEM_BOOT0_START_ADDRESS == Info.OppositBootStartAddr)
                {
                    EraseSector = BOOT0_SECTOR;
                    Erase_Err += SwitchBank(0);
                }
            }
            ServiceDog();
            /*Erase all applicative Flash */
            Erase_Err += prv_Sector_Erase(EraseSector);
            if(Erase_Err)
            {
                /* Send error message*/
                SendGenericResponse(SECD_MEMORY_AREA, MEMORY_NOT_BLANK);
            }
            else
            {
                /*Send OK*/
                ptr_st_BootFlag->bAppMemoryErase = true;
                SendGenericResponse(SECD_MEMORY_AREA, NO_ERROR);
            }
            ReleaseFlashPump();

        }/* If Authorized */

    }/* If logistic area */

}

/* len must be divisible by 4, Address must be aligned with 4*/
#pragma CODE_SECTION(WriteFlash,".TI.ramfunc");
uint16_t WriteFlash(uint32_t Address, uint16_t* Data, uint16_t len)
{
    bool Write_Err = true;
    Fapi_StatusType oReturnCheck;
    uint16_t k, j, fail = 0, miniLen;
    uint16_t miniBuffer[4]; //useful for 4-word access to flash with

    EALLOW;
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
            //check that miniBuffer is not already all erased data
            if(!((miniBuffer[0] == 0xFFFF) && (miniBuffer[1] == 0xFFFF) && (miniBuffer[2] == 0xFFFF)
                    && (miniBuffer[3] == 0xFFFF)))
            {
                ServiceDog();
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
        }

    } //for(int k); loads miniBuffer with Buffer elements

    EDIS;

    if(NO_ERROR == fail)
    {
        Write_Err = false;
    }

    return Write_Err;
}

#pragma CODE_SECTION(WriteLogisticInfo,".TI.ramfunc");
void WriteLogisticInfo(volatile uint8_t *Data, uint8_t MemoryArea)
{
    uint8_t i = 0;
    Un_FLAG Flag;
    EALLOW;

    SwitchBank(1);

    if((MemoryArea & 0x0F) == 1)
    {
        /*HW Version case*/
        /*Create structure filled with 0xFF*/
        for (i = 0; i < FLAG_TOTAL_LEN; i++)
        {
            Flag.data[i] = 0xFF;
        }
        Flag.flag.HW_Ver = ((uint16_t)Data[1] << 8) + (uint16_t)Data[2];
        /*Write 2 bytes of data*/
        WriteFlash(HW_VERSION_ADDRESS, &Flag.flag.HW_Ver, 1);
    }
    else if((MemoryArea & 0x0F) == 2)
    {
        for (i = 0; i < FLAG_TOTAL_LEN; i++)
        {
            Flag.data[i] = 0xFF;
        }
        Flag.flag.HW_Ser_H[0] = ((uint16_t)Data[1] << 8) | Data[2];
        Flag.flag.HW_Ser_H[1] = ((uint16_t)Data[3] << 8) | Data[4];
        Flag.flag.HW_Ser_H[2] = ((uint16_t)Data[5] << 8) | Data[6];
        Flag.flag.HW_Ser_L    = (uint16_t)Data[7] << 8;
        /*Write 8 bytes of data*/
        WriteFlash(HW_SERIAL_NUMBER_ADDRESS, (uint16_t *)&Flag.flag.HW_Ser_H, 4);
    }

    ReleaseFlashPump();
    EDIS;
    return;
}
