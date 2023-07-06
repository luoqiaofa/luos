/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : shellLib.c
 * 内容摘要   : 
 * 其它说明   : 
 * 版本       : 
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-06-25 03:46:02 PM
 * 
 * 修改记录1:
 *    修改日期: 2023-06-25
 *    版 本 号: 
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容: 
 * ===========================================================================
 */

/******************************************************************************
 *                #include (依次为标准库头文件、非标准库头文件)               *
 ******************************************************************************/
#include <stdarg.h>
#include <stdio.h>
#include "coreLib.h"

static TCB_ID shellTcb = NULL;
static void *taskShell(void *arg);
LOCAL SEMAPHORE semMuxprint;

#ifndef CONFIG_SYS_PBSIZE
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + 128)
#endif
int shellLibInit(void)
{
    int rc;
    TCB_ID tcb;

    rc = semMInit(&semMuxprint, 0);
    tcb = (TCB_ID)taskCreate("tShell", 10, 0, 4096, taskShell, NULL);
    while(NULL == tcb) {;}
    shellTcb = tcb;
    return rc;
}

TCB_ID shllTcbGet(void)
{
    return shellTcb;
}

#if 0
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
#endif


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

extern int shellCmdlineProcess(char *cmdline);
extern char * readline (const char *prompt);
static void *taskShell(void *arg)
{
    char *cmdline;
    size_t len = 0;
    int cnt = 0;
    int retVal;

    while(true) {
        cnt++;
		// printf("[%s]cnt=%d\n", taskName(taskIdSelf()), cnt);
        cmdline = readline("luos->");
        if (NULL == cmdline) {
            continue;
        }
        len = strlen(cmdline);
        if (len > 0) {
            retVal = shellCmdlineProcess(cmdline);
            Printf("retVal=%d(0x%0x)\n", retVal, retVal);
        }
        // semTake(id, WAIT_FOREVER);
        // taskDelay(2);
    }
    return NULL;
}

