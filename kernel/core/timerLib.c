/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : timerLib.c
 * 内容摘要   :
 * 其它说明   :
 * 版本       :
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-06-13 03:41:55 PM
 *
 * 修改记录1:
 *    修改日期: 2023-06-13
 *    版 本 号:
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容:
 * ===========================================================================
 */

/******************************************************************************
 *                #include (依次为标准库头文件、非标准库头文件)               *
 ******************************************************************************/
#include "coreLib.h"
#include "timerLib.h"

LOCAL void *timerTaskEntry(void *arg);

LOCAL timerQ_t qTimerWait;
LOCAL timerQ_t qTimerExpires;
LOCAL TCB_ID   taskIdTimer = NULL;


STATUS timerLibInit(void)
{
    timerQInit(&qTimerWait);
    timerQInit(&qTimerExpires);

    taskIdTimer = (TCB_ID)taskCreate("tTimer", 9, 0, 512, timerTaskEntry, NULL);
    return OK;
}

timerid_t timerCreate(cputime_t expires, TIMER_CB cb, void *arg)
{
    timerList_t *tmr;

    tmr = osMemAlloc(sizeof(*tmr));
    if (NULL == tmr) {
        return 0;
    }
    timerInit((timerid_t)tmr, expires, cb, arg);
    return (timerid_t)tmr;
}

STATUS timerInit(timerid_t tmrid, cputime_t expires, TIMER_CB cb, void *arg)
{
    timerList_t *tmr = (timerList_t *)tmrid;

    INIT_LIST_HEAD(&tmr->entry);
    tmr->arg      = arg;
    tmr->callback = cb;
    tmr->expires  = expires;
    return OK;
}

STATUS timerAdd(timerid_t tmrid)
{
    int level;

    level = intLock();
    timerQAdd(&qTimerWait, (timerList_t *)tmrid);
    intUnlock(level);
    return OK;
}

STATUS timerDelete(timerid_t tmrid)
{
    return OK;
}

STATUS timerModify(timerid_t tmrid, cputime_t expires)
{
    timerList_t *tmr = (timerList_t *)tmrid;

    tmr->expires = expires;
    if (list_empty(&tmr->entry)) {
        timerAdd(tmrid);
    }
    return OK;
}

LOCAL void *timerTaskEntry(void *arg)
{
    int level;
    TLIST *node, newhead;
    timerList_t *tmr;

    while (1) {
        level = intLock();
        if (list_empty(&qTimerExpires.list)) {
            intUnlock(level);
            taskSuspend(taskIdSelf());
            continue;
        }
        // num = qTimerExpires.numItem;
        list_replace(&qTimerExpires.list, &newhead);
        timerQInit(&qTimerExpires);
        intUnlock(level);
        tmr = NULL;
        list_for_each(node, &newhead) {
            if (NULL != tmr) {
                list_del_init(&tmr->entry);
                if (NULL != tmr->callback) {
                    tmr->callback(tmr->arg);
                }
                tmr = NULL;
            }
            tmr = list_entry(node, timerList_t, entry);
        }
        if (NULL != tmr) {
            list_del_init(&tmr->entry);
            if (NULL != tmr->callback) {
                tmr->callback(tmr->arg);
            }
            tmr = NULL;
        }
        taskSuspend(taskIdSelf());
    }
    return NULL;
}

void timerListDing(void)
{
    TLIST *node;
    timerList_t *tmr, *tdel;
    LUOS_INFO *osInfo = osCoreInfo();

    /* if (0 == osInfo->sysTicksCnt) return ; */
    if (0 == qTimerWait.numItem) return ;
    tdel = NULL;
    list_for_each(node, &qTimerWait.list) {
        if (NULL != tdel) {
            timerQRemove(&qTimerWait, tdel);
            timerQAdd(&qTimerExpires, tdel);
            tdel = NULL;
        }
        tmr = list_entry(node, timerList_t, entry);
#if 0
        if (0 == tmr->expires) {
            continue;
        }
#endif
        if (osInfo->sysTicksCnt == tmr->expires) {
            tdel = tmr;
        }
    }
    if (NULL != tdel) {
        timerQRemove(&qTimerWait, tdel);
        timerQAdd(&qTimerExpires, tdel);
        tdel = NULL;
    }
    if (qTimerExpires.numItem > 0) {
#if 1
         tcbActivate(taskIdTimer);
#else
         taskLock();
         taskResume((tid_t)taskIdTimer);
         taskUnlock();
#endif
    }
}

