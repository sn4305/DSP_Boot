/*********************************************************************
File name:       SCI_Buf.h
Purpose :        Ring buffer for SCI receive/transmit message
Copyright Notice:
All source code and data contained in this file is Proprietary and
Confidential to Eaton, and must not be reproduced, transmitted, or
disclosed; in whole or in part, without the express written permission of Eaton.

Copyright 2021 - Eaton, All Rights Reserved.

Author                Date                      Ver#        Description
Jason.Chen         2021/05/20                   1.0          Initialization version


append log after baseline, especially for milestone or release version, no log is allowed for minor modification
**********************************************************************/

#ifndef BSW_SVC_SCISTACK_DLL_SCI_BUF_H_
#define BSW_SVC_SCISTACK_DLL_SCI_BUF_H_

/*************************************************************************************************************
 * Include Files
 *************************************************************************************************************/
#include "F2837xS_device.h"
#include "F2837xS_Examples.h"
#include <SciStack/SCI_Stack_Conf.h>

/*************************************************************************************************************
 * type Definitions
 *************************************************************************************************************/
typedef enum
{
    BUF_NO_ERROR      = 0,
    ERROR_WRONG_PARAM = 1,
    ERROR_NO_SPACE    = 2,
    ERROR_NO_DATA     = 3,
}enBufErr_t;



typedef enum
{
    enSCI_RX_Buf_ID = 0,
    enSCI_TX_Buf_ID = 1,
}enSciBuffId_t;

typedef struct
{
    volatile  Uint16 au16Buf[DRV_RINGBUFF_SIZE]; // ring  buffer to store RX/TX message from/to SCI
    volatile  Uint16 u16Fornt;
    volatile  Uint16 u16Rear;
    volatile  Uint16 u16RemainNum; // the remain  space number in ring buffer
}stRingBuff_t;

/*************************************************************************************************************
 * Function Definitions
 *************************************************************************************************************/
/******************************************************************************************
 * Function Name: RingBuff_Init
 * Description  : ring buffer initialization
 * Parameters   : void
 * Returns      : void
 * Remark       :
 *****************************************************************************************/
void  SCI_RingBuff_Init(void);

/******************************************************************************************
 * Function Name: RingBuff_Reset
 * Description  : ring buffer reset
 * Parameters   : SCI buffer id
 * Returns      : err_t, error id, 0=No error, others=error,
 * Remark       :
 *****************************************************************************************/
enBufErr_t SCI_RingBuff_Reset(enSciBuffId_t enId);

/******************************************************************************************
 * Function Name: RingBuff_Write
 * Description  : write the data to ring buffer
 * Parameters   : enSciBuffId_t enId, ring buffer id
 *                Uint16* pu16Data,   the address of data  which to be write
 *                Uint16 u16Num,      Data length
 * Returns      : err_t, error id, 0=No error, others=error,
 * Remark       :
 *****************************************************************************************/
enBufErr_t SCI_RingBuff_Write(enSciBuffId_t enId, Uint16* pu16Data, Uint16 u16Num);

/******************************************************************************************
 * Function Name: RingBuff_Read
 * Description  : read the data to ring buffer
 * Parameters   : enSciBuffId_t enId, ring buffer id
 *                Uint16* pu16Data,   the address of data  which to be read
 *                Uint16 u16Num,      Data length
 * Returns      : err_t, error id, 0=No error, others=error,
 * Remark       :
 *****************************************************************************************/
enBufErr_t SCI_RingBuff_Read(enSciBuffId_t enId, Uint16* pu16Data, Uint16 u16Num);



#endif /* BSW_SVC_SCISTACK_DLL_SCI_BUF_H_ */
