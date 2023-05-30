/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : coreLib.c
 * 内容摘要   : 
 * 其它说明   : 
 * 版本       : 
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-05-24 04:59:50 PM
 * 
 * 修改记录1:
 *    修改日期: 2023-05-24
 *    版 本 号: 
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容: 
 * ===========================================================================
 */

/******************************************************************************
 *                #include (依次为标准库头文件、非标准库头文件)               *
 ******************************************************************************/
#include "coreLib.h"

#ifndef CONFIG_TASK_MEM_POOL_SIZE
#define CONFIG_TASK_MEM_POOL_SIZE (30*1024)
#endif

LUOS_INFO __osinfo__;

LOCAL MEM_BLK  osMemPool[CONFIG_TASK_MEM_POOL_SIZE/sizeof(MEM_BLK)];
LOCAL MEM_PART osMemPart;
LOCAL PART_ID  osMemPartId = NULL;
LOCAL BOOL coreLibInstalled = false;

#ifndef CONFIG_SLICE_MS
#define CONFIG_SLICE_MS   20 /* 20MS */
#endif

STATUS coreLibInit()
{
    STATUS rc;
    int idx;
    LUOS_INFO *pInfo;
    PriInfo_t *pri;
    char *mem_pool = (char *)&osMemPool[0]; 
    size_t size = CONFIG_TASK_MEM_POOL_SIZE;
    size_t blksize = MIN_BLK_SIZE;

    osMemPartId = &osMemPart;
    memset(osMemPartId, 0, sizeof(*osMemPartId));
    rc = memPartInit(osMemPartId, mem_pool, size, blksize);
    if (0 == rc) {
        coreLibInstalled = true;
    }
    pInfo = &__osinfo__;
    memset(pInfo, 0, sizeof(*pInfo));
    pInfo->sliceTicks  = CONFIG_SLICE_MS * sysClkRateGet() / 1000;
    if (pInfo->sliceTicks < 1) {
        pInfo->sliceTicks = 1;
    }
    INIT_LIST_HEAD(&pInfo->qDelayHead);
    INIT_LIST_HEAD(&pInfo->qPendHead);
    for (idx = 0; idx < CONFIG_NUM_PRIORITY; idx++) {
        pri = pInfo->priInfoTbl + idx;
        pri->schedPolicy = SCHED_RR;
        pri->sliceTicks  = pInfo->sliceTicks;
        INIT_LIST_HEAD(&pri->qReadyHead);
    }
    return rc;
}

void * osMemAlloc(size_t nbytes)
{
    if (coreLibInstalled) {
        return memPartAlloc(osMemPartId, nbytes);
    }
    return NULL;
}

STATUS osMemFree(void *ptr)
{
    if (coreLibInstalled) {
        return memPartFree(osMemPartId, ptr);
    }
    return 0;
}

STATUS coreTickDoing()
{
    STATUS rc;
    int idx;
    TLIST *node, *node_del;
    LUOS_INFO *osInfo;
    PriInfo_t *pri;
    LUOS_TCB *tcb;
    ULONG grp;
    ULONG off;

    node_del = NULL;
    osInfo = &__osinfo__;
    list_for_each(node, &osInfo->qDelayHead) {
        if (NULL != node_del) {
            list_del(node_del);
            tcb = list_entry(node_del, LUOS_TCB, qOsSched); 
            pri = osInfo->priInfoTbl + tcb->priority;
            list_add_tail(node_del, &pri->qReadyHead);
            pri->numTask++;
            node_del = NULL;
        }
        tcb = list_entry(node, LUOS_TCB, qOsSched); 
        if (tcb->dlyTicks > 0) {
            tcb->dlyTicks--;
            if (0 == tcb->dlyTicks) {
                tcb->status &= ~TASK_DELAY;
                if (TASK_READY == tcb->status) {
                    off = tcb->priority % BITS_PER_LONG;
                    grp = tcb->priority / BITS_PER_LONG;
                    osInfo->readyPriGrp |= (1 << grp);
                    osInfo->readyPriTbl[grp] |= (1 << off);
                    node_del = node;
                } else {
                    /* pend envent timeout */
                }
            }
        }
    }
    if (NULL != node_del) {
        list_del(node_del);
        tcb = list_entry(node_del, LUOS_TCB, qOsSched); 
        pri = osInfo->priInfoTbl + tcb->priority;
        list_add_tail(node_del, &pri->qReadyHead);
        pri->numTask++;
        node_del = NULL;
    }
    tcb = currentTask();
    pri = osInfo->priInfoTbl + tcb->priority;
    if (SCHED_RR == pri->schedPolicy) {
        if (tcb->sliceTicksCnt < pri->sliceTicks) {
            tcb->sliceTicksCnt++;
        }
    }
    if (0 == tcb->lockCnt) {
        if (pri->numTask > 1) {
            if (SCHED_RR == pri->schedPolicy) {
                if (tcb->sliceTicksCnt >= pri->sliceTicks) {
                    list_del(&tcb->qOsSched);
                    list_add_tail(&tcb->qOsSched, &pri->qReadyHead);
                    tcb->sliceTicksCnt = 0;
                }
            } /* SCHED_RR == pri->schedPolicy */
        } /* (pri->numTask > 1) */ else {
            /* only one task in the priority ready table */
        }
        /* find the highest ready priority , then shedule */
        coreTrySchedule();
    }
}

void coreTrySchedule()
{
    LUOS_TCB *tcb;
    LUOS_INFO *osInfo = &__osinfo__;
    ULONG grp = cpuCntLeadZeros(osInfo->readyPriGrp);
    ULONG off = cpuCntLeadZeros(osInfo->readyPriTbl[grp]);
    ULONG priority = grp * BITS_PER_LONG + off;
    PriInfo_t *pri = osInfo->priInfoTbl + priority;
    
    tcb = list_first_entry(&pri->qReadyHead, LUOS_TCB, qOsSched);
    if (tcb != currentTask()) {
        /* start scheduled */
#if 0
        cpuTaskContextSwitchTrig();
#endif
    }
}

