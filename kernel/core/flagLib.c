/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : flagLib.c
 * 内容摘要   : 
 * 其它说明   : 
 * 版本       : 
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-06-19 01:35:55 PM
 * 
 * 修改记录1:
 *    修改日期: 2023-06-19
 *    版 本 号: 
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容: 
 * ===========================================================================
 */

/******************************************************************************
 *                #include (依次为标准库头文件、非标准库头文件)               *
 ******************************************************************************/
#include "coreLib.h"

LOCAL BOOL flagLibInstalled = false;

STATUS flagLibInit(void)
{
    return flagLibInstalled ? OK : ERROR;
}

STATUS flagInit(SEM_ID semId, int options, UINT flags)
{
    memset(semId, 0, sizeof(*semId));
    semId->semType   = SEM_TYPE_FLAGS;
    semId->options   = options;
    semId->semEvents = flags;
    semQInit(semId, options);
    return OK;
}

SEM_ID flagCreate(int options, UINT flags)
{
    SEM_ID semId;

    semId = osMemAlloc(sizeof(*semId));
    if (NULL == semId) {
        return NULL;
    }
    flagInit(semId, options, flags);
    return semId;
}

STATUS flagGive(SEM_ID semId, UINT flags, int flgOptions)
{
    int level;
    TCB_ID tcb;

    if (NULL == semId || semId->semType != SEM_TYPE_FLAGS) {
        return ERROR;
    }

    level = intLock();
    tcb = currentTask();
    switch (flgOptions) {
        case FLAG_OPT_SET_ALL:
            semId->semEvents |= flags;
            break;
        case FLAG_OPT_CLR_ALL:
            semId->semEvents &= ~flags;
            break;
        default:
            return ERROR;
            break;
    }
    if (list_empty(&semId->qPendHead)) {
        intUnlock(level);
        return OK;
    }
    if (OK == taskPendQueGet(tcb, semId)) {
        intUnlock(level);
        coreTrySchedule();
        return OK;
    }
    intUnlock(level);
    return OK;
}

STATUS flagTake(SEM_ID semId, UINT flags, int flgOptions, int timeout)
{
    int level;
    UINT newflags;
    UINT updFlags = 0;
    // PriInfo_t *pri;
    TCB_ID tcb;
    LUOS_INFO *osInfo = osCoreInfo();
    BOOL willPended = false; 
    if (NULL == semId || semId->semType != SEM_TYPE_FLAGS) {
        return ERROR;
    }
    if (timeout < WAIT_FOREVER) {
        return ERROR;
    }
again:
    willPended = false; 
    level = intLock();
    tcb = currentTask();
    switch (tcb->flgOptions & FLAG_OPT_SETCLR_MSK) {
        case FLAG_OPT_SET_ALL:
            newflags = semId->semEvents & flags;
            if (newflags != flags) {
                willPended = true; 
            } else {
                if (tcb->flgOptions & FLAG_OPT_CONSUME) {
                    updFlags = (semId->semEvents & ~flags); 
                }
            }
            break;
        case FLAG_OPT_CLR_ALL:
            newflags = semId->semEvents & flags;
            if (0 != newflags) {
                willPended = true; 
            } else {
                if (tcb->flgOptions & FLAG_OPT_CONSUME) {
                    updFlags = (semId->semEvents | flags); 
                }
            }
            break;
        case FLAG_OPT_SET_ANY:
            newflags = semId->semEvents & flags;
            if (0 == newflags) {
                willPended = true; 
            } else {
                if (tcb->flgOptions & FLAG_OPT_CONSUME) {
                    updFlags = (semId->semEvents & ~flags); 
                }
            }
            break;
        case FLAG_OPT_CLR_ANY:
            newflags = semId->semEvents & flags;
            if (newflags == flags) {
                willPended = true; 
            } else {
                if (tcb->flgOptions & FLAG_OPT_CONSUME) {
                    updFlags = (semId->semEvents | flags); 
                }
            }
            break;
        default:
            intUnlock(level);
            return ERROR;
            break;
    }
    if (!willPended) {
        if (tcb->flgOptions & FLAG_OPT_CONSUME) {
           semId->semEvents = updFlags;
        }
        intUnlock(level);
        return OK;
    }
    if (NO_WAIT == timeout) {
        intUnlock(level);
        return ERROR;
    }
    // pri = osInfo->priInfoTbl + tcb->priority;
    tcb->flgsWaited = flags;
    tcb->flgOptions = flgOptions;
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
    intUnlock(level);
    coreTrySchedule();
    if (OK == tcb->errCode) {
        goto again;
    }
    return tcb->errCode;
}

STATUS flagFlush(SEM_ID id)
{
    int num = 0;
    int level;
    TLIST *node, *n2;
    TCB_ID tcb;

    if (NULL == id || SEM_TYPE_FLAGS != id->semType) {
        return ERROR;
    }

    if (list_empty(&id->qPendHead)) {
        return OK;
    }
    n2 = NULL;
    level = intLock();
    list_for_each(node, &id->qPendHead) {
        num++;
        if (NULL != n2) {
            list_del_init(n2);
            tcb = list_entry(n2, LUOS_TCB, qNodePend);
            tcb->status = tcb->status & ~(TASK_PEND | TASK_DELAY);
            if (TASK_READY == tcb->status) {
                list_del_init(&tcb->qNodeSched);
                taskReadyAdd(tcb, true);
            }
            n2 = NULL;
        }
        n2 = node;
    }
    if (NULL != n2) {
        list_del_init(n2);
        tcb = list_entry(n2, LUOS_TCB, qNodePend);
        tcb->status = tcb->status & ~(TASK_PEND | TASK_DELAY);
        if (TASK_READY == tcb->status) {
            list_del(&tcb->qNodeSched);
            taskReadyAdd(tcb, true);
        }
        n2 = NULL;
    }
    id->semEvents = num;
    intUnlock(level);
    coreTrySchedule();
    return OK;
}


