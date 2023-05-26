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
#ifndef CONFIG_NUM_PRIORITY 
#define CONFIG_NUM_PRIORITY 256 /* 64/256/1024/4096 */
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
  UINT     numTask;
  TLIST    qReadyHead; 
  uint32_t sliceTicks;
} PriInfo_t;

/* @param rdyPriTbl 用于保存所当前优先级属哪一组就绪任务优先级列表 
 * @param rdyPriGrp 用于保存所当前优先级属哪一组, 辅助最快找到当前处于优先就绪状态的最高优先级 
 * @param currentTcb 处于运行状态任务控制块
 * @param sliceTicks 时间片轮转算法调度时使用的时间片
 */
typedef struct luosInfo {
    BOOL      osRunning;
    BOOL      schedLocked;
    volatile  LUOS_TCB* currentTcb;
    uint32_t  sliceTicks;
    TLIST     qDelayHead; 
    TLIST     qPendHead; 
    ULONG     readyPriGrp; 
    ULONG     readyPriTbl[NLONG_PRIORITY];
    PriInfo_t priReadyTbl[CONFIG_NUM_PRIORITY];
    UINT      taskCreatedCnt;     
} LUOS_INFO;

extern volatile LUOS_INFO __osinfo__;
static inline LUOS_TCB* currentTask() {
    return __osinfo__.currentTcb;
}

static inline LUOS_INFO* osCoreInfo() {
    return &__osinfo__;
}

extern void * osMemAlloc(size_t nbytes);
extern STATUS osMemFree(void *ptr);
extern void coreTrySchedule();
#endif /* #ifndef __OSCORE_H__ */


