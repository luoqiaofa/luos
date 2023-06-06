/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : semBLib.c
 * 内容摘要   : 
 * 其它说明   : 
 * 版本       : 
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-05-29 04:42:29 PM
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

LOCAL BOOL semBinLibInstalled = false;

STATUS semBLibInit()
{
    SEM_OPS semOps = {
        .psemGive       = (semGive_t)semBGive,
        .psemTake       = (semTake_t)semBTake,
        .psemFlush      = (semGive_t)semFlush,
        .psemGiveDefer  = (semGive_t)semGiveDefer,
        .psemFlushDefer = (semGive_t)semFlushDefer,
    };
    semTypeInit(SEM_TYPE_BINARY, &semOps);
    if (OK == semLibInit()) {
        semBinLibInstalled = true;
    }
    return semBinLibInstalled ? OK : ERROR;
}

STATUS semBInit(SEM_ID semId, int options, SEM_B_STATE initialState)
{
    if (SEM_EMPTY != initialState && SEM_FULL != initialState) {
        return ERROR;
    }
    memset(semId, 0, sizeof(*semId));
    semId->semType  = SEM_TYPE_BINARY;
    semId->options  = options;
    semId->semCount = initialState;
    return semQInit(semId, options);
}

SEM_ID semBCreate(int options, SEM_B_STATE initialState)
{
    SEM_ID semId;

    if (SEM_EMPTY != initialState && SEM_FULL != initialState) {
        return NULL;
    }

    semId = osMemAlloc(sizeof(*semId));
    if (NULL == semId) {
        return NULL;
    }
    semBInit(semId, options, initialState);
    return semId;
}

STATUS semBGive(SEM_ID semId)
{
    int level;
    TCB_ID tcb;

    if (NULL == semId || semId->semType != SEM_TYPE_BINARY) {
        return ERROR;
    }
    if (SEM_EMPTY != semId->recurse && SEM_FULL != semId->recurse) {
        return ERROR;
    }
    
    level = intLock();
    tcb = currentTask();
    if (tcb == semId->semOwner) {
        if (SEM_EMPTY == semId->recurse) {
            semId->recurse = SEM_FULL;
            semId->semOwner = NULL;
        }
        if (list_empty(&semId->qPendHead)) {
            intUnlock(level);
            return OK;
        }
        if (OK == taskPendQueGet(tcb, semId)) {
            coreTrySchedule();
        }
        intUnlock(level);
        return OK;
    }
    return OK;
}

STATUS semBTake(SEM_ID semId, int timeout)
{
    int level;
    // PriInfo_t *pri;
    TCB_ID tcb;
    LUOS_INFO *osInfo = osCoreInfo();

    if (SEM_EMPTY != semId->recurse && SEM_FULL != semId->recurse) {
        return ERROR;
    }
    if (timeout < WAIT_FOREVER) {
        return ERROR;
    }
    level = intLock();
    tcb = currentTask();
    if (NULL == semId->semOwner) {
        semId->semOwner = tcb;
        semId->recurse  = SEM_EMPTY;
        intUnlock(level);
        return OK;
    }
    if (NO_WAIT == timeout) {
        intUnlock(level);
        return ERROR;
    }
    // pri = osInfo->priInfoTbl + tcb->priority;
    list_del(&tcb->qNodeSched);
    taskReadyRemove(tcb);
    tcb->status |= TASK_PEND;
    if (WAIT_FOREVER != timeout) {
        tcb->dlyTicks = timeout;
        tcb->status |= TASK_DELAY;
        list_add_tail(&tcb->qNodeSched, &(osInfo->qDelayHead));
    } else {
        list_add_tail(&tcb->qNodeSched, &(osInfo->qPendHead));
    }
    taskPendQuePut(tcb, semId);
    coreTrySchedule();
    intUnlock(level);
    return tcb->errCode;
}

