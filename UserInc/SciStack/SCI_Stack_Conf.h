/*
 * SCI_Stack_Conf.h
 *
 *  Created on: 2021骞�5鏈�27鏃�
 *      Author: E0515072
 */

#ifndef BSW_SVC_SCISTACK_SCI_STACK_CONF_H_
#define BSW_SVC_SCISTACK_SCI_STACK_CONF_H_

#include "scicom.h"

#define SCI_CON_A  0
#define SCI_CON_B  1
#define SCI_CON_C  2

extern ST_SEC2PRI_COM_TX_OBJ g_stSec2PriTxObj;
extern ST_PRI2SEC_COM_RX_OBJ g_stPri2SecRxObj;

#define DISBLE_SCI_RX_INTERRUPT
#define ENABLE_SCI_RX_INTERRUPT
#define DISBLE_SCI_TX_INTERRUPT
#define ENABLE_SCI_TX_INTERRUPT

#define SCI_PRISEC_CONTROLLER          SCI_CON_B

#define SCI_BAUDRATE                    enBaud_38400

#define SUPPORT_SCI_C                   0

#define DRV_RINGBUFF_SIZE               32

#define TPL_BUFF_SIZE                   38
#define APL_BUFF_SIZE                   32

#define SCI_TX_NUM                      8
#define SCI_RX_NUM                      3


/********************************************
  typedef struct
   {
     Uint16 u16MsgId;
     Uint16 u16MsgLen;   // message length
     Uint16 *pu16Data;   // Data address
   }stSAplMsg_t;
 *********************************************/
#if 0
#define CNF_SCI_CYC_STX {                                                  \
                           { 0x81, 2, g_stSec2PriTxObj.unTxMsg0x81.u16All}, \
                           { 0xC2, 2, g_stSec2PriTxObj.unTxMsg0xC2.u16All}, \
                        }

#define CNF_SCI_CYC_SRX {                                                  \
                           { 0x01, 9, g_stPriSecRxObj.unRxMsg0x01.u16All}, \
                           { 0x02, 4, g_stPriSecRxObj.unRxMsg0x02.u16All}, \
                           { 0x41, 2, g_stPriSecRxObj.unRxMsg0x41.u16All}, \
                           { 0x42, 1, &g_stPriSecRxObj.unRxMsg0x42.u16All}, \
                        }
#endif
#define CNF_SCI_CYC_STX {                                                  \
                           { SCI_EraseMemory,           1, &g_stSec2PriTxObj.unTxMsgEraseMemory.u16All}, \
                           { SCI_TransferInformation,   4, g_stSec2PriTxObj.unTxMsgTransInfo.u16All}, \
                           { SCI_TransferData,          4, g_stSec2PriTxObj.unTxMsgTransData.u16All}, \
                           { SCI_CRCRequest,            2, g_stSec2PriTxObj.unTxMsgCRCReq.u16All}, \
                           { SCI_SecurityAccess,        4, g_stSec2PriTxObj.unTxMsgSecuAccess.u16All}, \
                           { SCI_LogisticRequest,       1, &g_stSec2PriTxObj.unTxMsgLogiReq.u16All}, \
                           { SCI_SWVersionCheck,        3, g_stSec2PriTxObj.unTxMsgVersionCheck.u16All}, \
                           { SCI_ModeRequest,           4, g_stSec2PriTxObj.unTxMsgModeRequest.u16All}, \
                        }

#define CNF_SCI_CYC_SRX {                                                  \
                           { SCI_GENERAL_RESP,          1, &g_stPri2SecRxObj.unRxMsgGeneralRsp.u16All}, \
                           { SCI_LOGI_RESP,             4, g_stPri2SecRxObj.unRxMsgLogiRsp.u16All}, \
                           { SCI_DIAG_SESSION,          4, g_stPri2SecRxObj.unRxMsgDiagRsp.u16All}, \
                        }
/********************************************
typedef struct
{
  bool   bEnable;
  Uint16 u16Period; // ms
  Uint32 u32Timer;
}stDAplTxMsg_t;
 *********************************************/
#define CNF_SCI_CYC_DTX {                      \
                           {false,  0,    0},  \
                           {false,  0,    0},  \
                           {false,  0,    0},  \
                           {false,  0,    0},  \
                           {false,  0,    0},  \
                           {false,  0,    0},  \
                           {false,  0,    0},  \
                           {false,  0,    0},  \
                        }

/********************************************
typedef struct
{
  Uint32 u32Period;
  Uint32 u32Timer;
  bool   bMsgRecvived;
}stDAplRxMsg_t;
 *********************************************/
#define CNF_SCI_CYC_DRX {                         \
                           {0,      0,  false},  \
                           {0,      0,  false},  \
                           {0,      0,  false},  \
                        }

#endif /* BSW_SVC_SCISTACK_SCI_STACK_CONF_H_ */
