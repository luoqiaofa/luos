/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : main.c
 * 内容摘要   :
 * 其它说明   :
 * 版本       :
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-05-30 04:36:11 PM
 *
 * 修改记录1:
 *    修改日期: 2023-05-30
 *    版 本 号:
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容:
 * ===========================================================================
 */

/******************************************************************************
 *                #include (依次为标准库头文件、非标准库头文件)               *
 ******************************************************************************/
#include <stdio.h>
#include <stdarg.h>

#include "coreLib.h"

extern int usrSymInit(void);
extern int sysSymTblInit(void);
extern int shellCmdlineProcess(char *cmdline);
int Printf(const char *fmt, ...);


extern int version(void);
LOCAL SEM_ID semId;
SEMAPHORE  semMuxprint;

void *taskRtn2(void *arg);
extern void USARTx_Config(void);
extern char * readline(const char *const prompt);

volatile UINT cpuIdleCnt = 0;
void *taskIdle(void *arg) {
    while (true) {
        cpuIdleCnt++;
    };
}

static SEM_ID semIds[9];
volatile UINT cpuStatusCnt = 0;
void *taskStatus(void *arg) {
    Printf("[%s]Join enter\n", taskName(taskIdSelf()));
    semTake(semIds[0], WAIT_FOREVER);
    Printf("[%s]Join exit\n", taskName(taskIdSelf()));
    while (true) {
        cpuStatusCnt++;
        taskDelay(1);
    };
}

void *taskOneshort(void *arg)
{
    Printf("[%s] enter!\n", taskName(taskIdSelf()));
    taskDelay(sysClkRateGet());
    Printf("[%s] exit!\n", taskName(taskIdSelf()));
    return NULL;
}

extern void LED_Init ( void );
extern void ledToggle(void);
int dbg_print = 0;
void *taskRtn1(void *arg)
{
    tid_t tid;
    TCB_ID tcb;
    int cnt = 0;
    char tname[20] = "tOne";

    sysClkRateSet(CONFIG_HZ);

    taskSpawn("t2",     10, 0, 1024, taskRtn2, semId);
    taskSpawn("tIdle", 255, 0, 512,  taskIdle, NULL);
    taskSpawn("tStat", 252, 0, 512,  taskStatus, NULL);
    for (cnt = 0; cnt < 9; cnt++) {
        tname[4] = '1' + cnt;
        tid = taskSpawn(tname,  253, 0, 512,  taskOneshort, NULL);
        tcb = (TCB_ID)tid;
        semIds[cnt] = &(tcb->semJoinExit);
        if (tid == (tid_t)0) {
            Printf("task %s created failed\n", tname);
        }
    }

    cnt = 0;
	LED_Init();
    while(true) {
        cnt++;
        ledToggle();
        if (dbg_print) {
		    Printf("[%s]cnt=%d\n", taskName(taskIdSelf()), cnt);
		}
        semGive(semId);
        taskDelay(sysClkRateGet());
    }
    return NULL;
}

void *taskRtn2(void *arg)
{
    SEM_ID id;
    char *cmdline;
    size_t len = 0;
    int cnt = 0;
    int retVal;

    id = (SEM_ID)arg;
    sysSymTblInit();
    usrSymInit();
    while(true) {
        cnt++;
		// printf("[%s]cnt=%d\n", taskName(taskIdSelf()), cnt);
        cmdline = readline("luos->");
        if (NULL == cmdline) {
            continue;
        }
        len = strlen(cmdline);
        if (len > 0) {
            if (dbg_print) {
                Printf("len=%d,cmdline=%s\n", len, cmdline);
            }
            retVal = shellCmdlineProcess(cmdline);
            Printf("retVal=%d(0x%0x)\n", retVal, retVal);
        }
        // semTake(id, WAIT_FOREVER);
        // taskDelay(2);
    }
    return NULL;
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:
 * =====================================================================================
 */
int main (int argc, char *argv[])
{
    int rc;
    tid_t tid;


    rc = coreLibInit();
    rc = memPartLibInit();
    rc = taskLibInit();
    semCLibInit();
    semMLibInit();
    semBLibInit();
    semMInit(&semMuxprint, 0);

    semId = semCCreate(SEM_Q_PRIORITY, 0);
    if (NULL == semId) {
        return -1;
    }
	USARTx_Config();
	version();
    if (OK != rc) {
        printf("taskLibInit failed\n");
    }
    osCoreInfo()->running = true;
    tid = taskSpawn("t1", 35, 0, 1024, taskRtn1, NULL);
    if (NULL == (TCB_ID)tid) {
        printf("Create task t1 failed\n");
    }
    while (true) {
    }
    return 0;
}                /* ----------  end of function main  ---------- */


#ifndef CONFIG_SYS_PBSIZE
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + 128)
#endif

int vscnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
	int i;

	i = vsnprintf(buf, size, fmt, args);

	if (i < size)
		return i;
	if (size != 0)
		return size - 1;
	return 0;
}


int Printf(const char *fmt, ...)
{
	va_list args;
	uint32_t i;

	static char printbuffer[CONFIG_SYS_PBSIZE];

    semTake(&semMuxprint, WAIT_FOREVER);
	va_start(args, fmt);

	/*
	 * For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	printbuffer[0] = '\0';
	i = vscnprintf(printbuffer, sizeof(printbuffer), fmt, args);
	va_end(args);

	/* Print the string */
	puts(printbuffer);
	semGive(&semMuxprint);
	return i;
}

