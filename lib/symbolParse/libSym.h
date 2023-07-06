/*
 * ===========================================================================
 * 版权所有 (C)2010, BigMrLuo股份有限公司。
 *
 * 文件名称   : symtbl.h
 * 内容摘要   :
 * 其它说明   :
 * 版本       :
 * 作    者   : Luoqiaofa (Luo), qiaofaluo@126.com
 * 创建时间   : 2014年05月27日 13时37分02秒
 *
 * 修改记录1:
 *    修改日期: 2014年05月27日
 *    版 本 号:
 *    修 改 人: Luoqiaofa (Luo), qiaofaluo@126.com
 *    修改内容:
 * ===========================================================================
 */

#ifndef _SYMTBL_H_
#define _SYMTBL_H_

/******************************************************************************
 *                其它条件编译选项                                            *
 ******************************************************************************/


/******************************************************************************
 *                #include（依次为标准库头文件、非标准库头文件）              *
 ******************************************************************************/

/******************************************************************************
 *                常量定义                                                    *
 ******************************************************************************/
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#endif
/******************************************************************************
 *                全局宏(带参数宏)                                            *
 ******************************************************************************/

#define SYM_TYPE_I8     0
#define SYM_TYPE_I16    1
#define SYM_TYPE_I32    2
#define SYM_TYPE_I64    3
#define SYM_TYPE_STR    4
#define SYM_TYPE_FUNC   5
#define SYM_TYPE_T      SYM_TYPE_FUNC
#define SYM_TYPE_D      SYM_TYPE_I32
#define SYM_TYPE_B      SYM_TYPE_I32


#define MAX_SYM_LEN     	80
#define NUM_ARGS        	8
#define USR_SYMTBL_NUM      2


/******************************************************************************
 *                数据类型定义                                                *
 ******************************************************************************/
typedef	char		INT8;
typedef	short		INT16;
typedef	int		INT32;
typedef	long long	INT64;

typedef	unsigned char	UINT8;
typedef	unsigned short	UINT16;
typedef	unsigned int	UINT32;
typedef	unsigned long long UINT64;

typedef	unsigned char	UCHAR;
typedef unsigned short	USHORT;
typedef	unsigned int	UINT;
typedef unsigned long	ULONG;

typedef	int		BOOL;
typedef	int		STATUS;
typedef int 		ARGINT;

typedef void		VOID;

#if NUM_ARGS == 4
typedef int (*CMD_FUNC)(void * arg1, void * arg2, void * arg3, void * arg4);
#elif NUM_ARGS == 8
typedef int (*CMD_FUNC)(void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6, void *arg7, void *arg8);
#else
typedef int (*CMD_FUNC)(void * arg1, void * arg2, void * arg3, void * arg4);
#endif

typedef struct t_symbol
{
    char *name;
    char  symType;
    void *sym;
} TsymPara;

#if 0
#define SYMBOL_TBL_HASH
#endif
#ifdef SYMBOL_TBL_HASH
#define SYMBOL_HASH_HAED_SZ 1024
typedef struct t_symbol_hash_data {
    TList list;
    const TsymPara *psym;
} TSymHashData;
#endif

typedef TsymPara SYMBOL;

/******************************************************************************
 *                函数原型声明                                                *
 ******************************************************************************/
int sysSymTblInit(void);
int sysSymTblDeinit(void);
int shellCmdlineProcess(char *cmdline);
int sysSymTblAdd(const TsymPara *ptTbl, int NumElement);
TsymPara *prefixSymMatched(char *name, int *num_item);
int shellOutputFdSet(int fd);

// void symsDumpToFile(FILE *fp);
// int symbolDump2File(char *fpath);

#endif /* #ifndef _SYMTBL_H_ */


