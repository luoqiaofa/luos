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
        .psemFlush      = (semFlush_t)NULL,
        .psemGiveDefer  = (semGiveDefer_t)NULL,
        .psemFlushDefer = (semFlushDefer_t)NULL
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
    BOOL need_sch;
    // int level;
    TCB_ID tcb;

    if (NULL == semId || semId->semType != SEM_TYPE_MUTEX) {
        return ERROR;
    }

    // level = intLock();
    need_sch = false;
    taskLock();
    tcb = currentTask();
    if (tcb == semId->semOwner) {
        if (semId->recurse > 0) {
            semId->recurse--;
        }
        if (0 == semId->recurse) {
            semId->semOwner = NULL;
            if (OK == taskPendQueGet(tcb, semId)) {
                need_sch = true;
            }
            if (semId->oriPriority != tcb->priority) {
                need_sch = true;
                taskPrioritySet((tid_t)tcb, semId->oriPriority);
            }
        }
        // intUnlock(level);
    }
    taskUnlock();
    if (need_sch) {
        coreTrySchedule();
    }
    return OK;
}

STATUS semMTake(SEM_ID semId, int timeout)
{
    int rc;
    // int level;
    int newpri;
    /* PriInfo_t *pri; */
    TCB_ID tcb;
    LUOS_INFO *osInfo = osCoreInfo();

    if (timeout < WAIT_FOREVER) {
        return ERROR;
    }
again:
    // level = intLock();
    taskLock();
    tcb = currentTask();
    if (NULL == semId->semOwner) {
        semId->oriPriority = tcb->priority;
        semId->semOwner = tcb;
        semId->recurse = 1;
        // intUnlock(level);
        taskUnlock();
        return OK;
    } else if (tcb == semId->semOwner) {
        if (semId->recurse < SEM_METUX_RECURSE_MAX) {
            semId->recurse++;
            // intUnlock(level);
            taskUnlock();
            return OK;
        } else {
            // intUnlock(level);
            taskUnlock();
            return ERROR;
        }
    }
    if (NO_WAIT == timeout) {
        // intUnlock(level);
        taskUnlock();
        return ERROR;
    }
    /* pri = osInfo->priInfoTbl + tcb->priority; */
    taskReadyRemove(tcb);
    tcb->errCode = 0;
    tcb->status |= TASK_PEND;
    if (WAIT_FOREVER != timeout) {
        luosDelay(tcb, timeout);
        tcb->status |= TASK_DELAY;
    } else {
        list_add_tail(&tcb->qNodeSched, &(osInfo->qPendHead));
    }
    taskPendQuePut(tcb, semId);
    if (semId->semOwner->priority > tcb->priority) {
        newpri = tcb->priority;
        // intUnlock(level);
        rc = taskPrioritySet((tid_t)(semId->semOwner), newpri);
    } else {
        // intUnlock(level);
        // coreTrySchedule();
    }
    taskUnlock();
    coreTrySchedule();
    rc = tcb->errCode;
    if (OK == rc) {
        goto again;
    }
    return rc;
}

