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
#include "semLibP.h"

LOCAL BOOL semMinLibInstalled = false;

STATUS semMLibInit()
{
    SEM_OPS semCountOps = {
        .psemGive       = (semGive_t)semMGive;
        .psemTake       = (semGive_t)semMTake;
        .psemFlush      = (semGive_t)semFlush;
        .psemGiveDefer  = (semGive_t)semGiveDefer;
        .psemFlushDefer = (semGive_t)semFlushDefer;
    };
    semTypeInit(SEM_TYPE_MUTEX, &semCountOps);
    semMinLibInstalled = true;
    return OK;
}

SEM_ID semMInit(SEM_ID semId, int options)
{
    if (SEM_EMPTY != initialState && SEM_FULL != initialState) {
        return ERROR;
    }
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
        return ERROR;
    }
    return semMInit(semId, options);
}

STATUS semMGive(SEM_ID semId)
{
    int level;
    int newpri;
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
                newpri = semId->oriPriority;
                intUnlock(level);
                return taskPrioritySet(tcb, semId->oriPriority);
            }
            if (taskPendQueGet(tcb, semId)) {
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
    ULONG grp;
    ULONG off;
    ULONG priority;
    PriInfo_t *pri;
    TCB_ID tcb, tcb1;
    TLIST *node;
    LUOS_INFO *osInfo = osCoreInfo();

    if (timeout < WAIT_FOREVER) {
        return ERROR;
    }
    level = intLock();
    tcb = currentTask();
    if (NULL == semId->semOwner) {
        semId->oriPriority = tcp->priority;
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
    pri = osInfo->priInfoTbl + tcb->priority;
    list_del(&tcb->qOsSched);
    pri->numTask--;
    tcb->status |= TASK_PEND;
    if (WAIT_FOREVER != timeout) {
        tcb->dlyTicks = timeout;
        tcb->status |= TASK_DELAY;
        list_add_tail(&tcb->qOsSched, &(osInfo->qDelayHead));
    } else {
        list_add_tail(&tcb->qOsSched, &(osInfo->qPendHead));
    }
    taskPendQuePut(tcb, semId);
    tcb->semIdPended = semId;
    if (0 == pri->numTask) {
        grp = priorityGroup(tcb->priority);
        off = priorityOffset(tcb->priority);
        osInfo->readyPriGrp      &= ~(1 << grp);
        osInfo->readyPriTbl[grp] &= ~(1 << off);
    }
    if (semId->semOwner->priority > tcb->priority) {
        newpri = tcb->priority;
        tcb = semId->semOwner;
        intUnlock(level);
        return taskPrioritySet(tcb, newpri);
    } else {
        coreTrySchedule();
        intUnlock(level);
        return tcb->errCode;
    }
    return OK;
}

