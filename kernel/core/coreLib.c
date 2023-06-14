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

extern int cpuCntLeadZeros(cpudata_t val);

STATUS coreLibInit(void)
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

#define NUM_TICK_JOBS 256
typedef STATUS (*tickAnnounce_t)(void);
static tickAnnounce_t coreTickJobsTbl[NUM_TICK_JOBS];
volatile uint8_t tickQworkWrIdx = 0;
volatile uint8_t tickQWorkRdIdx = 0;
volatile int16_t numTocksQWork = 0;

#define tickQWorkEmpey() (tickQworkWrIdx == tickQWorkRdIdx)
void tickAnnounce(void)
{
    TCB_ID tcb = currentTask();
    if (0 == tcb->lockCnt) {
        tickQWorkDoing();
        coreTickDoing();
    } else {
        numTocksQWork++;
        coreTickJobsTbl[tickQworkWrIdx++] = coreTickDoing;
    }
}

STATUS tickQWorkDoing(void)
{
    int num = 0;
    tickAnnounce_t pfunc;

    while (!tickQWorkEmpey()) {
        pfunc = coreTickJobsTbl[tickQWorkRdIdx++];
        pfunc();
        num++;
        numTocksQWork--;
    }
    /* maybe need ched 0 == numTocksQWork or not for overflow */
    return num;
}

STATUS coreTickDoing(void)
{
    TLIST *node, *node_del;
    LUOS_INFO *osInfo;
    PriInfo_t *pri;
    TCB_ID tcb = currentTask();

    if (tcb->lockCnt > 0) {
        return ERROR;
    }

    osInfo = &__osinfo__;
    osInfo->sysTicksCnt++;
    timerListDing();
    tcb->runTicksCnt++;
    node_del = NULL;
    list_for_each(node, &osInfo->qDelayHead) {
        if (NULL != node_del) {
            list_del(node_del);
            tcb = list_entry(node_del, LUOS_TCB, qNodeSched);
            pri = osInfo->priInfoTbl + tcb->priority;
            list_add_tail(node_del, &pri->qReadyHead);
            taskReadyAdd(tcb);
            node_del = NULL;
        }
        tcb = list_entry(node, LUOS_TCB, qNodeSched);
        if (tcb->dlyTicks > 0) {
            tcb->dlyTicks--;
            if (0 == tcb->dlyTicks) {
                tcb->status &= ~TASK_DELAY;
                if (TASK_READY == tcb->status) {
                    node_del = node;
                    tcb->errCode = 0;
                } else {
                    /* pend envent timeout */
                    tcb->errCode = ERROR;
                }
            }
        }
    }
    if (NULL != node_del) {
        list_del(node_del);
        tcb = list_entry(node_del, LUOS_TCB, qNodeSched);
        pri = osInfo->priInfoTbl + tcb->priority;
        list_add_tail(node_del, &pri->qReadyHead);
        taskReadyAdd(tcb);
        node_del = NULL;
    }
    tcb = currentTask();
    pri = osInfo->priInfoTbl + tcb->priority;
    if (SCHED_RR == pri->schedPolicy) {
        if (tcb->sliceTicksCnt < pri->sliceTicks) {
            tcb->sliceTicksCnt++;
        }
    }
    if (tcb->lockCnt > 0) {
        return ERROR;
    }
    if (pri->numTask > 1) {
        if (SCHED_RR == pri->schedPolicy) {
            if (tcb->sliceTicksCnt >= pri->sliceTicks) {
                list_del(&tcb->qNodeSched);
                list_add_tail(&tcb->qNodeSched, &pri->qReadyHead);
                tcb->sliceTicksCnt = 0;
            }
        } /* SCHED_RR == pri->schedPolicy */
    } /* (pri->numTask > 1) */ else {
        /* only one task in the priority ready table */
    }
    return 0;
}

static inline TCB_ID highReadyTaskGet(void)
{
    int grp;
    int off;
    LUOS_TCB *tcb;
    PriInfo_t *pri;
    int  priority;
    LUOS_INFO *osInfo;

    osInfo = &__osinfo__;
    grp = cpuCntLeadZeros(osInfo->readyPriGrp);
    while (grp >= NLONG_PRIORITY) {
        ;/* hang here */
    }
    off = cpuCntLeadZeros(osInfo->readyPriTbl[grp]);
    while (off >= BITS_PER_LONG) {
       ;/* hang here */
    }
    priority = grp * BITS_PER_LONG + off;
    pri = __osinfo__.priInfoTbl + priority;
    while (list_empty(&pri->qReadyHead)) {
        ;/* hang here */
    }
    tcb = list_first_entry(&pri->qReadyHead, LUOS_TCB, qNodeSched);
    while (NULL == tcb) {;/* hang here */}
    return tcb;
}

void coreTrySchedule(void)
{
    LUOS_TCB *tcb;
    int level;

    level = intLock();
    if (0 == __osinfo__.intNestedCnt) {
        tickQWorkDoing();
    }
    tcb = currentTask();
    if (tcb->lockCnt > 0) {
        intUnlock(level);
        return;
    }

    tcb = highReadyTaskGet();
    if (currentTask() != tcb) {
        __osinfo__.highestTcb = tcb;
        if (0 == __osinfo__.intNestedCnt) {
            intUnlock(level);
            cpuTaskContextSwitchTrig(currentTask(), tcb);
            return;
        }
    }
    intUnlock(level);
}

void coreIntEnter(void)
{
    osCoreInfo()->intNestedCnt++;
}

void coreIntExit(void)
{
    coreTrySchedule();
    osCoreInfo()->intNestedCnt--;
}

void coreContextHook(void)
{
    TCB_ID tcb = currentTask();
    LUOS_INFO *info = osCoreInfo();

    info->contextCnt++;
    tcb->latestTick = info->sysTicksCnt;
    if (0 == tcb->firstSchedTs) {
        tcb->firstSchedTs = info->sysTicksCnt;
    }
    tcb->schedCnt++;
}

uint32_t sysClkTickGet(void)
{
    return osCoreInfo()->sysTicksCnt;
}

BOOL coreScheduleIsEna(void)
{
    return osCoreInfo()->schedLocked;
}

void coreScheduleEnable(void)
{
    osCoreInfo()->schedLocked = true;
}

void coreScheduleDisable(void)
{
    osCoreInfo()->schedLocked = false;
}

#include <stdio.h>
#define log(fmt, args...) printf(fmt " \n", ## args)
STATUS i(void)
{
    TCB_ID tcb;
    TCB_ID tcbs;
    TLIST *node;
    int priority;
    PriInfo_t *pri;
    cpudata_t grp, off;
    LUOS_INFO *osInfo = osCoreInfo();
    uint32_t ntick;
    uint32_t idx, num;

    num = 0;
    taskLock();
    for(grp = 0; grp < NLONG_PRIORITY; grp++) {
        if (0 != osInfo->readyPriTbl[grp]) {
            for (off = 0; off < BITS_PER_LONG; off++) {
                if (osInfo->readyPriTbl[grp] & (1 << (BITS_PER_LONG - 1 - off))) {
                    priority = grp * BITS_PER_LONG + off;
                    pri = osInfo->priInfoTbl + priority;
                    num += pri->numTask;
                }
            }
        }
    }
    list_for_each(node, &osInfo->qDelayHead) {
        num++;
    }
    list_for_each(node, &osInfo->qPendHead) {
        num++;
    }
    tcbs = osMemAlloc(num * sizeof(*tcb));
    if (NULL == tcbs) {
        taskUnlock();
        log("[%s] osMemAlloc failed", __func__);
        return 0;
    }
    idx = 0;
    for(grp = 0; grp < NLONG_PRIORITY; grp++) {
        if (0 != osInfo->readyPriTbl[grp]) {
            for (off = 0; off < BITS_PER_LONG; off++) {
                if (osInfo->readyPriTbl[grp] & (1 << (BITS_PER_LONG - 1 - off))) {
                    priority = grp * BITS_PER_LONG + off;
                    pri = osInfo->priInfoTbl + priority;
                    list_for_each(node, &pri->qReadyHead) {
                        tcb = list_entry(node, LUOS_TCB, qNodeSched);
                        memcpy(tcbs + idx, tcb, sizeof(*tcb));
                        idx++;
                    }
                }
            }
        }
    }
    list_for_each(node, &osInfo->qDelayHead) {
        tcb = list_entry(node, LUOS_TCB, qNodeSched);
        memcpy(tcbs + idx, tcb, sizeof(*tcb));
        idx++;
    }
    list_for_each(node, &osInfo->qPendHead) {
        tcb = list_entry(node, LUOS_TCB, qNodeSched);
        memcpy(tcbs + idx, tcb, sizeof(*tcb));
        idx++;
    }
    ntick = numTocksQWork ;
    taskUnlock();

    log("###############################################################################");
    log("Name         TID    Pri   Status stkBase     Stack  stkSize schedCnt runTicks");
    for (idx = 0; idx < num; idx++) {
        tcb = tcbs + idx;
        log("%-10s %p %-4d %7s %p %p %8d %8d %8d", \
                tcb->name, \
                tcb,               \
                tcb->priority, \
                taskStatusStr(tcb), \
                tcb->stkBase, \
                tcb->stack,    \
                tcb->stkSize,
                tcb->schedCnt,
                tcb->runTicksCnt);
    }
    log("###############################################################################");
    osMemFree(tcbs);
    // log("ntick=%u", ntick);
    return 0;
}

static volatile UINT cpuIdleCnt = 0;
static void *taskIdleEntry(void *arg) {
    while (true) {
        cpuIdleCnt++;
    };
    return NULL;
}


STATUS luosStart(START_RTN appStart, void *appArg, int stackSize)
{
    int level;
    TCB_ID tcb, tcb_idle, tcb_root;

    if (stackSize <= 0) {
        stackSize = 2 * 1024;
    }

    timerLibInit();

    tcb_root = (TCB_ID)taskCreate("tRoot", CONFIG_NUM_PRIORITY - 4, 0, stackSize, appStart, NULL);
    while (NULL == tcb_root) {;}
    tcb_idle = (TCB_ID)taskCreate("tIdle", CONFIG_NUM_PRIORITY - 1, 0, 512, taskIdleEntry, NULL);
    while (NULL == tcb_idle) {;}
    level = intLock();
    tcbActivate(tcb_idle);
    tcbActivate(tcb_root);
    tcb = highReadyTaskGet();
    __osinfo__.currentTcb = tcb;
    __osinfo__.highestTcb = tcb;
    osCoreInfo()->running = true;
    intUnlock(level);
    cpuTaskContextSwitchTrig(currentTask(), tcb);
    return 0;
}

