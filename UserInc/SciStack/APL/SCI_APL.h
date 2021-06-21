/*********************************************************************
File name:       SCI_APL.h
Purpose :        SCI application layer module
Copyright Notice:
All source code and data contained in this file is Proprietary and
Confidential to Eaton, and must not be reproduced, transmitted, or
disclosed; in whole or in part, without the express written permission of Eaton.

Copyright 2021 - Eaton, All Rights Reserved.

Author                Date                      Ver#        Description
Jason.Chen         2021/05/20                   1.0          Initialization version


append log after baseline, especially for milestone or release version, no log is allowed for minor modification
**********************************************************************/

#ifndef BSW_SVC_SCISTACK_APL_SCI_APL_H_
#define BSW_SVC_SCISTACK_APL_SCI_APL_H_

#define MSG_TYPE_EVENT_MUSK (1<<6)

/*************************************************************************************************************
 * Include Files
 *************************************************************************************************************/

#include <SciStack/TPL/SCI_TPL.h>

/*************************************************************************************************************
 * type Definitions
 *************************************************************************************************************/
typedef stTplMsg_t stSAplMsg_t;

typedef struct
{
  bool   bEnable;
  Uint32 u32Period;
  Uint32 u32Timer;
}stDAplTxMsg_t;

typedef struct
{
  Uint32 u32Period;
  Uint32 u32Timer;
  bool   bMsgRecvived;
}stDAplRxMsg_t;


/*************************************************************************************************************
 *  global variable
 *************************************************************************************************************/
extern stDAplTxMsg_t g_stDAplRegTxMsg[SCI_TX_NUM];
extern stDAplRxMsg_t g_stDAplRegRxMsg[SCI_RX_NUM];

/*************************************************************************************************************
 * Function Definitions
 *************************************************************************************************************/
/******************************************************************************************
 * Function Name: SCI_APL_Init
 * Description  : SCI stack initialization
 * Parameters   :
 * Returns      :
 * Remark       :
 *****************************************************************************************/
void BSW_SVC_SCI_APL_Init(void);

/******************************************************************************************
 * Function Name: SCI_Main_Task
 * Description  : SCI stack main task ,the function should be executed every 2 ms
 * Parameters   :
 * Returns      :
 * Remark       :
 *****************************************************************************************/
void BSW_SVC_SCI_Main_Task2ms(void);

#endif /* BSW_SVC_SCISTACK_APL_SCI_APL_H_ */
