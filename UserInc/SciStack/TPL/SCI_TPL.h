/*********************************************************************
File name:       SCI_TPL.c
Purpose :        Pack/Unpack SCI message
Copyright Notice:
All source code and data contained in this file is Proprietary and
Confidential to Eaton, and must not be reproduced, transmitted, or
disclosed; in whole or in part, without the express written permission of Eaton.

Copyright 2021 - Eaton, All Rights Reserved.

Author                Date                      Ver#        Description
Jason.Chen         2021/05/20                   1.0          Initialization version


append log after baseline, especially for milestone or release version, no log is allowed for minor modification
**********************************************************************/

#ifndef BSW_SVC_SCISTACK_TPL_SCI_TPL_H_
#define BSW_SVC_SCISTACK_TPL_SCI_TPL_H_

#include <SciStack/DLL/SCI_DLL.h>
#include "string.h"


typedef struct
{
    Uint16 u16MsgId;
    Uint16 u16MsgLen;   // message length
    Uint16 *pu16Data;      // Data address
}stTplMsg_t;

/******************************************************************************************
 * Function Name: SCI_TPL_Init
 * Description  : SCI controller and ring buffer initialization
 * Parameters   : enSciCtrler_t enController, SCI controller which need to be initialization
 * Returns      : i16SciHandle_t, SCI handle, if handle <0, initialization fail.
 * Remark       :
 *****************************************************************************************/
void SCI_TPL_Init(Uint16 u16Controller);

/******************************************************************************************
 * Function Name: SCI_TPL_Send
 * Description  : Send data to SCI
 * Parameters   : i16SciHandle_t i16Handle,  SCI handle
 *                stTplMsg_t *pstTPLmsg
 * Returns      : true=success, false =fail
 * Remark       :
 *****************************************************************************************/
bool SCI_TPL_Send(stTplMsg_t *pstTPLmsg);

/******************************************************************************************
 * Function Name: SCI_TPL_Receive
 * Description  : Receive one message from SCI
 * Parameters   : i16SciHandle_t i16Handle,  SCI handle
 * Returns      : true=success, false =fail
 * Remark       :
 *****************************************************************************************/
bool SCI_TPL_Receive(stTplMsg_t *pstTplMsg);

/******************************************************************************************
 * Function Name: SCI_TPL_ErrHandler
 * Description  : TPL error handler
 * Parameters   : i16SciHandle_t i16Handle,  SCI handle
 * Returns      :
 * Remark       :
 *****************************************************************************************/
void SCI_TPL_ErrHandle(void);

#endif /* BSW_SVC_SCISTACK_TPL_SCI_TPL_H_ */
