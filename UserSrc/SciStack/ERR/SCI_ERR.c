/*********************************************************************
File name:       SCI_ERR.c
Purpose :        SCI Error management module
Copyright Notice:
All source code and data contained in this file is Proprietary and
Confidential to Eaton, and must not be reproduced, transmitted, or
disclosed; in whole or in part, without the express written permission of Eaton.

Copyright 2021 - Eaton, All Rights Reserved.

Author                Date                      Ver#        Description
Jason.Chen         2021/05/20                   1.0          Initialization version


append log after baseline, especially for milestone or release version, no log is allowed for minor modification
**********************************************************************/

#include <SciStack/ERR/SCI_ERR.h>


/*************************************************************************************************************
 *  Global variable
 *************************************************************************************************************/
Uint16 g_u16SciErrStatus;

/******************************************************************************************
 * Function Name: void SCI_ERR_Init(void)
 * Description  : SCI error module init
 * Parameters   :
 * Returns      :
 * Remark       :
 *****************************************************************************************/
void SCI_ERR_Init(void)
{
    g_u16SciErrStatus = 0;
}

/******************************************************************************************
 * Function Name:unSciErr_t SCI_ERR_GetErrStatus(Uint16 u16SciChannel)
 * Description  : interface to application to get SCI error status
 * Parameters   : u16SciChannel
 * Returns      : unSciErr_t
 * Remark       :
 *****************************************************************************************/
Uint16 SCI_ERR_GetErrStatus(void)
{
    return g_u16SciErrStatus;
}

/******************************************************************************************
 * Function Name: void SCI_ERR_ClearErrStatus(Uint16 u16SciChannel)
 * Description  : interface to application to clear SCI error status
 * Parameters   : u16SciChannel, sci channel
 * Returns      : unSciErr_t
 * Remark       :
 *****************************************************************************************/
void SCI_ERR_ClearErrStatus(Uint16 unErrFlag)
{
    g_u16SciErrStatus &= ~unErrFlag ;
}
