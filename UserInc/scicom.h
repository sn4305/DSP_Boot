/*
 * scicom.h
 *
 *  Created on: 2021Äê6ÔÂ16ÈÕ
 *      Author: E9981231
 */

#ifndef USERINC_SCICOM_H_
#define USERINC_SCICOM_H_

#include "F28x_Project.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_uart.h"
#include "driverlib/uart.h"

#define SCI_GENERAL_RESP                    0x90
#define SCI_LOGI_RESP                       0x91
#define SCI_DIAG_SESSION                    0x92

/* CAN Id */
#define SCI_EraseMemory                     0xE0
#define SCI_TransferInformation             0xE1
#define SCI_TransferData                    0xE2
#define SCI_CRCRequest                      0xE3
#define SCI_SecurityAccess                  0xE4
#define SCI_LogisticRequest                 0xE5
#define SCI_SWVersionCheck                  0xE6
#define SCI_ModeRequest                     0xE7

/*SCI timeout, unit: 1ms */
#define SCI_TIMOUT_EraseMemory              200U
#define SCI_TIMOUT_TransferInformation      40U
#define SCI_TIMOUT_TransferData             150U
#define SCI_TIMOUT_CRCRequest               250U
#define SCI_TIMOUT_SecurityAccess           40U
#define SCI_TIMOUT_LogisticRequest          40U
#define SCI_TIMOUT_SWVersionCheck           40U
#define SCI_TIMOUT_ModeRequest              40U

typedef struct
{
    Uint16 u4MemArea    :4;
    Uint16 u4SubSect    :4;
}ST_EraseMemory_SIGN;

typedef union
{
    Uint16 u16All;
    ST_EraseMemory_SIGN stSignal;
}UN_EraseMemory_OBJ;

typedef struct
{
    Uint16 u4MemArea    :4;
    Uint16 u4SubSect    :4;
    Uint16 u8BSC        :8;         /**< Bloc Sequence Counter*/
    Uint16 u8Addr1      :8;         /**< Address high byte*/
    Uint16 u8Addr2      :8;         /**< Address mid byte*/
    Uint16 u8Addr3      :8;         /**< Address low byte*/
    Uint16 u8MemSize1   :8;         /**< memory size high byte*/
    Uint16 u8MemSize2   :8;         /**< memory size low byte*/
    Uint16 u8rsv        :8;         /**< reserve byte*/
}ST_TransInfo_SIGN;

typedef union
{
    Uint16 u16All[4];
    ST_TransInfo_SIGN stSignal;
}UN_TransInfo_OBJ;

typedef struct
{
    Uint16 u8SN         :8;         /**< data frame Sequence Number */
    Uint16 u8D1         :8;         /**< data 1 */
    Uint16 u8D2         :8;         /**< data 2 */
    Uint16 u8D3         :8;         /**< data 3 */
    Uint16 u8D4         :8;         /**< data 4 */
    Uint16 u8D5         :8;         /**< data 5 */
    Uint16 u8D6         :8;         /**< data 6 */
    Uint16 u8D7         :8;         /**< data 7 */
}ST_TransData_SIGN;

typedef union
{
    Uint16 u16All[4];
    ST_TransData_SIGN stSignal;
}UN_TransData_OBJ;

typedef struct
{
    Uint16 u4MemArea    :4;
    Uint16 u4SubSect    :4;
    Uint16 u8CRCMSB     :8;
    Uint16 u8CRCLSB     :8;
    Uint16 u8rsv        :8;         /**< reserve byte*/
}ST_CRCReq_SIGN;

typedef union
{
    Uint16 u16All[2];
    ST_CRCReq_SIGN stSignal;
}UN_CRCReq_OBJ;

typedef struct
{
    Uint16 u4MemArea    :4;
    Uint16 u4SubSect    :4;
    Uint16 u8Key1       :8;
    Uint16 u8Key2       :8;
    Uint16 u8Key3       :8;
    Uint16 u8Key4       :8;
    Uint16 u8Key5       :8;
    Uint16 u8Key6       :8;
    Uint16 u8Key7       :8;
}ST_SecuAccess_SIGN;

typedef union
{
    Uint16 u16All[4];
    ST_SecuAccess_SIGN stSignal;
}UN_SecuAccess_OBJ;

typedef struct
{
    Uint16 u4MemArea    :4;
    Uint16 u4SubSect    :4;
    Uint16 u8rsv        :8;         /**< reserve byte*/
}ST_LogiReq_SIGN;

typedef union
{
    Uint16 u16All;
    ST_LogiReq_SIGN stSignal;
}UN_LogiReq_OBJ;

typedef struct
{
    Uint16 u4MemArea    :4;
    Uint16 u4SubSect    :4;
    Uint16 u8Ver1       :8;         /**< SW version high byte in ASCII */
    Uint16 u8Ver2       :8;
    Uint16 u8Ver3       :8;
    Uint16 u8Ver4       :8;
    Uint16 u8Ver5       :8;         /**< SW version last byte in ASCII */
}ST_VersionCheck_SIGN;

typedef union
{
    Uint16 u16All[3];
    ST_VersionCheck_SIGN stSignal;
}UN_VersionCheck_OBJ;

typedef struct
{
    Uint16 u16Rsv1      :16;
    Uint16 u8Rsv2       :8;
    Uint16 u4Rsv3       :4;
    Uint16 u4Mode       :4;
    Uint16 u8Rsv4       :8;
    Uint16 u8Rsv5       :8;
    Uint16 u8Rsv6       :8;
}ST_ModeRequest_SIGN;

typedef union
{
    Uint16 u16All[4];
    ST_ModeRequest_SIGN stSignal;
}UN_ModeRequest_OBJ;

typedef struct
{
    Uint16 u4MemArea    :4;
    Uint16 u4SubSect    :4;
    Uint16 u8Error      :8;
}ST_GENERAL_RESP_SIGN;

typedef union
{
    Uint16 u16All;
    ST_GENERAL_RESP_SIGN stSignal;
}UN_GENERAL_RESP_OBJ;

typedef struct
{
    Uint16 u4MemArea    :4;
    Uint16 u4SubSect    :4;
    Uint16 u8D1         :8;         /**< data 1 */
    Uint16 u8D2         :8;         /**< data 2 */
    Uint16 u8D3         :8;         /**< data 3 */
    Uint16 u8D4         :8;         /**< data 4 */
    Uint16 u8D5         :8;         /**< data 5 */
    Uint16 u8D6         :8;         /**< data 6 */
    Uint16 u8D7         :8;         /**< data 7 */
}ST_LOGI_RESP_SIGN;

typedef union
{
    Uint16 u16All[4];
    ST_LOGI_RESP_SIGN stSignal;
}UN_LOGI_RESP_OBJ;

typedef struct
{
    Uint16 u8D0         :8;
    Uint16 u8D1         :8;         /**< data 1 */
    Uint16 u8D2         :8;         /**< data 2 */
    Uint16 u8D3         :8;         /**< data 3 */
    Uint16 u8D4         :8;         /**< data 4 */
    Uint16 u8D5         :8;         /**< data 5 */
    Uint16 u8D6         :8;         /**< data 6 */
    Uint16 u8D7         :8;         /**< data 7 */
}ST_DIAG_SIGN;

typedef union
{
    Uint16 u16All[4];
    ST_DIAG_SIGN stSignal;
}UN_DIAG_OBJ;

typedef struct
{
    UN_EraseMemory_OBJ  unTxMsgEraseMemory;
    UN_TransInfo_OBJ    unTxMsgTransInfo;
    UN_TransData_OBJ    unTxMsgTransData;
    UN_CRCReq_OBJ       unTxMsgCRCReq;
    UN_SecuAccess_OBJ   unTxMsgSecuAccess;
    UN_LogiReq_OBJ      unTxMsgLogiReq;
    UN_VersionCheck_OBJ unTxMsgVersionCheck;
    UN_ModeRequest_OBJ  unTxMsgModeRequest;
}ST_SEC2PRI_COM_TX_OBJ;

typedef struct
{
    UN_GENERAL_RESP_OBJ     unRxMsgGeneralRsp;
    UN_LOGI_RESP_OBJ        unRxMsgLogiRsp;
    UN_DIAG_OBJ             unRxMsgDiagRsp;
}ST_PRI2SEC_COM_RX_OBJ;


extern bool g_bSCI_TX_Flag;
extern Uint16 g_u16SCIMsgId;

/**
* Send msg from 2nd dsp to 1st dsp by sci.
*
* @param u16MsgId Message ID.
* @param u8timeout timeout, unit: 10ms.
 */
void SCI_Send_Cmd(Uint16 u16MsgId, uint8_t *data, uint8_t len);
void SCI_Init(void);


#endif /* USERINC_SCICOM_H_ */
