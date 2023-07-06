/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : stdio.c
 * 内容摘要   : 
 * 其它说明   : 
 * 版本       : 
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-07-05 05:47:46 PM
 * 
 * 修改记录1:
 *    修改日期: 2023-07-05
 *    版 本 号: 
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容: 
 * ===========================================================================
 */

/******************************************************************************
 *                #include (依次为标准库头文件、非标准库头文件)               *
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
FILE __stdin;
FILE __stdout;
FILE __stderr;
FILE *stdin = &__stdin; 
FILE *stdout = &__stdout; 
FILE *stderr = &__stderr; 

int fflush(FILE *stream)
{
    return 0;
}

int fprintf(FILE *stream, const char *format, ...)
{
    return 0;
}

