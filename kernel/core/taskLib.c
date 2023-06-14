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
    ulPtr = (ULONG)(p1 + sizeof(LUOS_TCB));
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

static int taskReturn(void)
{
    TCB_ID tcb;
    int level;

    /* semGive */
    tcb = currentTask();
    // printf("task[%s]tid=%p, exit!\n", tcb->name, tcb);
    level = intLock();
    list_del_init(&tcb->qNodeSched);
    list_add(&tcb->qNodeSched, &osInfo->qPendHead);
    taskReadyRemove(tcb);
    tcb->status = TASK_DEAD;
    intUnlock(level);
    semFlush(&tcb->semJoinExit);
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
    int level;
    LUOS_TCB *tcb;
    PriInfo_t *pri;

    tcb = (LUOS_TCB *)tid;
    if (0 == tid || currentTask() == tcb) {
        return OK;
    }
    level = intLock();
    tcb->status = TASK_READY;
    pri = osInfo->priInfoTbl + tcb->priority;
    list_del_init(&tcb->qNodeSched);
    list_add_tail(&tcb->qNodeSched, &pri->qReadyHead);
    taskReadyAdd(tcb);
    intUnlock(level);
    coreTrySchedule();
    return 0;
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
    list_del(&tcb->qNodeSched);
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
    list_del(&tcb->qNodeSched);
    INIT_LIST_HEAD(&tcb->qNodeSched);
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
            tcb->status |= TASK_SUSPEND;
            list_del_init(&tcb->qNodeSched);
            taskReadyRemove(tcb);
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
    PriInfo_t *pri;
    int level;

    tcb = (LUOS_TCB *)tid;
    if (0 == tid || currentTask() == tcb) {
        return OK;
    }
    level = intLock();
    pri = osInfo->priInfoTbl + tcb->priority;

    tcb->status &= ~(TASK_SUSPEND/* | TASK_DELAY*/);
    if (TASK_READY == tcb->status) {
        list_del_init(&tcb->qNodeSched);
        list_add_tail(&tcb->qNodeSched, &pri->qReadyHead);
        taskReadyAdd(tcb);
    }
    intUnlock(level);
    coreTrySchedule();
    return 0;
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
    list_del(&tcb->qNodeSched);
    taskReadyRemove(tcb);
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
    SEM_ID semId;
    PriInfo_t *pri;

    tcb = (TCB_ID)tid;
    if (newPriority == tcb->priority) {
        return OK;
    }
    level = intLock();

    if (TASK_READY == tcb->status) {
        list_del(&tcb->qNodeSched);
        taskReadyRemove(tcb);
        tcb->priority = newPriority;
        pri = osInfo->priInfoTbl + tcb->priority;
        if (tcb == currentTask()) {
            list_add(&tcb->qNodeSched, &pri->qReadyHead);
        } else {
            list_add(&tcb->qNodeSched, &pri->qReadyHead);
        }
        taskReadyAdd(tcb);
        intUnlock(level);
        coreTrySchedule();
        return OK;
    }
    tcb->priority = newPriority;
    if (TASK_PEND & tcb->status) {
        semId = tcb->semIdPended;
        list_del(&tcb->qNodePend);
        taskPendQuePut(tcb, semId);
    }
    intUnlock(level);
    coreTrySchedule();
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
    PriInfo_t *pri;
    LUOS_INFO *osInfo = osCoreInfo();

    /* SEM_Q_PRIORITY */
    if (list_empty(&semId->qPendHead)) {
        return ERROR;
    } else {
        tcb1 = list_first_entry(&semId->qPendHead, LUOS_TCB, qNodePend);
        list_del(&tcb1->qNodePend);
        tcb1->semIdPended = NULL;
        list_del(&tcb1->qNodeSched); /* delete from delay queue or pend queue */
        tcb1->status &= ~(TASK_PEND | TASK_DELAY);
        pri = osInfo->priInfoTbl + tcb1->priority;
        if (SCHED_RR == pri->schedPolicy) {
            tcb1->sliceTicksCnt = 0;
        }
        list_add(&tcb1->qNodeSched, &pri->qReadyHead);
        taskReadyAdd(tcb1);
        if (tcb1->priority < tcb->priority) {
            switch (semId->semType) {
                case SEM_TYPE_MUTEX :
                case SEM_TYPE_BINARY:
                    semId->semOwner = tcb1;
                    break;
                default:
                    break;
            }
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

