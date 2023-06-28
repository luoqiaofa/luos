/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : msgQLib.c
 * 内容摘要   :
 * 其它说明   :
 * 版本       :
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-06-16 10:42:02 AM
 *
 * 修改记录1:
 *    修改日期: 2023-06-16
 *    版 本 号:
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容:
 * ===========================================================================
 */

/******************************************************************************
 *                #include (依次为标准库头文件、非标准库头文件)               *
 ******************************************************************************/
#include "coreLib.h"

STATUS msgQLibInit(void)
{
    return OK;
}

static STATUS msgQInit(MSG_Q_ID msgQId, int numMsg, int maxDataLen, int options)
{
    msgQNode_t *msg;
    int idx;
    size_t sz, one;
    void *p;
    int  semOpts;

    if (NULL == msgQId || numMsg < 0 || maxDataLen < 0 || options < 0) {
        return ERROR;
    }
    switch(options & MSG_Q_TYPE_MASK) {
        case MSG_Q_FIFO:
            semOpts = SEM_Q_FIFO;
            break;
        case MSG_Q_PRIORITY:
            semOpts = SEM_Q_PRIORITY;
            break;
        default:
            return ERROR;
            break;
    }
    memset(msgQId, 0, sizeof(*msgQId));
    msgQId->options = options;

    one = STACK_ROUND_UP(sizeof(*msg) + maxDataLen);
    if (MSG_Q_ZERO_COPY & options) {
        one = STACK_ROUND_UP(sizeof(*msg) + sizeof(void *));
    }
    
    sz = numMsg * one;
    msgQId->memBase = osMemAlloc(sz);
    if (NULL == msgQId->memBase) {
        return ERROR;
    }
    p = msgQId->memBase;
    INIT_LIST_HEAD(&msgQId->msgQ);
    INIT_LIST_HEAD(&msgQId->qFree);
    for (idx = 0; idx < numMsg; idx++) {
        msg = (msgQNode_t *)p;
        msg->length = one - sizeof(*msg);
        if (MSG_Q_ZERO_COPY & msgQId->options) {
            msg->length = maxDataLen;
        }
        list_add_tail(&msg->qNode, &msgQId->qFree);
        p = p + one;
    }

    msgQId->numMsgs   = numMsg;
    msgQId->numFree   = numMsg;
    msgQId->maxMsgLen = one - sizeof(*msg);
    if (MSG_Q_ZERO_COPY & msgQId->options) {
        msgQId->maxMsgLen = maxDataLen;
    }
    semCInit(&msgQId->semMsgRx, semOpts, 0);
    semCInit(&msgQId->semMsgTx, semOpts, numMsg);
    return OK;
}

MSG_Q_ID msgQCreate(int numMsg, int maxMsgDataLen, int options)
{
    int rc;
    MSG_Q_ID msgQId;

    if (numMsg < 0 || maxMsgDataLen < 0 || options < 0) {
        return NULL;
    }
    msgQId = osMemAlloc(sizeof(*msgQId));
    if (NULL == msgQId) {
    }
    rc = msgQInit(msgQId, numMsg, maxMsgDataLen, options);
    if (OK == rc) {
        return msgQId;
    }
    osMemFree(msgQId);
    return NULL;
}

STATUS msgQSend(MSG_Q_ID msgQId, char *buf, UINT nBytes, int timeout, int priority)
{
    int rc;
    void *p;
    void **pp;
    msgQNode_t *msg;

    if (NULL == msgQId || NULL == buf || 0 == nBytes || timeout < WAIT_FOREVER) {
        return ERROR;
    }
    if (nBytes > msgQId->maxMsgLen) {
        return ERROR;
    }
    rc = semTake(&msgQId->semMsgTx, timeout);
    if (OK != rc) {
        return rc;
    }
    taskLock();
    msg = list_first_entry(&msgQId->qFree, msgQNode_t, qNode);
    while (NULL == msg) {;}
    msgQId->numFree--;
    list_del_init(&msg->qNode);
    p = (void *)msg;
    p += sizeof(*msg);

    msg->length = nBytes;
    if (MSG_Q_ZERO_COPY & msgQId->options) {
        pp = (void **)p;
        *pp = buf;
    } else {
        memcpy(p, buf, nBytes);
    }
    if (priority) {
        list_add(&msg->qNode, &msgQId->msgQ);
    } else {
        list_add_tail(&msg->qNode, &msgQId->msgQ);
    }
    taskUnlock();
    rc = semGive(&msgQId->semMsgRx);

    return rc;
}

int msgQReceive(MSG_Q_ID msgQId, char **pbuf, UINT maxNBytes, int timeout)
{
    int rc;
    void *p;
    void **pp;
    msgQNode_t *msg;

    if (NULL == msgQId || NULL == pbuf || 0 == maxNBytes || timeout < WAIT_FOREVER) {
        return ERROR;
    }
    if (!(MSG_Q_ZERO_COPY & msgQId->options)) {
        p = *pbuf;
        if (NULL == p) {
            return ERROR;
        }
    }
    rc = semTake(&msgQId->semMsgRx, timeout);
    if (OK != rc) {
        return 0;
    }
    taskLock();
    msg = list_first_entry(&msgQId->msgQ, msgQNode_t, qNode);
    while (NULL == msg) {;}
    list_del_init(&msg->qNode);
    p = (char *)msg;
    p += sizeof(*msg);

    if (msgQId->maxMsgLen < maxNBytes) {
        maxNBytes = msgQId->maxMsgLen;
    }
    if (maxNBytes > msg->length) {
        maxNBytes = msg->length;
    }
    if (MSG_Q_ZERO_COPY & msgQId->options) {
        pp = (void **)p;
        *pbuf = *pp;
    } else {
        memcpy(*pbuf, p, maxNBytes);
    }

    msgQId->numFree++;
    list_add_tail(&msg->qNode, &msgQId->qFree);

    taskUnlock();
    rc = semGive(&msgQId->semMsgTx);

    return maxNBytes;
}

STATUS msgQDelete(MSG_Q_ID msgQId)
{
    while (msgQId->numFree != msgQId->numMsgs) {
        taskDelay(2);
    }
    taskLock();
    osMemFree(msgQId->memBase);
    msgQId->memBase = NULL;
    memset(msgQId, 0, sizeof(*msgQId));
    osMemFree(msgQId);
    taskUnlock();
    return OK;
}

