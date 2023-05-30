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
#include <stdio.h>
#include <stdlib.h>
#include "semLib.h"

LOCAL BOOL semCountLibInstalled = false;

STATUS semCLibInit()
{
    SEM_OPS semCountOps = {
        .psemGive       = (semGive_t)semCGive;
        .psemTake       = (semGive_t)semCTake;
        .psemFlush      = (semGive_t)semFlush;
        .psemGiveDefer  = (semGive_t)semGiveDefer;
        .psemFlushDefer = (semGive_t)semFlushDefer;
    };
    semTypeInit(SEM_TYPE_COUNT, &semCountOps);
    semCountLibInstalled = true;
    return OK;
}

SEM_ID semCInit(SEM_ID semId, int options, int initialCount)
{
    memset(semId, 0, sizeof(*semId));
    semId->semType  = SEM_TYPE_COUNT;
    semId->options  = options;
    semId->semCount = initialCount;
    return semQInit(semId, options);
    return OK;
}

SEM_ID semCCreate(int options, int initialCount)
{
    SEM_ID semId;

    semId = osMemAlloc(sizeof(*semId));
    if (NULL == semId) {
        return ERROR;
    }
    return semCInit(semId, options, initialCount);
}

STATUS semCGive(SEM_ID semId)
{
    int level;
    TCB_ID tcb;

    if (NULL == semId || semId->semType != SEM_TYPE_COUNT) {
        return ERROR;
    }
    
    level = intLock();
    if (list_empty(&semId->qPendHead)) {
        intUnlock(level);
        return OK;
    }
    tcb = currentTask();
    semId->semCount++;
    if (taskPendQueGet(tcb, semId)) {
        coreTrySchedule();
    }
    intUnlock(level);
    return OK;
}

STATUS semCTake(SEM_ID semId, int timeout)
{
    int level;
    ULONG grp;
    ULONG off;
    ULONG priority;
    PriInfo_t *pri;
    TCB_ID tcb, tcb1;
    TLIST *node;
    LUOS_INFO *osInfo = osCoreInfo();

    if (NULL == semId || semId->semType != SEM_TYPE_COUNT) {
        return ERROR;
    }
    if (timeout < WAIT_FOREVER) {
        return ERROR;
    }
    level = intLock();
    if (semId->semCount > 0) {
        semId->semCount--;
        intUnlock(level);
        return OK;
    }
    if (NO_WAIT == timeout) {
        intUnlock(level);
        return ERROR;
    }
    tcb = currentTask();
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
    coreTrySchedule();
    intUnlock(level);
    return tcb->errCode;
}
