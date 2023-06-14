/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : timerLib.h
 * 内容摘要   :
 * 其它说明   :
 * 版本       :
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-06-13 03:06:28 PM
 *
 * 修改记录1:
 *    修改日期: 2023-06-13
 *    版 本 号:
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容:
 * ===========================================================================
 */

#ifndef __TIMERLIB_H__
#define __TIMERLIB_H__

typedef int (*TIMER_CB)(void *arg);
typedef struct timer_list {
    TLIST     entry;
    cputime_t expires;
    void *    arg;
    TIMER_CB callback;
} timerList_t;

typedef struct timer_queue {
    TLIST    list;
    uint32_t numItem;
} timerQ_t;

static inline void timerQInit(timerQ_t *q)
{
    q->numItem = 0;
    INIT_LIST_HEAD(&q->list);
}

static inline void timerQAdd(timerQ_t *q,  timerList_t *tmr)
{
    q->numItem++;
    list_add_tail(&tmr->entry, &q->list);
}

static inline void timerQRemove(timerQ_t *q,  timerList_t *tmr)
{
    q->numItem--;
    list_del_init(&tmr->entry);
}

typedef cpudata_t timerid_t;

STATUS timerLibInit(void);
timerid_t timerCreate(cputime_t expires, TIMER_CB cb, void *arg);
STATUS timerInit(timerid_t tmrid, cputime_t expires, TIMER_CB cb, void *arg);
STATUS timerAdd(timerid_t tmrid);
STATUS timerDelete(timerid_t tmrid);
STATUS timerModify(timerid_t tmrid, cputime_t expires);
void timerListDing(void);


#endif /* #ifndef __TIMERLIB_H__ */


