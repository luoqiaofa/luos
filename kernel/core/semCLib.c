/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : semCLib.c
 * 内容摘要   :
 * 其它说明   :
 * 版本       :
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-05-29 11:19:05 AM
 *
 * 修改记录1:
 *    修改日期: 2023-05-29
 *    版 本 号:
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容:
 * ===========================================================================
 */

/******************************************************************************
 *                #include (依次为标准库头文件、非标准库头文件)               *
 ******************************************************************************/
#include "coreLib.h"

LOCAL BOOL semCountLibInstalled = false;

STATUS semCLibInit()
{
    SEM_OPS semOps = {
        .psemGive       = (semGive_t)semCGive,
        .psemTake       = (semTake_t)semCTake,
        .psemFlush      = (semGive_t)NULL,
        .psemGiveDefer  = (semGive_t)NULL,
        .psemFlushDefer = (semGive_t)NULL
    };
    semTypeInit(SEM_TYPE_COUNT, &semOps);
    if (OK == semLibInit()) {
        semCountLibInstalled = true;
    }
    return semCountLibInstalled ? OK : ERROR;
}

STATUS semCInit(SEM_ID semId, int options, int initialCount)
{
    memset(semId, 0, sizeof(*semId));
    semId->semType  = SEM_TYPE_COUNT;
    semId->options  = options;
    semId->semCount = initialCount;
    return semQInit(semId, options);
}

SEM_ID semCCreate(int options, int initialCount)
{
    SEM_ID semId;

    semId = osMemAlloc(sizeof(*semId));
    if (NULL == semId) {
        return NULL;
    }
    semCInit(semId, options, initialCount);
    return semId;
}

STATUS semCGive(SEM_ID semId)
{
    TCB_ID tcb;

    if (NULL == semId || semId->semType != SEM_TYPE_COUNT) {
        return ERROR;
    }

    taskLock();
    tcb = currentTask();
    semId->semCount++;
    if (list_empty(&semId->qPendHead)) {
        taskUnlock();
        return OK;
    }
    if (OK == taskPendQueGet(tcb, semId)) {
        taskUnlock();
        coreTrySchedule();
        return OK;
    }
    taskUnlock();
    return OK;
}

STATUS semCTake(SEM_ID semId, int timeout)
{
    // PriInfo_t *pri;
    TCB_ID tcb;
    LUOS_INFO *osInfo = osCoreInfo();

    if (NULL == semId || semId->semType != SEM_TYPE_COUNT) {
        return ERROR;
    }
    if (timeout < WAIT_FOREVER) {
        return ERROR;
    }
again:
    taskLock();
    if (semId->semCount > 0) {
        semId->semCount--;
        taskUnlock();
        return OK;
    }
    if (NO_WAIT == timeout) {
        taskUnlock();
        return ERROR;
    }
    tcb = currentTask();
    // pri = osInfo->priInfoTbl + tcb->priority;
    taskReadyRemove(tcb);
    tcb->status |= TASK_PEND;
    if (WAIT_FOREVER != timeout) {
        luosDelay(tcb, timeout);
        tcb->status |= TASK_DELAY;
    } else {
        list_add_tail(&tcb->qNodeSched, &(osInfo->qPendHead));
    }
    tcb->semIdPended = semId;
    taskPendQuePut(tcb, semId);
    taskUnlock();
    coreTrySchedule();
    if (OK == tcb->errCode) {
        goto again;
    }
    return tcb->errCode;
}

STATUS semCFlush(SEM_ID id)
{
    int num = 0;
    TLIST *node, *n2;
    TCB_ID tcb;

    if (NULL == id || SEM_TYPE_COUNT != id->semType) {
        return ERROR;
    }

    if (list_empty(&id->qPendHead)) {
        return OK;
    }
    n2 = NULL;
    taskLock();
    list_for_each(node, &id->qPendHead) {
        num++;
        if (NULL != n2) {
            list_del(n2);
            tcb = list_entry(n2, LUOS_TCB, qNodePend);
            tcb->status &= ~(TASK_PEND | TASK_DELAY);
            if (TASK_READY == tcb->status) {
                list_del_init(&tcb->qNodeSched);
                taskReadyAdd(tcb, true);
            }
            n2 = NULL;
        }
        n2 = node;
    }
    if (NULL != n2) {
        list_del(n2);
        tcb = list_entry(n2, LUOS_TCB, qNodePend);
        tcb->status &= ~(TASK_PEND | TASK_DELAY);
        if (TASK_READY == tcb->status) {
            list_del(&tcb->qNodeSched);
            taskReadyAdd(tcb, true);
        }
        n2 = NULL;
    }
    id->semCount = num;
    taskUnlock();
    return OK;
}

