/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : semMLib.c
 * 内容摘要   : 
 * 其它说明   : 
 * 版本       : 
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-05-29 06:20:46 PM
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

LOCAL BOOL semMLibInstalled = false;

STATUS semMLibInit()
{
    SEM_OPS semOps = {
        .psemGive       = (semGive_t)semMGive,
        .psemTake       = (semTake_t)semMTake,
        .psemFlush      = (semFlush_t)semFlush,
        .psemGiveDefer  = (semGiveDefer_t)semGiveDefer,
        .psemFlushDefer = (semFlushDefer_t)semFlushDefer
    };
    semTypeInit(SEM_TYPE_MUTEX, &semOps);
    if (OK == semLibInit()) {
        semMLibInstalled = true;
    }
    return semMLibInstalled ? OK : ERROR;
}

STATUS semMInit(SEM_ID semId, int options)
{
    memset(semId, 0, sizeof(*semId));
    semId->semType  = SEM_TYPE_MUTEX;
    semId->options  = options;
    semId->semOwner = NULL;
    return semQInit(semId, options);
}

SEM_ID semMCreate(int options)
{
    SEM_ID semId;

    semId = osMemAlloc(sizeof(*semId));
    if (NULL == semId) {
        return NULL;
    }
    semMInit(semId, options);
    return semId;
}

STATUS semMGive(SEM_ID semId)
{
    int level;
    TCB_ID tcb;

    if (NULL == semId || semId->semType != SEM_TYPE_MUTEX) {
        return ERROR;
    }
 
    level = intLock();
    tcb = currentTask();
    if (tcb == semId->semOwner) {
        if (semId->recurse > 0) {
            semId->recurse--;
        }
        if (0 == semId->recurse) {
            semId->semOwner = NULL;
            if (semId->oriPriority != tcb->priority) {
                intUnlock(level);
                return taskPrioritySet((tid_t)tcb, semId->oriPriority);
            }
            if (OK == taskPendQueGet(tcb, semId)) {
                coreTrySchedule();
            }
        }
        intUnlock(level);
        return OK;
    }
    return OK;
}

STATUS semMTake(SEM_ID semId, int timeout)
{
    int level;
    int newpri;
    /* PriInfo_t *pri; */
    TCB_ID tcb;
    LUOS_INFO *osInfo = osCoreInfo();

    if (timeout < WAIT_FOREVER) {
        return ERROR;
    }
    level = intLock();
    tcb = currentTask();
    if (NULL == semId->semOwner) {
        semId->oriPriority = tcb->priority;
        semId->semOwner = tcb;
        semId->recurse++;
        intUnlock(level);
        return OK;
    } else if (tcb == semId->semOwner) {
        if (semId->recurse < SEM_METUX_RECURSE_MAX) {
            semId->recurse++;
            intUnlock(level);
            return OK;
        } else {
            intUnlock(level);
            return ERROR;
        }
    }
    if (NO_WAIT == timeout) {
        intUnlock(level);
        return ERROR;
    }
    /* pri = osInfo->priInfoTbl + tcb->priority; */
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
    if (semId->semOwner->priority > tcb->priority) {
        newpri = tcb->priority;
        tcb = semId->semOwner;
        intUnlock(level);
        return taskPrioritySet((tid_t)tcb, newpri);
    } else {
        coreTrySchedule();
        intUnlock(level);
        return tcb->errCode;
    }
    return OK;
}

