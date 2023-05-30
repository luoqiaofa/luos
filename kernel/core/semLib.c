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
#include <stdio.h>
#include <stdlib.h>
#include "taskLib.h"
#include "taskLibP.h"
#include "semLibP.h"

LOCAL struct sem_ops semOpsTbl[8];
LOCAL BOOL semLibInstalled = false;

STATUS semLibInit()
{
    int idx;

    if (!semLibInstalled) {
        memset(semOpsTbl, 0, sizeof(semOpsTbl));
        for (idx = 0; idx = ARRAY_SIZE(semOpsTbl); idx++) {
            semOpsTbl[idx].psemGive       = (semGive_t)semInvalid;
            semOpsTbl[idx].psemTake       = (semGive_t)semInvalid;
            semOpsTbl[idx].psemFlush      = (semGive_t)semInvalid;
            semOpsTbl[idx].psemGiveDefer  = (semGive_t)semInvalid;
            semOpsTbl[idx].psemFlushDefer = (semGive_t)semInvalid;
        }
        semLibInstalled = true;
    }
    return OK;
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
    if (id->semType >= SEM_TYPE_MAX) {
        return ERROR;
    }
    return semOpsTbl[id->semType].psemGive(id);
}

STATUS semTake(SEM_ID id, int timeout)
{
    if (id->semType >= SEM_TYPE_MAX) {
        return ERROR;
    }
    return semOpsTbl[id->semType].psemTake(id, timeout);
}


STATUS semFlush(SEM_ID id)
{
    if (id->semType >= SEM_TYPE_MAX) {
        return ERROR;
    }
    return semOpsTbl[id->semType].psemFlush(id);
}

STATUS semGiveDefer(SEM_ID id)
{
    if (id->semType >= SEM_TYPE_MAX) {
        return ERROR;
    }
    return semOpsTbl[id->semType].psemGiveDefer(id);
}

STATUS semFlushDefer(SEM_ID id)
{
    if (id->semType >= SEM_TYPE_MAX) {
        return ERROR;
    }
    return semOpsTbl[id->semType].psemFlushDefer(id);
}

STATUS semDestroy(SEM_ID semId, BOOL dealloc)
{
    return 0;
}

STATUS semDelete(SEM_ID semId)
{
    return semDestroy(semId, true);
}

