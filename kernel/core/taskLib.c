/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : taskLib.c
 * 内容摘要   :
 * 其它说明   :
 * 版本       :
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-05-24 05:00:17 PM
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
#include <stdio.h>
#include "coreLib.h"
extern int Printf(const char *fmt, ...);

LOCAL LUOS_INFO * osInfo = &__osinfo__;
LOCAL BOOL taskLibInstalled = false;
extern void cpuStackInit(LUOS_TCB *tcb, FUNCPTR exitRtn);

STATUS taskLibInit()
{
    osInfo = &__osinfo__;
    taskLibInstalled = true;

    return 0;
}

LOCAL inline void taskListInit(LUOS_TCB *tcb)
{
    INIT_LIST_HEAD(&tcb->memListHdr);
    INIT_LIST_HEAD(&tcb->qNodeSched);
    INIT_LIST_HEAD(&tcb->qNodePend);
}

tid_t taskCreate(char *name, int priority, int options, int stackSize,
                START_RTN taskEntry, void *arg)
{
    int rc;
    LUOS_TCB *tcb = NULL;
    size_t sz;
    void *p1;

    if (!taskLibInstalled) {
        return (tid_t)NULL;
    }
    stackSize = STACK_ROUND_UP(stackSize);
    sz = stackSize + sizeof(LUOS_TCB);
    sz = STACK_ROUND_UP(sz);

    p1 = osMemAlloc(sz);
    if (NULL == p1) {
        return (tid_t)NULL;
    }
#ifdef CONFIG_STACK_GROWSUP
    tcb = (LUOS_TCB *)p1;
    memset(tcb, 0, sizeof(*tcb));
    p1 += sizeof(LUOS_TCB);
    ulPtr = (ULONG)p1;
    ulPtr = STACK_ROUND_UP(ulPtr);
    tcb->stkBase = (void *)ulPtr;
#else
    tcb = (LUOS_TCB *)(p1 + stackSize);
    memset(tcb, 0, sizeof(*tcb));
    tcb->stkBase = p1;
#endif
    osInfo->taskCreatedCnt++;
    rc = taskInit(tcb, name, priority, options, tcb->stkBase, stackSize, taskEntry, arg);
    if (rc) {
        if (NULL != tcb->name) {
            osMemFree(tcb->name);
        }
        osMemFree(p1);
        return (tid_t)NULL;
    }
    return (tid_t)tcb;
}


static int taskDelTimer(void *arg)
{
    int rc = 0;
    int level;
    TCB_ID tcb;
    TCB_ID *ptcb;
    timerList_t *tmr = (timerList_t *)arg;
    SEM_ID semId;
    cputime_t expires;

    if (NULL == tmr) return -1;
    ptcb = (TCB_ID *)(tmr + 1);
    tcb = (TCB_ID)*ptcb;
    if (NULL == tcb)  return -2;

    if (TASK_DEAD != tcb->status) {
        rc =-3;
        goto tmr_del_loop;
    }
    semId = &tcb->semJoinExit;
    if (list_empty(&semId->qPendHead)) {
        level = intLock();
        list_del_init(&tcb->qNodeSched);
        intUnlock(level);
        Printf("task %s will be really deleted\n", tcb->name);
        #ifdef CONFIG_STACK_GROWSUP
        osMemFree(tcb);
        #else
        osMemFree(tcb->stkBase);
        #endif
        osMemFree(tmr);
        return rc;
    }
tmr_del_loop:
    expires = sysClkTickGet();
    expires += 50 * sysClkRateGet() / 1000; /* 50 ms */
    timerModify((timerid_t)tmr, expires);
    return rc;
}

static void taskDelDefer(TCB_ID tcb)
{
    TCB_ID *ptcb;
    timerList_t *tmr;
    cputime_t expires;
    size_t sz = sizeof(timerid_t);

    sz += sizeof(tcb);
    tmr = (timerList_t *)osMemAlloc(sz);
    if (NULL == tmr) return;
    expires = sysClkTickGet();
    expires += sysClkRateGet();
    ptcb = (TCB_ID *)(tmr + 1);
    *ptcb = tcb;

    timerInit((timerid_t)tmr, expires, taskDelTimer, tmr);
    timerAdd((timerid_t)tmr);
}

static int taskReturn(void)
{
    TCB_ID tcb;
    int level;
    bool needFlush = false;
    SEM_ID semId;

    /* semGive */
    tcb = currentTask();
    // printf("task[%s]tid=%p, exit!\n", tcb->name, tcb);
    level = intLock();
    taskReadyRemove(tcb);
    list_add(&tcb->qNodeSched, &osInfo->qPendHead);
    tcb->status = TASK_DEAD;
    semId = &tcb->semJoinExit;
    if (!list_empty(&semId->qPendHead)) {
        needFlush = true;
    }
    intUnlock(level);
    taskDelDefer(tcb);
    if (needFlush) {
        semFlush(semId);
    }
    coreTrySchedule();
    return 0;
}

static STATUS taskNameInit(LUOS_TCB *tcb, char *name)
{
    char *p1;
    size_t idx, len;
    static char digits[] = "0123456789";
    char defname[32];
    uint32_t taskid = osInfo->taskCreatedCnt;

    if (NULL != name) {
        len = strlen(name);
    } else {
        defname[31] = '\0';
        idx = 0;
        while (taskid > 0) {
            idx++;
            defname[31 - idx] = digits[taskid % 10];
            taskid /= 10;
        }
        defname[31 - idx] = 't';
        name = &defname[31 - idx];
        len = idx + 1;
    }
    p1 = (char *)osMemAlloc(len);
    if (NULL == p1) {
        return -1;
    }
    strcpy(p1, name);
    tcb->name = p1;
    return 0;
}

STATUS taskInit(LUOS_TCB *tcb, char *name, int priority, int options,
        char *pStackBase, int stackSize, START_RTN taskEntry, void *arg)
{
    int level;

    if (taskNameInit(tcb, name)) {
        return -1;
    }
    tcb->stkEnd    = tcb->stkBase + stackSize;
#ifdef CONFIG_STACK_GROWSUP
    tcb->stack     = tcb->stkBase;
#else
    tcb->stack     = tcb->stkEnd;
#endif
    tcb->stkSize   = stackSize;
    tcb->param     = arg;
    tcb->taskEntry = taskEntry;
    tcb->priority  = priority;
    tcb->options   = options;
    tcb->status    = TASK_SUSPEND;
    tcb->stkLimit  = stackSize / 10;

    taskListInit(tcb);
    level = intLock();
    list_add_tail(&tcb->qNodeSched, &osInfo->qPendHead);
    intUnlock(level);

    cpuStackInit(tcb, taskReturn);
    semCInit(&tcb->semJoinExit, 0, 0);
    return 0;
}

STATUS taskActivate(tid_t tid)
{
    return taskResume(tid);
}

tid_t taskSpawn(char *name, int priority, int options, int stackSize,
                START_RTN taskEntry, void *arg)
{
    LUOS_TCB* tcb;

    tcb = (LUOS_TCB *)taskCreate(name, priority, options, stackSize, taskEntry, arg);
    if (NULL == tcb) {
        return (tid_t)0;
    }
    taskActivate((tid_t)tcb);
    return (tid_t)tcb;
}

STATUS taskLock(void)
{
    int intSR;
    LUOS_TCB *tcb;

    intSR = intLock();
    tcb = currentTask();
    tcb->lockCnt++;
    intSR = intUnlock(intSR);
    return 0;
}

STATUS taskUnlock (void)
{
    int level;
    LUOS_TCB *tcb;

    level = intLock();
    tcb = currentTask();
    tcb->lockCnt--;
    intUnlock(level);
    coreTrySchedule();
    return 0;
}

STATUS taskDelay(int ticks)
{
    LUOS_TCB *tcb;
    int level;

    if (ticks < 0) {
        return -1;
    }
    level = intLock();
    tcb = currentTask();
    tcb->dlyTicks = ticks;
    taskReadyRemove(tcb);
    tcb->status = TASK_DELAY;
    list_add_tail(&tcb->qNodeSched, &osInfo->qDelayHead);
    intUnlock(level);
    coreTrySchedule();
    return 0;
}

char * taskName(tid_t tid)
{
    return ((TCB_ID)tid)->name;
}


tid_t taskIdSelf(void)
{
    return (tid_t)currentTask();
}

STATUS taskDelete(tid_t tid)
{
    LUOS_TCB *tcb;
    int level;

    tcb = (LUOS_TCB *)tid;
    if (0 == tid) {
        tcb = currentTask();
    }
    level = intLock();
    tcb = (LUOS_TCB *)tid;
    taskReadyRemove(tcb);
    tcb->status |= TASK_DEAD;
    intUnlock(level);
    coreTrySchedule();
    return 0;
}

STATUS taskDeleteForce(tid_t tid)
{
    return 0;
}

STATUS taskSuspend(tid_t tid)
{
    LUOS_TCB *tcb;
    int level;

    tcb = (LUOS_TCB *)tid;
    level = intLock();

    if (TASK_READY == tcb->status) {
        if (NULL == tcb->semIdOwner) {
            taskReadyRemove(tcb);
            tcb->status |= TASK_SUSPEND;
            list_add(&tcb->qNodeSched, &osInfo->qPendHead);
            intUnlock(level);
            coreTrySchedule();
            return OK;
        }
    }
    tcb->status |= TASK_SUSPEND;
    intUnlock(level);
    return ERROR;
}

STATUS taskResume(tid_t tid)
{
    LUOS_TCB *tcb;
    int level;

    tcb = (LUOS_TCB *)tid;
    if (0 == tid || currentTask() == tcb) {
        return OK;
    }
    if (TASK_READY == tcb->status) {
        return OK;
    }
    level = intLock();
    tcb->status &= ~(TASK_SUSPEND/* | TASK_DELAY*/);
    if (TASK_READY == tcb->status) {
        list_del_init(&tcb->qNodeSched);
        taskReadyAdd(tcb, true);
        intUnlock(level);
        coreTrySchedule();
        return OK;
    }

    intUnlock(level);
    return ERROR;
}

STATUS taskRestart(tid_t tid)
{
    STATUS rc;
    LUOS_TCB *tcb;

    tcb = (LUOS_TCB *)tid;
    if (0 == tid) {
        tcb = currentTask();
    }
    if (tcb == currentTask()) {
        return -1;
    }
    if (TASK_READY == tcb->status) {
        taskReadyRemove(tcb);
    }
    list_del_init(&tcb->qNodeSched);
    rc = taskInit(tcb, tcb->name, tcb->priority, tcb->options, tcb->stkBase,
            tcb->stkSize, tcb->taskEntry, tcb->param);
    if (OK == rc) {
        return taskActivate((tid_t)tcb);
    }
    return rc;
}

STATUS taskPrioritySet(tid_t tid, int newPriority)
{
    int level;
    TCB_ID tcb;
    TCB_ID htcb;
    SEM_ID semId;

    if (newPriority < 1 || newPriority >= CONFIG_NUM_PRIORITY) {
        return ERROR;
    }
    tcb = (TCB_ID)tid;
    if (newPriority == tcb->priority) {
        return OK;
    }
    // return OK;

    level = intLock();
    if (TASK_READY == tcb->status) {
        taskReadyRemove(tcb);
        tcb->priority = newPriority;
        htcb = highReadyTaskGet();
        if (tcb == currentTask()) {
            if (newPriority <= htcb->priority
             || taskLocked()) {
                taskReadyAdd(tcb, false);
                intUnlock(level);
                return OK;
            }
        }
        taskReadyAdd(tcb, true);
        intUnlock(level);
        if (htcb->priority < newPriority) {
            coreTrySchedule();
        }
        return OK;
    }
    tcb->priority = newPriority;
    if (TASK_PEND & tcb->status) {
        semId = tcb->semIdPended;
        if (SEM_Q_PRIORITY == (semId->options & SEM_Q_MASK)) {
            /* resort the sem pendQ */
            list_del_init(&tcb->qNodePend);
            taskPendQuePut(tcb, semId);
        }
    }
    intUnlock(level);
    return OK;
}

STATUS taskPriorityGet(tid_t tid, int *pPriority)
{
    TCB_ID tcb = (TCB_ID)tid;

    if (NULL != pPriority) {
        *pPriority = tcb->priority;
        return OK;
    }
    return ERROR;
}

STATUS taskPendQuePut(TCB_ID tcb, SEM_ID semId)
{
    TCB_ID tcb1;
    TLIST *node;

    tcb->semIdPended = semId;
    if (SEM_Q_FIFO == (semId->options & SEM_Q_MASK)) {
        list_add_tail(&tcb->qNodePend, &semId->qPendHead);
    } else {
        /* SEM_Q_PRIORITY */
        if (list_empty(&semId->qPendHead)) {
            list_add_tail(&tcb->qNodePend, &semId->qPendHead);
        } else {
            tcb1 = list_first_entry(&semId->qPendHead, LUOS_TCB, qNodePend);
            if (tcb1->priority > tcb->priority) {
                list_add(&tcb->qNodePend, &semId->qPendHead);
            } else {
                list_for_each_prev(node, &semId->qPendHead) {
                    tcb1 = list_entry(node, LUOS_TCB, qNodePend);
                    if (tcb1 == tcb) {
                        while (1) {/* hang up here */};
                    }
                    if (tcb->priority >= tcb1->priority) {
                        /* list_add_tail(&tcb->qNodePend, node); */
                        tcb->qNodePend.next = node->next;
                        tcb->qNodePend.prev = node;
                        node->next = &tcb->qNodePend;
                        break;
                    }
                }
            }
        }
    }
    return 0;
}

STATUS taskPendQueGet(TCB_ID tcb, SEM_ID semId)
{
    TCB_ID tcb1;

    /* SEM_Q_PRIORITY */
    if (list_empty(&semId->qPendHead)) {
        return ERROR;
    } else {
        tcb1 = list_first_entry(&semId->qPendHead, LUOS_TCB, qNodePend);
        list_del_init(&tcb1->qNodePend);
        tcb1->status &= ~(TASK_PEND | TASK_DELAY);
        if (TASK_READY == tcb1->status) {
            tcb1->semIdPended = NULL;
            list_del_init(&tcb1->qNodeSched);
            taskReadyAdd(tcb1, false);
            return OK;
        }
    }
    return ERROR;
}

STATUS taskQReadyPut(TCB_ID tcb)
{
    return 0;
}

const char *taskStatusStr(TCB_ID tcb)
{
    if (TASK_READY == tcb->status) {
        if (tcb == currentTask()) {
            return "Running";
        }
        return "Ready";
    } else if (TASK_PEND & tcb->status) {
        if (TASK_DELAY & tcb->status) {
            return "Pend+T";
        }
        return "Pend";
    } else if (TASK_SUSPEND & tcb->status) {
        if (TASK_DELAY & tcb->status) {
            return "Delay+S";
        }
        return "Suspend";
    } else if (TASK_DEAD & tcb->status) {
        return "Dead";
    } else if (TASK_DELAY & tcb->status) {
        return "Delay";
    } else {
        return "Unkown";
    }
    return "Unkown";
}

STATUS taskStatusString(tid_t tid, char *str)
{
    char *strs;
    TCB_ID tcb = (TCB_ID)tid;

    if (NULL == str) {
        return ERROR;
    }
    if (0 == tid) {
        tcb = (TCB_ID)taskIdSelf();
    }
    strs = taskStatusStr(tcb);
    if (NULL == str) {
        return ERROR;
    }
    strcpy(str, strs);
    return OK;
}



