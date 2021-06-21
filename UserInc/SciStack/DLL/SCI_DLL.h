/*********************************************************************
File name:       SCI_DLL.h
Purpose :        Send/Received message through SCI
Copyright Notice:
All source code and data contained in this file is Proprietary and
Confidential to Eaton, and must not be reproduced, transmitted, or
disclosed; in whole or in part, without the express written permission of Eaton.

Copyright 2021 - Eaton, All Rights Reserved.

Author                Date                      Ver#        Description
Jason.Chen         2021/05/20                   1.0          Initialization version


append log after baseline, especially for milestone or release version, no log is allowed for minor modification
**********************************************************************/

#ifndef BSW_SVC_SCISTACK_DLL_SCI_DLL_H_
#define BSW_SVC_SCISTACK_DLL_SCI_DLL_H_

/*************************************************************************************************************
 * Include Files
 *************************************************************************************************************/
#include "F2837xS_device.h"
#include "F2837xS_Examples.h"
#include <SciStack/SCI_Stack_Conf.h>

/*************************************************************************************************************
 * type Definitions
 *************************************************************************************************************/

/*SCI parameter configuration*/
typedef enum
{
    enNo_Parity   = 0,
    enOdd_Parity  = 1,
    enEven_Parity = 2,
}enParity_t;

typedef enum
{
    enBaud_1200,
    enBaud_9600,
    enBaud_19200,
    enBaud_38400,
    enBaud_56000,
    enBaud_57600,
    enBaud_115200,
    enBaud_256000,
}enBaud_t;

typedef struct
{
    Uint16      u16Stopbit;
    enParity_t  enParity;
    Uint16      u16CharLen; // character-length
    enBaud_t    enBaud;     // Baudrate
}stSciConfig_t;


/*************************************************************************************************************
 * Function Definitions
 *************************************************************************************************************/
/******************************************************************************************
 * Function Name: SCI_DLL_Init
 * Description  : SCI controller and ring buffer initialization
 * Parameters   : enSciCtrler_t enController, SCI controller which need to be initialization
 *                stSciConfig_t *pstSciCfg, SCI communication configuration parameters
 * Returns      : i16SciHandle_t, SCI handle, if handle <0, initialization fail.
 * Remark       :
 *****************************************************************************************/
int16 SCI_DLL_Init(Uint16 u16Controller, stSciConfig_t *pstSciCfg);

/******************************************************************************************
 * Function Name: SCI_DLL_Reset
 * Description  : Reset DLL
 * Parameters   : enSciCtrler_t enController,
 * Returns      :
 * Remark       :
 *****************************************************************************************/
void SCI_DLL_Reset(void);

/******************************************************************************************
 * Function Name: SCI_DLL_Receive
 * Description  : Receive one 16bit data from SCI
 * Parameters   : i16SciHandle_t i16Handle,  SCI handle
 *                Uint16 * pu16Data, data address which to save the message.
 * Returns      : true=success, false =fail
 * Remark       :
 *****************************************************************************************/
bool SCI_DLL_Receive(Uint16* pu16Data, Uint16 u16Len);

/******************************************************************************************
 * Function Name: SCI_DLL_Send
 * Description  : Send data to SCI
 * Parameters   : i16SciHandle_t i16Handle,  SCI handle
 *                Uint16 * pu16Data, data address which need to be sent
 *                 Uint16 u16Len, Data length to be sent
 * Returns      : true=success, false =fail
 * Remark       :
 *****************************************************************************************/
bool SCI_DLL_Send(Uint16 * pu16Data, Uint16 u16Len);

/******************************************************************************************
 * Function Name: SCI_DLL_ErrHandle
 * Description  : Reset DLL
 * Parameters   : enSciCtrler_t enController,
 * Returns      :
 * Remark       :
 *****************************************************************************************/
void SCI_DLL_ErrHandle(void);

#if (SCI_PRISEC_CONTROLLER == SCI_CON_A)
/******************************************************************************************
 * Function Name: SCI_RX_InterruptServiceRoutine
 * Description  : SCI Data receive interrupt Service Routine
 * Parameters   :
 * Returns      :
 * Remark       :
 *****************************************************************************************/
void SCIA_RX_InterruptServiceRoutine(void);

/******************************************************************************************
 * Function Name: SCI_TX_InterruptServiceRoutine
 * Description  : SCI Data transmit interrupt Service Routine
 * Parameters   :
 * Returns      :
 * Remark       :
 *****************************************************************************************/
void SCIA_TX_InterruptServiceRoutine(void);
#endif

#if (SCI_PRISEC_CONTROLLER == SCI_CON_B)
/******************************************************************************************
 * Function Name: SCI_RX_InterruptServiceRoutine
 * Description  : SCI Data receive interrupt Service Routine
 * Parameters   :
 * Returns      :
 * Remark       :
 *****************************************************************************************/
void SCIB_RX_InterruptServiceRoutine(void);

/******************************************************************************************
 * Function Name: SCI_TX_InterruptServiceRoutine
 * Description  : SCI Data transmit interrupt Service Routine
 * Parameters   :
 * Returns      :
 * Remark       :
 *****************************************************************************************/
void SCIB_TX_InterruptServiceRoutine(void);
#endif


#endif /* BSW_SVC_SCISTACK_DLL_SCI_DLL_H_ */
