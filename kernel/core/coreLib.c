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
static volatile int16_t numTickQWork = 0;
static volatile uint8_t tickQworkWrIdx = 0;
static volatile uint8_t tickQWorkRdIdx = 0;

#define tickQWorkEmpey() (tickQworkWrIdx == tickQWorkRdIdx)
void tickAnnounce(void)
{
    if (!taskLocked()) {
        tickQWorkDoing();
        coreTickDoing();
    } else {
        numTickQWork++;
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
        numTickQWork--;
    }
    /* maybe need ched 0 == numTickQWork or not for overflow */
    return num;
}

STATUS coreTickDoing(void)
{
    TLIST *node;
    LUOS_INFO *osInfo;
    PriInfo_t *pri;
    TCB_ID tcb, tdel;

    if (taskLocked()) {
        return ERROR;
    }

    osInfo = &__osinfo__;
    osInfo->sysTicksCnt++;
    osInfo->absTicksCnt++;
    timerListDoing();

    tcb = currentTask();
    tcb->runTicksCnt++;
    pri = osInfo->priInfoTbl + tcb->priority;
    if (SCHED_RR == pri->schedPolicy) {
        if (tcb->sliceTicksCnt < pri->sliceTicks) {
            tcb->sliceTicksCnt++;
            if (tcb->sliceTicksCnt >= pri->sliceTicks) {
                tcb->sliceTicksCnt = 0;
                if (pri->numTask > 1) {
                    list_del_init(&tcb->qNodeSched);
                    list_add_tail(&tcb->qNodeSched, &pri->qReadyHead);
                }
            }
        }
    }

    tdel = NULL;
    list_for_each(node, &osInfo->qDelayHead) {
        if (NULL != tdel) {
            luosQDelayRemove(tdel);
            if (TASK_READY == tdel->status) {
                taskReadyAdd(tdel, true);
            } else {
                taskReadyAdd(tdel, true);
            }
            tdel = NULL;
        }
        tcb = list_entry(node, LUOS_TCB, qNodeSched);
        while(NULL == tcb){;}
        if (osInfo->sysTicksCnt < tcb->dlyTicks) {
            break;
        }
        tdel = tcb;
        tcb->status &= ~TASK_DELAY;
        if (TASK_READY == tcb->status) {
            tcb->errCode = 0;
        } else {
            /* pend envent timeout */
            tcb->status &= ~TASK_PEND;
            if (TASK_READY == tcb->status) {
                tcb->errCode = ERROR;
            }
        }
    }
    if (NULL != tdel) {
        luosQDelayRemove(tdel);
        if (TASK_READY == tdel->status) {
            taskReadyAdd(tdel, true);
        } else {
            taskReadyAdd(tdel, true);
        }
        tdel = NULL;
    }

    return 0;
}

int interrupt_from_handler = 0;
void intIsFromHandlerSet(int isHandler)
{
#ifdef DBG
    if (isHandler) {
        interrupt_from_handler = 1;
    } else {
        interrupt_from_handler = 0;
    }
#else
    interrupt_from_handler = isHandler;
#endif
}

int intIsFromHandlerGet(void)
{
    return interrupt_from_handler;
}

void coreTrySchedule(void)
{
    LUOS_TCB *tcb;
    int level;

    if (taskLocked()) {
        return;
    }
    level = intLock();
    if (interrupt_from_handler) {
        intUnlock(level);
        return ;
    }
    tcb = currentTask();

    if (0 == __osinfo__.intNestedCnt) {
        tickQWorkDoing();
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
    intIsFromHandlerSet(0);
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

ULONG sysClkTickGet(void)
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
#define log(fmt, args...) Printf(fmt " \n", ## args)
STATUS i(void)
{
    // int level;
    TCB_ID tcb;
    TCB_ID *ptcbs;
    TLIST *node;
    int priority;
    PriInfo_t *pri;
    cpudata_t grp, off;
    LUOS_INFO *osInfo = osCoreInfo();
    // uint32_t ntick;
    uint32_t idx, num;
    char tname[11];

    num = 0;
    taskLock();
    // level = intLock();
    for(grp = 0; grp < NLONG_PRIORITY; grp++) {
        if (0 != osInfo->readyPriTbl[grp]) {
            for (off = 0; off < BITS_PER_LONG; off++) {
                if (osInfo->readyPriTbl[grp] & (1 << (BITS_PER_LONG - 1 - off))) {
                    priority = grp * BITS_PER_LONG + off;
                    pri = osInfo->priInfoTbl + priority;
                    list_for_each(node, &pri->qReadyHead) {
                        num++;
                    }
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
    ptcbs = osMemAlloc(num * sizeof(tcb));
    if (NULL == ptcbs) {
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
                        ptcbs[idx] = tcb;
                        idx++;
                    }
                }
            }
        }
    }
    list_for_each(node, &osInfo->qDelayHead) {
        tcb = list_entry(node, LUOS_TCB, qNodeSched);
        ptcbs[idx] = tcb;
        idx++;
    }
    list_for_each(node, &osInfo->qPendHead) {
        tcb = list_entry(node, LUOS_TCB, qNodeSched);
        ptcbs[idx] = tcb;
        idx++;
    }
    // ntick = numTickQWork;
    taskUnlock();
    // intUnlock(level);
    tname[10] = '\0';
    log("###############################################################################");
    log("Name         TID      Pri   Status stkBase     Stack  stkSize schedCnt runTicks");
    for (idx = 0; idx < num; idx++) {
        tcb = ptcbs[idx];
        strncpy(tname, tcb->name, 10);
        log("%-10s %10u %-4d %7s %p %p %8d %8d %8d", \
                tname, \
                (UINT)tcb,               \
                tcb->priority, \
                taskStatusStr(tcb), \
                tcb->stkBase, \
                tcb->stack,    \
                tcb->stkSize,
                tcb->schedCnt,
                tcb->runTicksCnt);
    }
    log("###############################################################################");
    osMemFree(ptcbs);
    // log("ntick=%u", ntick);
    return num;
}

TCB_ID shllTcbGet(void);
static volatile UINT64 cpuIdleCnt = 0;
static void *taskIdleEntry(void *arg) {
    tid_t tshell;
    
    tshell = (tid_t)shllTcbGet();
    taskActivate(tshell);
    while (true) {
        cpuIdleCnt++;
    };
    return NULL;
}

static TCB_ID idleTcb = NULL;
extern int sysSymTblInit(void);
STATUS luosStart(START_RTN appStart, void *appArg, int stackSize)
{
    int rc;
    int level;
    TCB_ID tcb, tcb_idle, tcb_root;

    if (stackSize <= 0) {
        stackSize = 2 * 1024;
    }

    rc = coreLibInit();
    if (OK != rc) {
        return rc;
    }
    rc = memPartLibInit();
    if (OK != rc) {
        return rc;
    }
    rc = taskLibInit();
    if (OK != rc) {
        return rc;
    }
    semCLibInit();
    semMLibInit();
    semBLibInit();

    timerLibInit();

    sysSymTblInit();

    shellLibInit();

    tcb_root = (TCB_ID)taskCreate("tRoot", CONFIG_NUM_PRIORITY - 10, 0, stackSize, appStart, NULL);
    while (NULL == tcb_root) {;}
    tcb_idle = (TCB_ID)taskCreate("tIdle", CONFIG_NUM_PRIORITY - 1, 0, 512, taskIdleEntry, NULL);
    while (NULL == tcb_idle) {;}
    idleTcb = tcb_idle;

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

UINT64 tick64Get(void)
{
    int level;
    UINT64 ticks;

    level = intLock();
    ticks = __osinfo__.absTicksCnt;
    intUnlock(level);
    return ticks;
}

UINT cpuUsageGet(void)
{
    UINT rc;
    UINT ticks;
    UINT ticks_valid;

    ticks = __osinfo__.sysTicksCnt;
    ticks_valid = ticks - idleTcb->runTicksCnt;
    rc = (ticks_valid * 100000)/ticks;
    return rc;
}

void luosQDelayAdd(TCB_ID tcb)
{
    TCB_ID tcb1;
    LUOS_INFO *osInfo;
    TLIST *node, *prev, *next, *new;

    osInfo = &__osinfo__;
    osInfo->numDelayed++;
    list_for_each(node, &osInfo->qDelayHead) {
        tcb1 = list_entry(node, LUOS_TCB, qNodeSched);
        while (NULL == tcb1) {;/* hang up here */}
        if (tcb->dlyTicks < tcb1->dlyTicks) {
            new = &tcb->qNodeSched;
            next = &tcb1->qNodeSched;
            prev = next->prev;
            __list_add(new, prev, next);
            return ;
        }
    }
    list_add_tail(&tcb->qNodeSched, &osInfo->qDelayHead);
}

void luosQDelayRemove(TCB_ID tcb)
{
    LUOS_INFO *osInfo;

    osInfo = &__osinfo__;
    if (list_empty(&osInfo->qDelayHead)) {
        return ;
    }
    osInfo->numDelayed--;
    list_del_init(&tcb->qNodeSched);
}


void luosDelay(TCB_ID tcb, int ticks)
{
    ULONG delta;
    ULONG expires;
    LUOS_INFO *osInfo;

    osInfo = &__osinfo__;
    expires = osInfo->sysTicksCnt;
    if ((expires + ticks) < expires) {
        delta = ~expires + 1;
        luosSysTicksReset(delta);
    }

    expires = osInfo->sysTicksCnt + ticks;
    tcb->dlyTicks = expires;
    luosQDelayAdd(tcb);
}

void luosSysTicksReset(ULONG delta)
{
    TCB_ID tcb;
    TLIST *node;
    LUOS_INFO *osInfo;

    osInfo = &__osinfo__;
    osInfo->sysTicksCnt = 0;
    list_for_each(node, &osInfo->qDelayHead) {
        tcb = list_entry(node, LUOS_TCB, qNodeSched);
        while (NULL == tcb) {;/* hang up here */}
        tcb->dlyTicks += delta;
    }
    timerQWaitAdjust(delta);
}

