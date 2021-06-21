/*********************************************************************
File name:       SCI_ERR.h
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

#ifndef BSW_SVC_SCISTACK_ERR_SCI_ERR_H_
#define BSW_SVC_SCISTACK_ERR_SCI_ERR_H_

/*************************************************************************************************************
 * Include Files
 *************************************************************************************************************/
#include "F2837xS_device.h"
#include <SciStack/SCI_Stack_Conf.h>

/*************************************************************************************************************
 *  Definitions
 *************************************************************************************************************/
#define SCI_APL_TX_ERR          1
#define SCI_APL_RX_PERIOD_ERR   (1<<1)
#define SCI_TPL_WRONGLENGTH_ERR (1<<2)
#define SCI_TPL_CHECKSUM_ERR    (1<<3)
#define SCI_DLL_COM_ERR         (1<<4)
#define SCI_DLL_DISCONNECT_ERR  (1<<5)
#define SCI_DLL_TX_BUFOVER_ERR  (1<<6)
#define SCI_DLL_RX_BUFOVER_ERR  (1<<7)


/*************************************************************************************************************
 * Output global variable
 *************************************************************************************************************/
extern Uint16 g_u16SciErrStatus;

/*************************************************************************************************************
 * Function Definitions
 *************************************************************************************************************/
/******************************************************************************************
 * Function Name: void SCI_ERR_Init(void)
 * Description  : SCI error module init
 * Parameters   : void
 * Returns      :
 * Remark       :
 *****************************************************************************************/
void SCI_ERR_Init(void);

/******************************************************************************************
 * Function Name:unSciErr_t SCI_ERR_GetErrStatus(Uint16 u16SciChannel)
 * Description  : interface to application to get SCI error status
 * Parameters   : u16SciChannel
 * Returns      : unSciErr_t
 * Remark       :
 *****************************************************************************************/
Uint16 SCI_ERR_GetErrStatus( );

/******************************************************************************************
 * Function Name: void SCI_ERR_ClearErrStatus(Uint16 u16SciChannel)
 * Description  : interface to application to clear SCI error status
 * Parameters   : u16SciChannel, sci channel
 * Returns      : unSciErr_t
 * Remark       :
 *****************************************************************************************/
void SCI_ERR_ClearErrStatus(Uint16 unErrFlag);

/******************************************************************************************
 * Function bool SCI_ERR_SetErr(Uint16 u16SciChannel)
 * Description  : interface to Set SCI error status
 * Parameters   : u16SciChannel
 * Returns      : unSciErr_t
 * Remark       :
 *****************************************************************************************/
#define SCI_ERR_SetErr(u16ErrFlag)  {g_u16SciErrStatus |= u16ErrFlag;}

#endif /* BSW_SVC_SCISTACK_ERR_SCI_ERR_H_ */
