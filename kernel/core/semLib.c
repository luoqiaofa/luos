/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : semLib.c
 * 内容摘要   :
 * 其它说明   :
 * 版本       :
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-05-29 09:52:41 AM
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

LOCAL struct sem_ops semOpsTbl[SEM_TYPE_MAX] = {
    [SEM_TYPE_BINARY] = {
        .psemGive       = (semGive_t)semInvalid,
        .psemTake       = (semTake_t)semInvalid,
        .psemFlush      = (semFlush_t)semInvalid,
        .psemGiveDefer  = NULL,
        .psemFlushDefer = NULL,
    },
    [SEM_TYPE_MUTEX]  = {
        .psemGive       = (semGive_t)semInvalid,
        .psemTake       = (semTake_t)semInvalid,
        .psemFlush      = (semFlush_t)semInvalid,
        .psemGiveDefer  = NULL,
        .psemFlushDefer = NULL,
    },
    [SEM_TYPE_COUNT]  = {
        .psemGive       = (semGive_t)semInvalid,
        .psemTake       = (semTake_t)semInvalid,
        .psemFlush      = (semFlush_t)semInvalid,
        .psemGiveDefer  = NULL,
        .psemFlushDefer = NULL,
    },
    [SEM_TYPE_FLAGS]  = {
        .psemGive       = (semGive_t)semInvalid,
        .psemTake       = (semTake_t)semInvalid,
        .psemFlush      = (semFlush_t)semInvalid,
        .psemGiveDefer  = NULL,
        .psemFlushDefer = NULL,
    },
};
LOCAL BOOL semLibInstalled = false;

STATUS semLibInit(void)
{
    if (!semLibInstalled) {
        semLibInstalled = true;
    }
    return semLibInstalled ? OK : ERROR;
}

STATUS semTypeInit(int semtype, SEM_OPS *ops)
{
    if (semtype >= SEM_TYPE_MAX) {
        return ERROR;
    }
    semOpsTbl[semtype].psemGive       = ops->psemGive;
    semOpsTbl[semtype].psemTake       = ops->psemTake;
    semOpsTbl[semtype].psemFlush      = ops->psemFlush;
    semOpsTbl[semtype].psemGiveDefer  = ops->psemGiveDefer;
    semOpsTbl[semtype].psemFlushDefer = ops->psemFlushDefer;
    return OK;
}

STATUS semQInit(SEM_ID id, int options)
{
    if (id->semType >= SEM_TYPE_MAX) {
        return ERROR;
    }
    switch (options & SEM_Q_MASK) {
        case SEM_Q_FIFO:
        case SEM_Q_PRIORITY:
            INIT_LIST_HEAD(&id->qPendHead);
            break;
    }
    return OK;
}

STATUS semInvalid(SEM_ID id) {
    return ERROR;
}

STATUS semGive(SEM_ID id)
{
    if (NULL == id || id->semType >= SEM_TYPE_MAX) {
        return ERROR;
    }
    return semOpsTbl[id->semType].psemGive(id);
}

STATUS semTake(SEM_ID id, int timeout)
{
    if (NULL == id || id->semType >= SEM_TYPE_MAX) {
        return ERROR;
    }
    return semOpsTbl[id->semType].psemTake(id, timeout);
}


STATUS semFlush(SEM_ID id)
{
    int num = 0;
    TLIST *node;
    UINT oldStatus;
    TCB_ID tcb = NULL;

    if (NULL == id || id->semType >= SEM_TYPE_MAX) {
        return ERROR;
    }

    if (NULL != semOpsTbl[id->semType].psemFlush) {
        if (semFlush != semOpsTbl[id->semType].psemFlush) {
            return semOpsTbl[id->semType].psemFlush(id);
        }
    }
    taskLock();
    if (list_empty(&id->qPendHead)) {
        taskUnlock();
        return OK;
    }
    tcb = NULL;
    list_for_each(node, &id->qPendHead) {
        num++;
        if (NULL != tcb) {
            list_del_init(&tcb->qNodePend);
            if (oldStatus & TASK_DELAY) {
                luosQDelayRemove(tcb);
            } else {
                luosQPendRemove(tcb);
            }
            if (TASK_READY == tcb->status) {
                taskReadyAdd(tcb, true);
            } else {
                luosQPendAdd(tcb, true);
            }
            tcb = NULL;
        }
        tcb = list_entry(node, LUOS_TCB, qNodePend);
        oldStatus = tcb->status;
        tcb->status &= ~(TASK_PEND | TASK_DELAY);
    }
    if (NULL != tcb) {
        list_del_init(&tcb->qNodePend);
        if (oldStatus & TASK_DELAY) {
            luosQDelayRemove(tcb);
        } else {
            luosQPendRemove(tcb);
        }
        if (TASK_READY == tcb->status) {
            taskReadyAdd(tcb, true);
        } else {
            luosQPendAdd(tcb, true);
        }
        tcb = NULL;
    }
    if (SEM_TYPE_COUNT == id->semType) {
        id->semCount = num;
    }
    taskUnlock();
    coreTrySchedule();
    return OK;
}

STATUS semGiveDefer(SEM_ID id)
{
    if (NULL == id || id->semType >= SEM_TYPE_MAX) {
        return ERROR;
    }
    if (NULL != semOpsTbl[id->semType].psemGiveDefer) {
        return semOpsTbl[id->semType].psemGiveDefer(id);
    }
    return ERROR;
}

STATUS semFlushDefer(SEM_ID id)
{
    if (NULL == id || id->semType >= SEM_TYPE_MAX) {
        return ERROR;
    }
    if (NULL != semOpsTbl[id->semType].psemFlushDefer) {
        return semOpsTbl[id->semType].psemFlushDefer(id);
    }
    return ERROR;
}

STATUS semDestroy(SEM_ID semId, BOOL dealloc)
{
    if (NULL == semId || semId->semType >= SEM_TYPE_MAX) {
        return ERROR;
    }
    return 0;
}

STATUS semDelete(SEM_ID semId)
{
    if (NULL == semId || semId->semType >= SEM_TYPE_MAX) {
        return ERROR;
    }
    return semDestroy(semId, true);
}

