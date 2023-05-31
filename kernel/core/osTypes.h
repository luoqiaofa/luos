/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : os-types.h
 * 内容摘要   : 
 * 其它说明   : 
 * 版本       : 
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-05-15 04:31:36 PM
 * 
 * 修改记录1:
 *    修改日期: 2023-05-15
 *    版 本 号: 
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容: 
 * ===========================================================================
 */

#ifndef __OS_TYPES_H__
#define __OS_TYPES_H__
// #include <stdio.h>
// #include <stdlib.h>
// #include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "autoconf.h"
#include "linux/list.h"
#include "asm/types.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#define FAST          register
#define IMPORT        extern
#define LOCAL         static

typedef int8_t        INT8;
typedef int16_t       INT16;
typedef int32_t       INT32;
typedef int64_t       INT64;

typedef uint8_t       UINT8;
typedef uint16_t      UINT16;
typedef uint32_t      UINT32;
typedef uint64_t      UINT64;

typedef uint8_t       UCHAR;
typedef uint16_t      USHORT;
typedef uint32_t      UINT;
typedef uint64_t      ULONG;

typedef bool          BOOL;
typedef int           STATUS;
typedef int           ARGINT;
typedef void          VOID;

typedef int           (*FUNCPTR) (void);        /*  ptr to function returning int */
typedef void          (*VOIDFUNCPTR) (void);    /*  ptr to function returning void */
typedef double        (*DBLFUNCPTR) (void);     /*  ptr to function returning double*/
typedef float         (*FLTFUNCPTR) (void);     /*  ptr to function returning float */
typedef void*         (*START_RTN)(void *arg);

typedef struct list_head TLIST;

#define OK      0
#define ERROR   (-1)

#endif /* #ifndef __OS_TYPES_H__ */

