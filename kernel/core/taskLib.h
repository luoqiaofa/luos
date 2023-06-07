/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : taskLib.h
 * 内容摘要   :
 * 其它说明   :
 * 版本       :
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-05-15 04:41:08 PM
 *
 * 修改记录1:
 *    修改日期: 2023-05-15
 *    版 本 号:
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容:
 * ===========================================================================
 */

#ifndef __TASKLIB_H__
#define __TASKLIB_H__
#include "osTypes.h"
#include "semLib.h"
#include "coreLib.h"
#include "taskLib.h"
#include "memPartLib.h"
/*  task status values */
#define TASK_READY      0x00    /*  ready to run */
#define TASK_SUSPEND    0x01    /*  explicitly suspended */
#define TASK_PEND       0x02    /*  pending on semaphore */
#define TASK_DELAY      0x04    /*  task delay (or timeout) */
#define TASK_DEAD       0x08    /*  dead task */

#define TASK_OPT_STK_CHK (1 << 0)
#define TASK_OPT_Q_FIFO  (1 << 1)

typedef struct luosTcb {
    char *    name;
    UINT      status;
    int32_t   errCode;
    int32_t   priority;
    void *    stack;     /* statck pointer */
    void *    stkBase;   /* statck origin pointer */
    void *    stkEnd;    /* statck origin pointer */
    int32_t   stkSize;   /* statck size */
    int32_t   stkLimit;
    int32_t   dlyTicks;
    int       options;
    TLIST     memListHdr;  /* mem list header */
    TLIST     qNodeSched;    /* for ready, delay queue list */
    TLIST     qNodePend;       /* for mutex,msgqueue,semphore,suspend cause to be pended. etc */
    SEM_ID    semIdPended;
    SEM_ID    semIdOwner;
    SEMAPHORE semJoinExit;
    int32_t   priNormal;
    int32_t   priDynamic;
    int32_t   sliceTicksCnt; /* count the ticks for slice */
    cputime_t firstSchedTs;  /* last scheduled os ticks cnt */
    int32_t   latestTick;    /* last scheduled os ticks cnt */
    START_RTN taskEntry;
    void *    param;
    cpudata_t lockCnt;
    cpudata_t schedCnt;
    cpudata_t runTicksCnt;
} LUOS_TCB;

typedef LUOS_TCB* TCB_ID;

typedef struct taskDesc {
    char*     name;
    UINT      status;
    int32_t   priority;
    void*     stack;     /* statck pointer */
    void*     stkBase;   /* statck origin pointer */
    void*     stkEnd;    /* statck origin pointer */
    int32_t   stkSize;   /* statck size */
    int32_t   stkLimit;
    cputime_t firstSchedTs;  /* last scheduled os ticks cnt */
} TASK_DESC;

extern STATUS taskLibInit(void);
extern tid_t taskSpawn(char *name, int priority, int options, int stackSize,
        START_RTN taskEntry, void *arg);
extern STATUS taskInit(LUOS_TCB *pTcb, char *name, int priority, int options,
        char *pStackBase, int stackSize, START_RTN taskEntry, void *arg);
extern STATUS taskActivate(tid_t tid);
extern STATUS taskDelete(tid_t tid);
extern STATUS taskDeleteForce(tid_t tid);
extern STATUS taskSuspend(tid_t tid);
extern STATUS taskResume(tid_t tid);
extern STATUS taskRestart(tid_t tid);
extern STATUS taskPrioritySet(tid_t tid, int newPriority);
extern STATUS taskPriorityGet(tid_t tid, int *pPriority);
extern STATUS taskLock(void);
extern STATUS taskUnlock(void);
extern STATUS taskSafe(void);
extern STATUS taskUnsafe(void);
extern STATUS taskDelay(int ticks);
extern STATUS taskOptionsSet(tid_t tid, int mask, int newOptions);
extern STATUS taskOptionsGet(tid_t tid, int *pOptions);
extern char * taskName(tid_t tid);
extern int    taskNameToId(char *name);
extern STATUS taskIdVerify(tid_t tid);
extern tid_t  taskIdSelf(void);
extern int    taskIdDefault(tid_t tid);
extern BOOL   taskIsReady(tid_t tid);
extern BOOL   taskIsSuspended(tid_t tid);
extern LUOS_TCB *taskTcb(tid_t tid);
extern int    taskIdListGet(int idList[], int maxTasks);
extern STATUS taskInfoGet(tid_t tid, TASK_DESC *pTaskDesc);
extern STATUS taskStatusString(tid_t tid, char *pString);
extern STATUS taskOptionsString(tid_t tid, char *pString);
extern STATUS taskRegsGet(tid_t tid, REG_SET *pRegs);
extern STATUS taskRegsSet(tid_t tid, REG_SET *pRegs);
extern void   taskRegsShow(tid_t tid);
extern void * taskStackAllot(tid_t tid, unsigned nBytes);
extern void   taskShowInit(void);
extern STATUS taskShow(tid_t tid, int level);


#endif /* #ifndef __TASKLIB_H_ */


