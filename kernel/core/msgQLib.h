/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : msgQLib.h
 * 内容摘要   :
 * 其它说明   :
 * 版本       :
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-06-16 09:24:44 AM
 *
 * 修改记录1:
 *    修改日期: 2023-06-16
 *    版 本 号:
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容:
 * ===========================================================================
 */

#ifndef __MSGQLIB_H__
#define __MSGQLIB_H__
#include "coreLib.h"

typedef struct msgQNode {
    TLIST   qNode;
    int     length;
} msgQNode_t;

/* for msgQ options */
#define MSG_Q_TYPE_MASK     0x03
#define MSG_Q_FIFO          0x00
#define MSG_Q_PRIORITY      0x01
#define MSG_Q_ZERO_COPY     0x04 /* support zero copy data for msgQ */

typedef struct msgQ {
    int         numMsgs;
    int         options;
    int         maxMsgLen;   /* max data length for per msg */
    void *      memBase;
    TLIST       qFree;       /* free msgQNode */
    TLIST       msgQ;        /* msgQNode */
    int         numFree;
    SEMAPHORE   semMsgTx;      /* blocked task queue head */
    SEMAPHORE   semMsgRx;      /* blocked task queue head */
} msgQ_t;

typedef msgQ_t * MSG_Q_ID;

STATUS msgQLibInit(void);
MSG_Q_ID msgQCreate(int numMsg, int maxMsgDataLen, int options);
int msgQReceive(MSG_Q_ID msgQId, char **pbuf, UINT maxNBytes, int timeout);
STATUS msgQSend(MSG_Q_ID msgQId, char *buf, UINT nBytes, int timeout, int priority);
STATUS msgQDelete(MSG_Q_ID msgQId);

#endif /* #ifndef __MSGQLIB_H__ */

