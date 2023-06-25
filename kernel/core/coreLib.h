/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : osCore.h
 * 内容摘要   :
 * 其它说明   :
 * 版本       :
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-05-15 05:58:57 PM
 *
 * 修改记录1:
 *    修改日期: 2023-05-15
 *    版 本 号:
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容:
 * ===========================================================================
 */

#ifndef __OSCORE_H__
#define __OSCORE_H__
#include "osTypes.h"
#include "semLib.h"
#include "memPartLib.h"
#include "timerLib.h"
#include "taskLib.h"
#include "taskLibP.h"
#include "msgQLib.h"
#include "flagLib.h"

#ifndef CONFIG_NUM_PRIORITY
#define CONFIG_NUM_PRIORITY 64 /* 64/256/1024/4096 */
#endif
#ifndef BITS_PER_LONG
#define BITS_PER_LONG 32
#define BYTES_PER_LONG (BITS_PER_LONG/8)
#endif
#define NLONG_PRIORITY ((CONFIG_NUM_PRIORITY + (BITS_PER_LONG - 1))/BITS_PER_LONG)
#if NLONG_PRIORITY > BITS_PER_LONG
#error "CONFIG_NUM_PRIORITY should be match (NLONG_PRIORITY > BITS_PER_LONG)"
#endif

#define SCHED_FIFO      1
#define SCHED_RR        2

#define ROUND_UP(x, align)      (((int) (x) + (align - 1)) & ~(align - 1))
#define ROUND_DOWN(x, align)    ((int)(x) & ~(align - 1))
#define ALIGNED(x, align)       (((int)(x) & (align - 1)) == 0)

#define TASK_STACK_ALIGN_SIZE   8
#define MEM_ALLOC_ALIGN_SIZE    8
#define MEM_ROUND_UP(x)         ROUND_UP(x, MEM_ALLOC_ALIGN_SIZE)
#define MEM_ROUND_DOWN(x)       ROUND_DOWN(x, MEM_ALLOC_ALIGN_SIZE)
#define STACK_ROUND_UP(x)       ROUND_UP(x, TASK_STACK_ALIGN_SIZE)
#define STACK_ROUND_DOWN(x)     ROUND_DOWN(x, TASK_STACK_ALIGN_SIZE)
#define MEM_ALIGNED(x)          ALIGNED(x, MEM_ALLOC_ALIGN_SIZE)

/* @param schedPolicy 相同优先级任务中使用哪种调度算法, 默认为时间片轮转, 支持先来先服务
 * @param rdyTcbHdr   用于串连就绪状态中相同优先级的任务控制控制块
 * @param sliceTicks  用于覆盖系统时间片,可为单独优先级配置指定时间片
 * @param numTask     rdyTcbHdr 链表中有多少个tcb, 即同一优先级中有多少个任务处于就绪
 */
typedef struct priorityInfo {
  /* uint32_t priority; */
  int      schedPolicy;
  int      numTask;
  TLIST    qReadyHead;
  uint32_t sliceTicks;
} PriInfo_t;

/* @param rdyPriTbl 用于保存所当前优先级属哪一组就绪任务优先级列表
 * @param rdyPriGrp 用于保存所当前优先级属哪一组, 辅助最快找到当前处于优先就绪状态的最高优先级
 * @param currentTcb 处于运行状态任务控制块
 * @param sliceTicks 时间片轮转算法调度时使用的时间片
 */
typedef struct luosInfo {
    LUOS_TCB* currentTcb;
    LUOS_TCB* highestTcb;
    uint32_t  sliceTicks;
    TLIST     qDelayHead;
    TLIST     qPendHead;
    cpudata_t readyPriGrp;
    cpudata_t readyPriTbl[NLONG_PRIORITY];
    PriInfo_t priInfoTbl[CONFIG_NUM_PRIORITY];
    UINT      taskCreatedCnt;
    BOOL      running;
    BOOL      schedLocked;
    UINT      intNestedCnt;
    ULONG     sysTicksCnt;
    UINT64    absTicksCnt;
    ULONG     contextCnt;
    UINT      numPended;
    UINT      numDelayed;
} LUOS_INFO;

extern LUOS_INFO __osinfo__;
static inline LUOS_TCB* currentTask(void) {
    return __osinfo__.currentTcb;
}

static inline LUOS_INFO* osCoreInfo(void) {
    return &__osinfo__;
}

static inline int priorityGroup(int priority) {
    if (BITS_PER_LONG == 32) {
        return (priority >> 5);
    } else if (BITS_PER_LONG == 64) {
        return (priority >> 6);
    } else if (BITS_PER_LONG == 16) {
        return (priority >> 4);
    } else {
        return (priority >> 3);
    }
}

static inline int priorityOffset(int priority) {
    if (BITS_PER_LONG == 32) {
        return (priority & ((1 << 5)-1));
    } else if (BITS_PER_LONG == 64) {
        return (priority & ((1 << 6)-1));
    } else if (BITS_PER_LONG == 16) {
        return (priority & ((1 << 4)-1));
    } else {
        return (priority & ((1 << 3)-1));
    }
}

static inline void taskReadyRemove(TCB_ID tcb)
{
    int grp;
    int off;
    PriInfo_t *pri = __osinfo__.priInfoTbl + tcb->priority;

    pri->numTask--;
    list_del_init(&tcb->qNodeSched);
    while (pri->numTask < 0) {;}
    grp = priorityGroup(tcb->priority);
    off = priorityOffset(tcb->priority);
    if (0 == pri->numTask) {
        __osinfo__.readyPriTbl[grp] &= ~(1 << (BITS_PER_LONG - 1 - off));
        if (0 == __osinfo__.readyPriTbl[grp]) {
            __osinfo__.readyPriGrp  &= ~(1 << (BITS_PER_LONG - 1 - grp));
        }
    }
}

static inline void taskReadyAdd(TCB_ID tcb, BOOL toTail)
{
    int grp;
    int off;
    PriInfo_t *pri = __osinfo__.priInfoTbl + tcb->priority;

    pri->numTask++;
    tcb->sliceTicksCnt = 0;
    grp = priorityGroup(tcb->priority);
    off = priorityOffset(tcb->priority);
    __osinfo__.readyPriTbl[grp] |= (1 << (BITS_PER_LONG - 1 - off));
    __osinfo__.readyPriGrp      |= (1 << (BITS_PER_LONG - 1 - grp));
    if (!toTail) {
        list_add(&tcb->qNodeSched, &pri->qReadyHead);
    } else {
        list_add_tail(&tcb->qNodeSched, &pri->qReadyHead);
    }
}

int cpuCntLeadZeros(cpudata_t val);
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

static inline void tcbActivate(TCB_ID tcb)
{
    if (TASK_READY == tcb->status) {
        return ;
    }
    tcb->status &= ~TASK_SUSPEND;
    if (TASK_READY == tcb->status) {
        list_del_init(&tcb->qNodeSched);
        taskReadyAdd(tcb, true);
    }
}

extern STATUS coreLibInit(void);
extern void * osMemAlloc(size_t nbytes);
extern STATUS osMemFree(void *ptr);
extern void coreTrySchedule(void);
STATUS coreTickDoing(void);
extern STATUS sysClkRateSet(int ticksPerSecond);
extern int sysClkRateGet(void);
extern ULONG sysClkTickGet(void);
void coreIntEnter(void);
void coreIntExit(void);
extern STATUS i(void);
extern STATUS tickQWorkDoing(void);
extern void tickAnnounce(void);
extern STATUS luosStart(START_RTN appStart, void *appArg, int stackSize);
int Printf(const char *fmt, ...);
UINT cpuUsageGet(void);
void luosQDelayAdd(TCB_ID tcb);
void luosQDelayRemove(TCB_ID tcb);
void luosSysTicksReset(ULONG delta);
void luosDelay(TCB_ID tcb, int ticks);
int sysHwInit(void);
int Printf(const char *fmt, ...);
int shellLibInit(void);

#endif /* #ifndef __OSCORE_H__ */


