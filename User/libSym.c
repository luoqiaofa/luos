#include <stdio.h>
#include <stdarg.h>

#ifdef SYMBOL_TBL_HASH
#include "linux/list.h"
#endif
#include "str.h"
#include "libSym.h"

#define DBG_SYM 0
#define SYM_SORTED 1

#define sys_printf(fmt, args...) printf(fmt " \n", ## args)


#if DBG_SYM
#define sym_dbg(format, args...) sys_printf("[%s,%d]: " format "\n", __FILE__, __LINE__, ## arg)
#define SYM_LOG(format, args...) sys_printf("[%s,%d]: " format "\n", __FILE__, __LINE__, ## arg)
#else
#define sym_dbg(fmt, args...)
#define SYM_LOG(fmt, args...)  printf(fmt " \n", ## args)
#endif

struct t_libSymTbl {
    TsymPara *SymTblBase;
    int	      numSym;
    struct t_libSymTbl *ptNext;
};

static int bin(int val);
static int hex(int val);
static int oct(int val);
static int symLookup(char *name);
static int sys_fun_test(int arg1,int arg2,char * arg3,char * arg4);
static int clear(void);
static const TsymPara * symFindByName(char *name);
static int symKeyGen(void);
static int md(void * addr, int len, int data_type);
static int mm(void * addr, int value, int data_type);
static int prefix_test(char *prefix);
static size_t BKDRHash(const char *str);
static int shellPrint(const char *fmt, ...);
static int sys_fun_repeat(int repeat_times, void *funptr, void *arg1, void *arg2, void *arg3, void *arg4);

static inline int funcRunning(CMD_FUNC pfunc, int argc, void *argv[])
{
    int rc;

#if NUM_ARGS == 4
    rc = (*pfunc)(argv[0], argv[1], argv[2], argv[3]);
#elif NUM_ARGS == 8
    rc = (*pfunc)(argv[0], argv[1], argv[2], argv[3],
            argv[4], argv[5], argv[6], argv[7]);
#else
    rc = (*pfunc)(argv[0], argv[1], argv[2], argv[3]);
#endif
    return rc;
}

static inline void strToNumber(void **ptr, const char *s)
{
#if __BITS_PER_LONG == 64
    *ptr = (void *)((char *)NULL + strToInt64(s));
#else
    *ptr = (void *)((char *)NULL + strToInt(s));
#endif
}

static inline bool isSymbolPrefix(char ch)
{
    if (isalpha(ch) || ('_' == ch)) {
        return true;
    }
    return false;
}

static inline bool isSymbolChar(char ch)
{
    if (isalpha(ch) || ('_' == ch) || isdigit(ch)) {
        return true;
    }
    return false;
}

static inline bool isSymbolEnd(char ch)
{
    if (('\0' == ch) || isspace(ch) || ('=' == ch) || ('(' == ch) || ('[' == ch)) {
        return true;
    }
    return false;
}

static bool isValidSymbol(char *s)
{
    if (!isSymbolPrefix(*s)) {
        return false;
    }
    s++;
    while (*s) {
        if (isspace(*s)) {
            break;
        }
        if (!isSymbolChar(*s)) {
            return false;
        }
        s++;
    }
    return true;
}

/*
 * 转义字符  意义            ASCII码值（十进制）
 * \a        响铃(BEL)       007
 * \b        退格(BS)        008
 * \f        换页(FF)        012
 * \n        换行(LF)        010
 * \r        回车(CR)        013
 * \t        水平制表(HT)    009
 * \v        垂直制表(VT)    011
 * \\        反斜杠          092
 * \?        问号字符        063
 * \'        单引号字符      039
 * \"        双引号字符      034
 * \0        空字符(NULL)    000
 * \ddd      任意字符        三位八进制
 * \xhh      任意字符        二位十六进制
*/
static inline int escapeCharacterDeal(char *s)
{
    int i = 2,j = 0;

    if ('\\' != *s) {return -1;}
    if ('\0' == s[1]) {return 0;}
    switch(s[1]) {
        case 'a': /* \a        响铃(BEL)       007 */
            s[j] = '\a';
            break;
        case 'b': /* \b        退格(BS)        008 */
            s[j] = '\b';
            break;
        case 'f': /* \f        换页(FF)        012 */
            s[j] = '\f';
            break;
        case 'n': /* \n        换行(LF)        010  */
            s[j] = '\n';
            break;
        case 'r': /* \r        回车(CR)        013 */
            s[j] = '\r';
            break;
        case 't': /* \t        水平制表(HT)    009 */
            s[j] = '\t';
            break;
        case 'v': /* \v        垂直制表(VT)    011 */
            s[j] = '\v';
            break;
        case '\\': /* \\        反斜杠          092 */
            s[j] = '\\';
            break;
        case '?': /* \?        问号字符        063 */
            s[j] = '\?';
            break;
        case '\'': /* \'        单引号字符      039 */
            s[j] = '\'';
            break;
        case '"': /* \"        双引号字符      034 */
            s[j] = '"';
            break;
        case '0': /* \0        空字符(NULL)    000 */
            s[j] = '\0';
            break;
        case 'x': /* \0        空字符(NULL)    000 */
            if (strlen(s+2) < 2) {
                return 0;
            }
            if (isxdigit(s[2]) && isxdigit(s[3])) {
                s[j] = (tolower(s[2]) - '0') * 16;
                if (tolower(s[2] >= 'a')) {
                    s[j] = (tolower(s[2]) - 'a' + 10) * 16;
                }
                if (tolower(s[3]) >= 'a') {
                    s[j] += (tolower(s[3]) - 'a' + 10);
                } else {
                    s[j] += (tolower(s[3]) - '0');
                }
                i += 2;
            } else {
                return 0; /* do nothing */
            }
            break;
        default :
            if (strlen(s+1) < 3) {
                return 0;
            }
            if (isodigit(s[1]) && isodigit(s[2]) && isodigit(s[3])) {
                i += 2;
                s[j] = (s[1] - '0') * 64 + (s[2] - '0') * 8 + (s[3] - '0');
            } else {
                return 0; /* do nothing */
            }
            break;
    }
    j++;
    while (s[i]) {
        s[j] = s[i];
        j++;
        i++;
    }
    s[j] = '\0';
    return 0;
}

static int s_output_fd = -1;

static int test_data[10] = {0x12345678, 1, 2, 3, 4, 5, 6, 7, 8, 9};
#ifdef SYMBOL_TBL_HASH
TList s_sym_hash_head[SYMBOL_HASH_HAED_SZ];
TSymHashData s_sym_hashdata[SYMBOL_HASH_HAED_SZ];
#endif

const static TsymPara g_sysSymTbl[] =
{
    {"bin",             SYM_TYPE_FUNC, (void *)bin},
    {"clear",           SYM_TYPE_FUNC, (void *)clear},
    {"hex",             SYM_TYPE_FUNC, (void *)hex},
    {"lkup",            SYM_TYPE_FUNC, (void *)symLookup},
    {"md",              SYM_TYPE_FUNC, (void *)md},
    {"mm",              SYM_TYPE_FUNC, (void *)mm},
    {"oct",             SYM_TYPE_FUNC, (void *)oct},
    {"prefix_test",  	SYM_TYPE_FUNC, (void *)prefix_test},
    {"symKeyGen"    ,	SYM_TYPE_FUNC, (void *)symKeyGen},
    {"sys_data_test",   SYM_TYPE_I32,  (void *)&test_data},
    {"sys_fun_repeat",  SYM_TYPE_FUNC, (void *)sys_fun_repeat},
    {"sys_fun_test" ,	SYM_TYPE_FUNC, (void *)sys_fun_test},
} ;


static struct t_libSymTbl g_sysSymLib[USR_SYMTBL_NUM + 1];

/*
 * ===========================================================================
 * 函数名称: sysSymTblInit
 * 功能描述: 系统/内建符号表初始化
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值：无
 * 其它说明：
 * 修改日期        版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2019/10/11  V1.0    罗乔发         创建
 * ===========================================================================
 */
int sysSymTblInit(void)
{
    int lp = 0;
    int ret = 0;
#ifdef SYMBOL_TBL_HASH
    TList *head;

    for (lp = 0; lp < SYMBOL_HASH_HAED_SZ; lp++) {
        head = &s_sym_hash_head[lp];
        INIT_LIST_HEAD(head);
    }
    ret = sysSymTblAdd(&g_sysSymTbl[0], ARRAY_SIZE(g_sysSymTbl));
#else
    struct t_libSymTbl *ptlibSym = &g_sysSymLib[0];
    memset((void *)&g_sysSymLib[0], 0, sizeof(g_sysSymLib));

    for (lp = 0; lp < ARRAY_SIZE(g_sysSymLib) - 1; lp++)
    {
        ptlibSym->ptNext = ptlibSym + 1;
        ptlibSym++;
    }
    ret = sysSymTblAdd(&g_sysSymTbl[0], ARRAY_SIZE(g_sysSymTbl));
#endif
    return ret;
}

int sysSymTblDeinit(void)
{
#ifdef SYMBOL_TBL_HASH
    int lp;
    TList *head;
    TSymHashData *pos;
    int idx = 0;
    int num_syms;
    TSymHashData *hashdata[50];

    for (lp = 0; lp < SYMBOL_HASH_HAED_SZ; lp++) {
        head = &s_sym_hash_head[lp];
        if (list_empty(head)) {
            continue;
        }
        pos = &s_sym_hashdata[lp];
        list_del(&pos->list);
        if (list_empty(head)) {
            continue;
        }
        num_syms = 0;
        list_for_each_entry(pos, head, list) {
            if (num_syms < 50) {
                hashdata[num_syms] = pos;
                num_syms++;
            }
        }
        for (idx = 0; idx < num_syms; idx++) {
            pos = hashdata[idx];
            sym_dbg("symbol=%s, hash_key=%d, idx=%d", pos->psym->name, lp, idx);
            list_del(&pos->list);
            free(pos);
            pos = NULL;
        }
    }
#endif
    s_output_fd = -1;
    return 0;
}

/*
 * ===========================================================================
 * 函数名称: sysSymTblAdd
 * 功能描述: 添加符号表到本模块
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值：无
 * 其它说明：
 * 修改日期        版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2019/10/11  V1.0    罗乔发         创建
 * ===========================================================================
 */
int sysSymTblAdd(const TsymPara *ptTbl, int NumElement)
{
#ifdef SYMBOL_TBL_HASH
    int lp;
    TList *head;
    TSymHashData *symhash;
    TSymHashData *pos;
    size_t hash_key;
    const TsymPara *symtbl;

    for (lp = 0; lp < NumElement; lp++) {
        symtbl = ptTbl + lp;
        hash_key = BKDRHash(symtbl->name);
        hash_key = hash_key % SYMBOL_HASH_HAED_SZ;
        head = &s_sym_hash_head[hash_key];
        if (list_empty(head)) {
            symhash = &s_sym_hashdata[hash_key];
            symhash->psym = symtbl;
            list_add(&symhash->list, head);
            sym_dbg("symbol=%s, hash_key=%d", symtbl->name, hash_key);
        } else {
            list_for_each_entry(pos, head, list) {
                if (0 == strcmp(pos->psym->name, symtbl->name)) {
                    SYM_LOG("<%s,%d>Error: symtbol=%s existed", __func__, __LINE__, symtbl->name);
                    return -EEXIST;
                }
            }
            symhash = (TSymHashData *)malloc(sizeof(TSymHashData));
            if (NULL == symhash) {
                SYM_LOG("<%s,%d>malloc failed", __func__, __LINE__);
                return -ENOMEM;
            }
            symhash->psym = symtbl;
            list_add(&symhash->list, head);
            sym_dbg("symbol=%s, hash_key=%d\n", symtbl->name, hash_key);
        }
    }
#else
    struct t_libSymTbl *ptlibSym = &g_sysSymLib[0];
    while (ptlibSym->ptNext) {
        if (NULL == ptlibSym->SymTblBase) {
            break;
        }
        ptlibSym = ptlibSym->ptNext;
    }
    if (ptlibSym->ptNext) {
        ptlibSym->SymTblBase = (TsymPara *)ptTbl;
        ptlibSym->numSym     = NumElement;
        return 0;
    }
#endif
    return -1;
}


/*
 * ===========================================================================
 * 函数名称: BKDRHash
 * 功能描述: BKDRHash 算法
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值：无
 * 其它说明：
 * 修改日期        版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2019/10/11  V1.0    罗乔发         创建
 * ===========================================================================
 */
static size_t BKDRHash(const char *str)
{
    size_t hash = 0;
    size_t ch;

    ch = (size_t)*str;
    while (ch) {
        hash = hash * 13131 + ch;
        str++;
        ch = (size_t)*str;
    }
    return hash;
}

/*
 * ===========================================================================
 * 函数名称: symFindByName
 * 功能描述: 查找符号名是否在符号表中
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值：无
 * 其它说明：
 * 修改日期        版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2019/10/11  V1.0    罗乔发         创建
 * ===========================================================================
 */
static const TsymPara * symFindByName(char *name)
{
    size_t len1 = strlen(name);
    size_t len2;
#ifdef SYMBOL_TBL_HASH
    TList *head;
    TSymHashData *pos;
    size_t hash_key;
    const TsymPara *symtbl;

    hash_key = BKDRHash(name);
    hash_key = hash_key % SYMBOL_HASH_HAED_SZ;
    head = &s_sym_hash_head[hash_key];
    if (list_empty(head)) {
        return NULL;
    }
    list_for_each_entry(pos, head, list) {
        symtbl = pos->psym;
        if (0 == strcmp(name, symtbl->name)) {
            len2 = strlen(symtbl->name);
            if (len1 == len2) {
                return symtbl;
            }
        }
    }
    return NULL;
#else
    int lp = 0;
    TsymPara *pTbl;
    size_t len;
    struct t_libSymTbl *ptlibSym = &g_sysSymLib[0];
    int hlf_idx = 0;
    int low;
    int high;
    int ret = 0;

#if SYM_SORTED == 1
    while (ptlibSym) {
        if (NULL != ptlibSym->SymTblBase) {
            /* 鑻ョ?﹀彿琛ㄦ寜椤哄簭鎺掑簭锛屽垯浣跨敤鎶樺崐鏌ユ壘绠???? */
            low = 0;
            high = (ptlibSym->numSym - 1);
            lp = 0;
            while (low <= high) {
                lp++;
                if ((lp / 2) > (ptlibSym->numSym)){
                    /* 闃叉?㈠紓甯告?诲惊??? */
                    break;
                }
                hlf_idx = (low + high) / 2;
                pTbl = ptlibSym->SymTblBase + hlf_idx;
                len2 = strlen(pTbl->name);
                len = (len1 > len2) ? len1 : len2;
                ret = strncmp(name, pTbl->name, len);
                sym_dbg("hlf_idx=%d,low=%d,high=%d,sym=%s,ret=%d,lp=%d", hlf_idx, low, high, pTbl->name,ret,lp);
                if (0 == ret) {
                    return pTbl;
                } else if (ret > 0) {
                    low = hlf_idx + 1;
                } else {
                    high = hlf_idx - 1;
                }
            }
        }
        ptlibSym = ptlibSym->ptNext;
    }
#endif
#endif

    return NULL;
}

/*
 * ===========================================================================
 * 函数名称: digitalStrCheck
 * 功能描述: 数字字符串合法性检查
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值：无
 * 其它说明：
 * 修改日期        版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2019/10/11  V1.0    罗乔发         创建
 * ===========================================================================
 */
static int digitalStrCheck(char *str, int forceDec)
{
    if (isValidNumber(str)) {
        return 0;
    }
    return -1;
}

/*
 * ===========================================================================
 * 函数名称: dataValueParse
 * 功能描述: 变量是否赋值解析
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值：无
 * 其它说明：
 * 修改日期        版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2019/10/11  V1.0    罗乔发         创建
 * ===========================================================================
 */
static int  dataValueParse(char *argLine, char *valued, int *value, int *idx, int8_t *is_arr)
{
    char *p1, *p2;
    int ret = 0;
    char buf[20];

    sym_dbg("argLine: %s", argLine);

    if (NULL != is_arr) {
        *is_arr = 0;
    }
    p1 = strSpaceSkip(argLine);
    if ('[' == *p1) {
        p1 = strSpaceSkip(p1 + 1);
        p2 = p1;
        while('\0' != *p2) {
            if (']' == *p2) {
                break;
            }
            p2++;
        }
        if (']' != *p2) {
            sym_dbg("");
            return -1;
        }
        if (p2 == p1) {
            sym_dbg("");
            return -2;
        }
        if (NULL != is_arr) {
            *is_arr = 1;
        }
        memset(buf, 0, sizeof(buf));
        memcpy(buf, p1, p2 - p1);
        ret = digitalStrCheck(buf, 0);
        sym_dbg("ret=%d, str=%s, forceDec=%d", ret, buf, 0);
        if (ret < 0) {
            SYM_LOG("Digital format err");
            return -3;
        }
        if (NULL != idx) {
            *idx = strToInt(buf);
            sym_dbg("index=%d(0x%x)", *idx, *idx);
        }

        p1 = strSpaceSkip(p2+1);
    }

    if ('=' == *p1) {
        p1 = strSpaceSkip(p1 + 1);
        ret = digitalStrCheck(p1, 0);
        sym_dbg("ret=%d, str=%s, forceDec=%d", ret, p1, 0);
        if (ret < 0) {
            SYM_LOG("Digital format err");
            return -4;
        }
        if (valued) {
            *valued = 1;
            if (value) {
                *value = strToInt(p1);
            }
        }
    }
    else if ('\0' != *p1) {
        sym_dbg("");
        SYM_LOG("Syntax err");
        return -5;
    }
    else {
        if (valued) {
            *valued = 0;
        }
    }
    return 0;
}

/*
 * ===========================================================================
 * 函数名称: strValueParse
 * 功能描述: 字符串类型解析
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值：无
 * 其它说明：
 * 修改日期        版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2019/10/11  V1.0    罗乔发         创建
 * ===========================================================================
 */
static int  strValueParse(char *argLine, char * valued, char **output)
{
    sym_dbg("argLine=%s", argLine);
    return 0;
}

enum eFuncArgsCharSM {
    eSM_ARGTYPE_IDLE        = 0,
    eSM_ARGTYPE_NUMBER_PRE1 = 1, /* prefix +,- */
    eSM_ARGTYPE_NUMBER_PRE2 = 2, /* prefix 0, [1-9] */
    eSM_ARGTYPE_NUMBER_PRE3 = 3, /* prefix x,o,b,digit */
    eSM_ARGTYPE_SYMBOL_PRE  = 4,
    eSM_ARGTYPE_STRING_PRE  = 5,
    eSM_ARGTYPE_DIGIT       = 6,
    eSM_ARGTYPE_XDIGIT      = 7,
    eSM_ARGTYPE_ODIGIT      = 8,
    eSM_ARGTYPE_BINARY      = 9,
    eSM_ARGTYPE_SYMBOL_CHAR = 10,
    eSM_ARGTYPE_STRING_CHAR = 11,
    eSM_ARGTYPE_STRING_END  = 12,
    eSM_ARGTYPE_ARG_SPLIT   = 13,
    eSM_ARGTYPE_NUM
};


/*
 * ===========================================================================
 * 函数名称: funcArgsParse
 * 功能描述: 函数参数解析
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值：无
 * 其它说明：
 * 修改日期        版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2019/10/11  V1.0    罗乔发         创建
 * ===========================================================================
 */
static int funcArgsParse(char *argLine, int *argc, void * argv[])
{
    int rc = 0;
    char *p, *pre, *p1, ch;
    const TsymPara *ptSym = NULL;
    int narg = 0;
    char *segv[NUM_ARGS];
    enum eFuncArgsCharSM esm_char = eSM_ARGTYPE_IDLE;
    bool bl_comma_sep = false;
    int base = 10;

    sym_dbg("argLine: %p=%s", argLine, argLine);

    *argc = 0;
    p = strSpaceSkip(argLine);
    if (NULL == p || '\0' == p) {
        sym_dbg("no arguments", p - argLine, p);
        return 0;
    }
    if ('(' == *p) {
        p++;
        p = strStrip(p, 0, &rc);
        if (NULL == p || rc < 1 || ')' != p[rc - 1]) {
            SYM_LOG("Syntax error: don't end with )");
            sym_dbg("state=%d, c=%c", esm_char, *p);
            return -1;
        }
        p[rc - 1] = '\0';
        bl_comma_sep = true;
    }
    p = strStrip(p, 0, &rc);
    if (NULL == p || 0 == rc) {
        sym_dbg("no arguments", p - argLine, p);
        return 0;
    }
    p1 = p;
    pre = p;
    while (*p) {
        if (isspace(*p) || (',' == *p)) {
            if (eSM_ARGTYPE_STRING_CHAR == esm_char || eSM_ARGTYPE_STRING_PRE == esm_char) {
                if (eSM_ARGTYPE_STRING_PRE == esm_char) {
                    esm_char = eSM_ARGTYPE_STRING_CHAR;
                }
                pre = p;
                p++;
                continue;
            }
            if (',' == *p) {
                if (!bl_comma_sep) {
                    SYM_LOG("Syntax error: invalid comma!");
                    sym_dbg("state=%d, c=%c", esm_char, *p);
                    return -1;
                }
            } else {
                /* space char branch */
                if (bl_comma_sep) {
                    pre = p;
                    p++;
                    continue;
                }
            }
            switch(esm_char) {
                case eSM_ARGTYPE_STRING_PRE:
                    esm_char = eSM_ARGTYPE_STRING_CHAR;
                    break;
                case eSM_ARGTYPE_STRING_CHAR:
                    break;
                case eSM_ARGTYPE_SYMBOL_PRE:
                    SYM_LOG("Syntax error: invalid arg!");
                    sym_dbg("state=%d, c=%c", esm_char, *p);
                    return -1;
                    break;
                case eSM_ARGTYPE_SYMBOL_CHAR:
                    esm_char = eSM_ARGTYPE_ARG_SPLIT;
                    *p = '\0';
                    segv[narg] = strStrip(segv[narg], 0, NULL);
                    ptSym = symFindByName(segv[narg]);
                    if (NULL == ptSym) {
                        SYM_LOG("Undefined symbol: %s", segv[narg]);
                        sym_dbg("state=%d, c=%c", esm_char, *p);
                        return -1;
                    }
                    argv[narg] = ptSym->sym;
                    sym_dbg("argv[%d]=%s(%p)", narg, ptSym->name, argv[narg]);
                    narg++;
                    break;
                case eSM_ARGTYPE_NUMBER_PRE2:
                case eSM_ARGTYPE_DIGIT:
                case eSM_ARGTYPE_XDIGIT:
                case eSM_ARGTYPE_ODIGIT:
                case eSM_ARGTYPE_BINARY:
                    esm_char = eSM_ARGTYPE_ARG_SPLIT;
                    *p = '\0';
                    strToNumber(argv+narg, segv[narg]);
                    sym_dbg("argv[%d]=%ld", narg, (long)argv[narg]);
                    narg++;
                    break;
                case eSM_ARGTYPE_STRING_END:
                    esm_char = eSM_ARGTYPE_ARG_SPLIT;
                    *p = '\0';
                    argv[narg] = segv[narg];
                    sym_dbg("argv[%d]=%p, %s", narg, argv[narg], NULL == argv[narg] ? "null":argv[narg]);
                    narg++;
                    break;
                case eSM_ARGTYPE_ARG_SPLIT:
                    if (',' == *p) {
                        SYM_LOG("Syntax error: duplicate comma split!");
                        sym_dbg("state=%d", esm_char);
                        return -1;
                    }
                    *p = '\0';
                    break;
                default:
                    SYM_LOG("Syntax error!");
                    sym_dbg("state=%d, c=%c", esm_char, *p);
                    return -1;
                    break;
            }
        } else if ('\\' == *p) {
            if (eSM_ARGTYPE_STRING_CHAR != esm_char && eSM_ARGTYPE_STRING_PRE != esm_char) {
                SYM_LOG("Syntax error: number char format error!");
                sym_dbg("state=%d, c=%c", esm_char, *p);
                return -1;
            }
            if (eSM_ARGTYPE_STRING_PRE == esm_char) {
                esm_char == eSM_ARGTYPE_STRING_CHAR;
            }
            rc = escapeCharacterDeal(p);
            sym_dbg("after escape deal: %s", p);
            if (rc < 0) {
                SYM_LOG("Syntax error: escape char format error!");
                sym_dbg("state=%d, c=%c", esm_char, *p);
                return -1;
            }
        } else {
            /* normal char */
            switch (esm_char) {
                case eSM_ARGTYPE_IDLE:
                case eSM_ARGTYPE_ARG_SPLIT:
                    if (isSymbolPrefix(*p)) {
                        esm_char = eSM_ARGTYPE_SYMBOL_PRE;
                        if (isalpha(*p)) {
                            esm_char = eSM_ARGTYPE_SYMBOL_CHAR;
                        }
                        segv[narg] = p;
                    } else if (isNumberPre(*p)) {
                        if (isdigit(*p)) {
                            esm_char = eSM_ARGTYPE_DIGIT;
                            if ('0' == *p) {
                                esm_char = eSM_ARGTYPE_NUMBER_PRE2;
                            }
                        } else {
                            esm_char = eSM_ARGTYPE_NUMBER_PRE1;
                        }
                        segv[narg] = p;
                    } else if ('"' == *p) {
                        segv[narg] = p + 1;
                        esm_char = eSM_ARGTYPE_STRING_PRE;
                    } else {
                        SYM_LOG("Syntax error: invalid arg!");
                        sym_dbg("state=%d, c=%c", esm_char, *p);
                        return -2;
                    }
                    break;
                case eSM_ARGTYPE_STRING_PRE:
                    esm_char = eSM_ARGTYPE_STRING_CHAR;
                case eSM_ARGTYPE_STRING_CHAR:
                    if ('"' == *p) {
                        *p = '\0';
                        esm_char = eSM_ARGTYPE_STRING_END;
                    }
                    break;
                case eSM_ARGTYPE_NUMBER_PRE1:
                    if (!isdigit(*p)) {
                        SYM_LOG("Syntax error: number format error!");
                        sym_dbg("state=%d, c=%c", esm_char, *p);
                        return -1;
                    }
                    if ('0' == ch) {
                        esm_char = eSM_ARGTYPE_NUMBER_PRE2;
                    } else {
                        esm_char = eSM_ARGTYPE_DIGIT;
                    }
                    break;
                case eSM_ARGTYPE_NUMBER_PRE2:
                    ch = tolower(*pre);
                    if (isdigit(ch)) {
                        base = 10;
                        esm_char = eSM_ARGTYPE_DIGIT;
                    } else if ('x' == ch) {
                        base = 16;
                        esm_char = eSM_ARGTYPE_NUMBER_PRE3;
                    } else if ('o' == ch) {
                        base = 8;
                        esm_char = eSM_ARGTYPE_NUMBER_PRE3;
                    } else if ('b' == ch) {
                        base = 2;
                        esm_char = eSM_ARGTYPE_NUMBER_PRE3;
                    } else {
                        SYM_LOG("Syntax error: number format error!");
                        sym_dbg("state=%d, c=%c", esm_char, *p);
                        return -1;
                    }
                    break;
                case eSM_ARGTYPE_DIGIT:
                case eSM_ARGTYPE_XDIGIT:
                case eSM_ARGTYPE_ODIGIT:
                case eSM_ARGTYPE_BINARY:
                case eSM_ARGTYPE_NUMBER_PRE3:
                    if (10 == base) {
                        if (!isdigit(*p)) {
                            SYM_LOG("Syntax error: number format error!");
                            sym_dbg("state=%d, c=%c", esm_char, *p);
                            return -1;
                        }
                        esm_char = eSM_ARGTYPE_DIGIT;
                    } else if (16 == base) {
                        if (!isxdigit(*p)) {
                            SYM_LOG("Syntax error: number format error!");
                            sym_dbg("state=%d, c=%c", esm_char, *p);
                            return -1;
                        }
                        esm_char = eSM_ARGTYPE_XDIGIT;
                    } else if (8 == base) {
                        if (!isodigit(*p)) {
                            SYM_LOG("Syntax error: number format error!");
                            sym_dbg("state=%d, c=%c", esm_char, *p);
                            return -1;
                        }
                        esm_char = eSM_ARGTYPE_ODIGIT;
                    } else if (2 == base) {
                        if (!isbinary(*p)) {
                            SYM_LOG("Syntax error: number format error!");
                            sym_dbg("state=%d, c=%c", esm_char, *p);
                            return -1;
                        }
                        esm_char = eSM_ARGTYPE_BINARY;
                    } else {
                    }
                    break;
                case eSM_ARGTYPE_SYMBOL_PRE:
                    if (isalpha(*p)) {
                        esm_char = eSM_ARGTYPE_SYMBOL_CHAR;
                    }
                case eSM_ARGTYPE_SYMBOL_CHAR:
                    if (!isSymbolChar(*p)) {
                        SYM_LOG("Syntax error, invalid symbol char!");
                        sym_dbg("state=%d, c=%c", esm_char, *p);
                        return -1;
                    }
                    break;
                default:
                    SYM_LOG("Syntax error, invalid character!");
                    sym_dbg("state=%d, c=%c", esm_char, *p);
                    return -1;
                    break;
            }
        }
        pre = p;
        p++;
    }
    sym_dbg("state=%d", esm_char);
    switch(esm_char) {
        case eSM_ARGTYPE_SYMBOL_PRE:
            SYM_LOG("Syntax error: invalid symbol name!");
            return -1;
            break;
        case eSM_ARGTYPE_SYMBOL_CHAR:
            segv[narg] = strStrip(segv[narg], 0, NULL);
            argv[narg] = NULL;
            ptSym = symFindByName(segv[narg]);
            if (NULL == ptSym) {
                SYM_LOG("Undefined symbol: %s", segv[narg]);
                return -1;
            }
            argv[narg] = ptSym->sym;
            sym_dbg("argv[%d]=%s(%p)", narg, ptSym->name, argv[narg]);
            narg++;
            break;
        case eSM_ARGTYPE_NUMBER_PRE2:
        case eSM_ARGTYPE_DIGIT:
        case eSM_ARGTYPE_XDIGIT:
        case eSM_ARGTYPE_ODIGIT:
        case eSM_ARGTYPE_BINARY:
            strToNumber(argv+narg, segv[narg]);
            sym_dbg("argv[%d]=%ld", narg, (long)argv[narg]);
            narg++;
            break;
        case eSM_ARGTYPE_STRING_END:
            argv[narg] = segv[narg];
            sym_dbg("argv[%d]=%p, %s", narg, argv[narg], NULL == argv[narg] ? "null":argv[narg]);
            narg++;
            break;
        case eSM_ARGTYPE_ARG_SPLIT:
            if (bl_comma_sep) {
                SYM_LOG("Syntax error: cmdline endwith ','", esm_char);
                return -1;
            }
            break;
        default:
            if (eSM_ARGTYPE_STRING_CHAR == esm_char || eSM_ARGTYPE_STRING_PRE == esm_char) {
                SYM_LOG("Syntax error, string don't end with '\"'", esm_char);
            } else {
                SYM_LOG("Syntax error, sm=%d", esm_char);
            }
            return -1;
            break;
    }
    *argc = narg;
    return 0;
}

/*
 * ===========================================================================
 * 函数名称: clangSymbolGet
 * 功能描述: 从命令行中获取命令名
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值：无
 * 其它说明：
 * 修改日期        版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2019/10/11  V1.0    罗乔发         创建
 * ===========================================================================
 */
static char * clangSymbolGet(char *cmdline, char *sym, char **tok)
{
    char *p1,*p2;

    sym_dbg("cmdline=%s", cmdline);

    p1 = strSpaceSkip(cmdline);
    if (!isSymbolPrefix(*p1)) {
        sym_dbg("");
        return NULL;
    }

    for (p2 = p1 + 1; *p2 != '\0'; p2++) {
        if (!isSymbolChar(*p2)) {
            sym_dbg("p2=%c", *p2);
            break ;
        }
    }
    if (isSymbolEnd(*p2)) {
        memcpy(sym, p1, p2 - p1);
        sym[p2 - p1] = '\0';
        if (NULL != tok) {
            *tok = p2;
        }
        return p1;
    }

    return NULL;
}


/*
 * ===========================================================================
 * 函数名称: shellCmdlineProcess
 * 功能描述: 执行命令行
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值：无
 * 其它说明：
 * 修改日期        版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2019/10/11  V1.0    罗乔发         创建
 * ===========================================================================
 */
int shellCmdlineProcess(char *cmdline)
{
    int ret = -1;
    char symbol[MAX_SYM_LEN];
    CMD_FUNC pfunc = NULL;
    const TsymPara * ptSym = NULL;
    int argc = 0;
    void * argv[NUM_ARGS];
    char *argLine = NULL;
    char *tmp;
    char valued = 0;
    int value = 0;
    int idx = 0;
    int8_t is_arr = 0;
    int8_t *addr8 = NULL;
    int16_t *addr16 = NULL;
    int32_t *addr32 = NULL;
    int64_t *addr64 = NULL;

    sym_dbg("cmdline=%s", cmdline);

    memset(symbol, 0, sizeof(symbol));
    tmp = clangSymbolGet(cmdline, symbol, &argLine);
    sym_dbg("symbol=%s", tmp? tmp:"null");
    if (NULL == tmp) {
        SYM_LOG("Syntax error");
        return -1;
    }
    sym_dbg("symbol=%s", symbol);
    ptSym = symFindByName(symbol);
    if (NULL == ptSym) {
        SYM_LOG("Undefined symbol: %s", symbol);
        return -2;
    }
    sym_dbg("symName=%s,symType=%d,symAddr=0x%x", ptSym->name, ptSym->symType, (unsigned long)ptSym->sym);
    /*  memcpy(line_bak, argLine, strlen(argLine) + 1);
    */
    switch (ptSym->symType) {
        case SYM_TYPE_I8  :
        case SYM_TYPE_I16 :
        case SYM_TYPE_I32 :
        case SYM_TYPE_I64 :
            valued = 0;
            value  = 0;
            if (NULL != argLine) {
                ret = dataValueParse(argLine, &valued, &value, &idx, &is_arr);
                sym_dbg("ret=%d,valued=%d,value=%d,idx=%d", ret, valued, value, idx);
                if (ret != 0) {
                    sym_dbg("dataValueParse return err(ret=%d)", ret);
                    return -1;
                }
            }
            if (0 != ret) {
                return 0;
            }

            addr8 = (int8_t *)ptSym->sym;
            addr8 += idx;
            addr16 = (int16_t *)ptSym->sym;
            addr16 += idx;
            addr32 = (int32_t *)ptSym->sym;
            addr32 += idx;
            addr64 = (int64_t *)ptSym->sym;
            addr64 += idx;

            if (valued) {
                if(SYM_TYPE_I8 == ptSym->symType) {
                    *addr8 = (int8_t)value;
                } else if(SYM_TYPE_I16 == ptSym->symType) {
                    *addr16 = (int16_t)value;
                } else if(SYM_TYPE_I32 == ptSym->symType) {
                    *addr32 = (int32_t)value;
                } else if(SYM_TYPE_I64 == ptSym->symType) {
                    *addr64 = (int64_t)value;
                } else {
                }

            }
            switch (ptSym->symType) {
                case SYM_TYPE_I8:
                    if (is_arr) {
                        SYM_LOG("%s[%d](addr=0x%lx) = %d(0x%x)", ptSym->name, idx, (char *)addr8, *addr8, *addr8);
                    } else {
                        SYM_LOG("%s(addr=0x%lx) = %d(0x%x)", ptSym->name, (char *)addr8, *addr8, *addr8);
                    }
                    break;
                case SYM_TYPE_I16:
                    if (is_arr) {
                        SYM_LOG("%s[%d](addr=0x%lx) = %d(0x%x)", ptSym->name, idx, (char *)addr16, *addr16, *addr16);
                    } else {
                        SYM_LOG("%s(addr=0x%lx) = %d(0x%x)", ptSym->name, (char *)addr16, *addr16, *addr16);
                    }
                    break;

                case SYM_TYPE_I32:
                    if (is_arr) {
                        SYM_LOG("%s[%d](addr=0x%lx) = %d(0x%x)", ptSym->name, idx, (char *)addr32, *addr32, *addr32);
                    } else {
                        SYM_LOG("%s(addr=0x%lx) = %d(0x%x)", ptSym->name, (char *)addr32, *addr32, *addr32);
                    }
                    break;
                case SYM_TYPE_I64:
                    if (is_arr) {
                        SYM_LOG("%s[%d](addr=0x%lx) = %ld(0x%x)", ptSym->name, idx, (char *)addr32, *addr32, *addr32);
                    } else {
                        SYM_LOG("%s(addr=0x%lx) = %ld(0x%x)", ptSym->name, (char *)addr32, *addr32, *addr32);
                    }
                    break;

                default :
                    break;
            }

            break;
        case SYM_TYPE_STR :
            ret = strValueParse(argLine, &valued, &tmp);
            sym_dbg("ret=%d,valued=%d,value=%s", ret, valued, tmp);
            if (0 == ret) {
                if (valued) {
                    strcpy(ptSym->sym, tmp);
                }
                else {
                    SYM_LOG("%s = %s", ptSym->name, (char *)ptSym->sym);
                }
            }
            break;

        case SYM_TYPE_FUNC:
            memset(argv, 0, sizeof(argv));
            ret = funcArgsParse(argLine, &argc, argv);
            if (0 == ret) {
                ret = funcRunning((CMD_FUNC)ptSym->sym, argc, argv);
            }
            break;
        default :
            SYM_LOG("Undefined symbol type");
            break;
    }

    return ret;
}



/*
 * ===========================================================================
 * 函数名称: sys_fun_test
 * 功能描述: 函数执行测试
 * 输入参数: arg1/arg2/arg3/arg4
 * 输出参数: 无
 * 返 回 值：0
 * 其它说明：
 * 修改日期        版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2019/10/11  V1.0    罗乔发         创建
 * ===========================================================================
 */
static int sys_fun_test(int arg1, int arg2, char *arg3, char *arg4)
{
    sys_printf("[%s]: arg1=%d,arg2=%d,arg3=%s,arg4=%s", __func__,
            arg1,arg2,arg3 == NULL?"null":arg3,
            arg4 == NULL?"null":arg4);
    return 0;
}

/*
 * ===========================================================================
 * 函数名称: clear
 * 功能描述: 清除屏幕
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值：无
 * 其它说明：
 * 修改日期        版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2019/10/11  V1.0    罗乔发         创建
 * ===========================================================================
 */
static int clear(void)
{
    char clear_chars[] = {0x1B, 0x5B, 0x48, 0x1B, 0x5B, 0x4A, 0x00};
    sys_printf("%s", clear_chars);
    return 0;
}

/*
 * ===========================================================================
 * 函数名称: symLookup
 * 功能描述: 查找符号表
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值：无
 * 其它说明：
 * 修改日期        版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2019/10/11  V1.0    罗乔发         创建
 * ===========================================================================
 */
static int symLookup(char *name)
{
    int lp = 0;
    int ret = -2;
    const TsymPara *pTbl;

#ifdef SYMBOL_TBL_HASH
    TList *head;
    TSymHashData *pos;
#endif
#ifdef SYMBOL_TBL_HASH
    if (NULL == name) {
        return -1;
    }
    for (lp = 0; lp < SYMBOL_HASH_HAED_SZ; lp++) {
        head = &s_sym_hash_head[lp];
        if (list_empty(head)) {
            continue;
        }
        list_for_each_entry(pos, head, list) {
            pTbl = pos->psym;
            if (NULL == pTbl) {
                continue;
            }
            if (NULL != strstr(pTbl->name, name)) {
                ret = 0;
                sys_printf("%s", pTbl->name);
            }
        }
    }
    return ret;
#else
    struct t_libSymTbl *ptlibSym = &g_sysSymLib[0];


    if (NULL == name) {
        return -1;
    }
    while (NULL != ptlibSym) {
        sym_dbg("name=%p, numSym=%d", name, ptlibSym->numSym);
        for (lp = 0; lp < ptlibSym->numSym; lp++) {
            pTbl = ptlibSym->SymTblBase + lp;
            if (NULL != strstr(pTbl->name, name)) {
                ret = 0;
                sys_printf("%s", pTbl->name);
            }
        }
        ptlibSym = ptlibSym->ptNext;
    }

    return ret;
#endif
}


/*
 * ===========================================================================
 * 函数名称: get_order
 * 功能描述: 获取指数
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值：无
 * 其它说明：
 * 修改日期        版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2019/10/11  V1.0    罗乔发         创建
 * ===========================================================================
 */
static int get_order(unsigned long size)
{
    int order;

    size = (size - 1);
    order = -1;
    do {
        size >>= 1;
        order++;
    } while (size);
    return order;
}

/*
 * ===========================================================================
 * 函数名称: symKeyGen
 * 功能描述: 计算字符串索引
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值：无
 * 其它说明：
 * 修改日期        版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2019/10/11  V1.0    罗乔发         创建
 * ===========================================================================
 */
static int symKeyGen(void)
{
    int lp = 0;
    TsymPara *pTbl;
    size_t key;
    struct t_libSymTbl *ptlibSym = &g_sysSymLib[0];
    int order = 0;

    while (NULL != ptlibSym) {
        /* 鑻ョ?﹀彿琛ㄦ寜椤哄簭鎺掑簭锛屽垯浣跨敤鎶樺崐鏌ユ壘绠楁?? */

        for (lp = 0; lp < ptlibSym->numSym; lp++) {
            pTbl = ptlibSym->SymTblBase + lp;
            order = get_order(ptlibSym->numSym);
            /* key = symHFuncName(1 << 8, pTbl, SYM_HFUNC_SEED);
            */
            key = BKDRHash(pTbl->name);

            SYM_LOG("0x%08x  %s", key, pTbl->name);
        }
        ptlibSym = ptlibSym->ptNext;
    }
    order = order;
    return 0;
}


/*
 * ===========================================================================
 * 函数名称: md
 * 功能描述: memory display
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值：无
 * 其它说明：
 * 修改日期        版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2019/10/11  V1.0    罗乔发         创建
 * ===========================================================================
 */
static int md(void *addr, int len, int data_type)
{
    int lp = 0;
    uint8_t  *mem_8 = (uint8_t *)addr;
    uint16_t *mem_16 = (uint16_t *)addr;
    uint32_t *mem_32 = (uint32_t *)addr;

    if ((0 == addr) && (0 == len) && (0 == data_type)) {
        sys_printf("Useage: md <mem_addr> <len> <0|1|2>");
    }
    if (0 == addr) {
        //SYM_LOG("Null pointer!!!");
        return -1;
    }

    len = (len? len:1);

    switch (data_type) {
        case SYM_TYPE_I8:
            sys_printf("%p: ", mem_8);
            for (lp = 0; lp < len; lp++) {
                sys_printf("%02x ", *mem_8);
                mem_8++;
                if (0 == ((lp + 1) % 16)) {
                    sys_printf("");
                    sys_printf("%p: ", mem_8);
                }
            }

            break;
        case SYM_TYPE_I16:
            sys_printf("%p: ", mem_16);
            for (lp = 0; lp < len; lp++) {
                sys_printf("%04x ", *mem_16);
                mem_16++;
                if (0 == ((lp + 1) % 16)) {
                    sys_printf("");
                    sys_printf("%p: ", mem_16);
                }
            }
            break;
        case SYM_TYPE_I32:
            sys_printf("%p: ", mem_32);
            for (lp = 0; lp < len; lp++) {
                sys_printf("%08x ", *mem_32);
                mem_32++;
                if ((lp % 16) == 15) {
                    sys_printf("");
                    sys_printf("%p: ", mem_32);
                }
            }
            break;
        default :
            break;
    }
    sys_printf("");
    return 0;
}


/*
 * ===========================================================================
 * 函数名称: mm
 * 功能描述: memory modify
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值：无
 * 其它说明：
 * 修改日期        版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2019/10/11  V1.0    罗乔发         创建
 * ===========================================================================
 */
static int mm(void *addr, int value, int data_type)
{
    uint8_t  *mem_8 = (uint8_t *)addr;
    uint16_t *mem_16 = (uint16_t *)addr;
    uint32_t *mem_32 = (uint32_t *)addr;

    if ((0 == addr) && (0 == value) && (0 == data_type)) {
        sys_printf("Useage: mm <mem_addr> <value> <0|1|2>");
    }
    if (0 == addr) {
        //SYM_LOG("Null pointer!!!");
        return -1;
    }

    switch (data_type) {
        case SYM_TYPE_I8:
            *mem_8 = value;
            break;
        case SYM_TYPE_I16:
            *mem_16 = value;
            break;
        case SYM_TYPE_I32:
            *mem_32 = value;
            break;
        default :
            break;
    }
    return 0;
}


TsymPara *prefixSymMatched(char *name, int *num_item)
{
    int lp = 0;
    int cnt = 0;
    int ln_ch_sum = 0;
    size_t len1 = strlen(name);
    size_t len2;
    TsymPara *pTbl = NULL;
#ifdef SYMBOL_TBL_HASH
    TList *head;
    TSymHashData *pos;

    for (lp = 0; lp < SYMBOL_HASH_HAED_SZ; lp++) {
        head = &s_sym_hash_head[lp];
        if (list_empty(head)) {
            continue;
        }
        cnt = 0;
        ln_ch_sum = 0;
        list_for_each_entry(pos, head, list) {
            pTbl = pos->psym;
            len2 = strlen(pTbl->name);
            if (len1 > len2) {
                continue;
            }
            if (0 == strncmp(pTbl->name, name, len1)) {
                cnt++;
                sys_printf("%s ", pTbl->name);
                ln_ch_sum += len2 + 1;
                if (ln_ch_sum >= 79) {
                    SYM_LOG("", pTbl->name);
                    ln_ch_sum = 0;
                }
            }
        }
        if (NULL == num_item) {
            *num_item = cnt;
        }
    }
    return NULL;
#else
    int ret = 0;
    size_t len;
    int idx = 0;
    TsymPara *pTbl_base = NULL;
    struct t_libSymTbl *ptlibSym = &g_sysSymLib[0];

#if DBG_SYM
    SYM_LOG("");
#endif
    sym_dbg("Enter, prefix: %s, len=%d", name, len1);

    while (ptlibSym) {
        if (NULL != ptlibSym->SymTblBase) {
            idx = 0;
            while (idx < (ptlibSym->numSym)) {
                pTbl = ptlibSym->SymTblBase + idx;
                idx++;
                len2 = strlen(pTbl->name);
                if (len1 > len2) {
                    continue;
                }
                len = len1;
                sym_dbg("symbol name: %s", pTbl->name);
                ret = strncmp(name, pTbl->name, len);
                if (0 == ret) {
                    cnt++;
                    ln_ch_sum += len2 + 1;
                    sym_dbg("%s", pTbl->name);
                    if (cnt > 1) {
                        sys_printf("%s ", pTbl->name);
                        if (ln_ch_sum >= 79) {
                            SYM_LOG("");
                            ln_ch_sum = 0;
                        }
                    }
                    if (NULL == pTbl_base) {
                        pTbl_base = pTbl;
                    }
                    for (lp = idx; lp < ptlibSym->numSym; lp++) {
                        pTbl = ptlibSym->SymTblBase + lp;
                        ret = strncmp(name, pTbl->name, len);
                        if (0 == ret) {
                            if (1 == cnt) {
                                SYM_LOG("%s ", pTbl_base->name);
                            }
                            cnt++;
                            ln_ch_sum += strlen(pTbl->name) + 1;
                            sym_dbg("%s", pTbl->name);
                            sys_printf("%s ", pTbl->name);
                            if (ln_ch_sum >= 79) {
                                SYM_LOG("");
                                ln_ch_sum = 0;
                            }
                        } else {
                            break;
                        }
                    } /* end of for */
                    break;
                }
            } /* while (idx < (ptlibSym->numSym - 1))  */
        }
        ptlibSym = ptlibSym->ptNext;
    }

    *num_item = cnt;
#if 0
    if (cnt > 1) {
        SYM_LOG("");
    }
#endif
    sym_dbg("num_item=%d, exit...", *num_item);
    return pTbl_base;
#endif
}

static int prefix_test(char *prefix)
{
    int num_item = 0;
    int lp = 0;

    TsymPara *pTbl = NULL;
    TsymPara *pTbl_base = NULL;

    pTbl_base = prefixSymMatched(prefix, &num_item);
    if (NULL == pTbl_base) {
        /* SYM_LOG("Don't find match prefix:%s", prefix); */
        return -1;
    }
    SYM_LOG("num_item=%d", num_item);
    for (lp = 0; lp < num_item; lp++) {
        pTbl = pTbl_base + lp;
        sys_printf("%s ", pTbl->name);
    }
    SYM_LOG("");
    return num_item;
}


int shellOutputFdSet(int fd)
{
    s_output_fd = fd;
    return 0;
}

void symsDumpToFile(FILE *fp)
{
    const TsymPara *pTbl = NULL;
#ifdef SYMBOL_TBL_HASH
    int lp = 0;
    TList *head;
    TSymHashData *pos;

    if (NULL == fp) {
        fp = stdout;
    }
    for (lp = 0; lp < SYMBOL_HASH_HAED_SZ; lp++) {
        head = &s_sym_hash_head[lp];
        if (list_empty(head)) {
            continue;
        }
        list_for_each_entry(pos, head, list) {
            if (NULL != pos) {
                pTbl = pos->psym;
                if (NULL != pTbl) {
                    fprintf(fp, "%s\n", pTbl->name);
                }
            }
        }
        fflush(fp);
    }
#else
    // int ret = 0;
    // size_t len;
    int idx = 0;
    // TsymPara *pTbl_base = NULL;
    struct t_libSymTbl *ptlibSym = &g_sysSymLib[0];

    if (NULL == fp) {
        fp = stdout;
    }
    while (ptlibSym) {
        if (NULL != ptlibSym->SymTblBase) {
            for (idx = 0; idx < ptlibSym->numSym; idx++) {
                pTbl = ptlibSym->SymTblBase + idx;
                fprintf(fp, "%s\n", pTbl->name);
            } /* while (idx < (ptlibSym->numSym - 1))  */
            fflush(fp);
        }
        ptlibSym = ptlibSym->ptNext;
    }
#endif
}


static int bin(int val)
{
    uint32_t n = val;
    char s[65+2] = {'0','b', '0', '\0'};
    char a[64];
    int i = 0, j = 3;

    if (val != 0) {
        j = 2;
    }
    while (n > 0) {
        a[i] = (char)(n & 0x01);
        i++;
        n >>= 1;
    }
    while (i > 0) {
        s[j] = a[i - 1] + '0';
        i--;
        j++;
    }
    s[j] = '\0';
    sys_printf("%s", s);
    return val;
}

static int hex(int val)
{
    uint32_t n = val;
    char s[64/4+3] = {'0','x', '0', '\0'};
    char a[64/4];
    int i = 0, j = 3;

    if (val != 0) {
        j = 2;
    }
    while (n > 0) {
        a[i] = (char)(n & 0xf);
        i++;
        n >>= 4;
    }
    while (i > 0) {
        s[j] = a[i - 1] + '0';
        if (a[i - 1] >= 0xa) {
            s[j] = a[i - 1] - 0x0a + 'a';
        }
        i--;
        j++;
    }
    s[j] = '\0';
    sys_printf("%s", s);
    return val;
}

static int oct(int val)
{
    uint32_t n = val;
    char s[64/3+3] = {'0','o', '0', '\0'};
    char a[64/3+1];
    int i = 0, j = 3;

    if (val != 0) {
        j = 2;
    }
    while (n > 0) {
        a[i] = (char)(n & 0x7);
        i++;
        n /= 8;
    }
    while (i > 0) {
        s[j] = a[i - 1] + '0';
        i--;
        j++;
    }
    s[j] = '\0';
    sys_printf("%s", s);
    return val;
}

static int sys_fun_repeat(int repeat_times, void *funptr, void *arg1, void *arg2, void *arg3, void *arg4)
{
    int rc;

    int (*fun)(void *arg1, void *arg2, void *arg3, void *arg4) = funptr;

    if (NULL== fun) {
        return -1;
    }
    if (repeat_times < 0) {
        while (true) {
            rc = fun(arg1, arg2, arg3, arg4);
            if (rc < 0) break;
        }
        return 0;
    }
    while (repeat_times > 0) {
        rc = fun(arg1, arg2, arg3, arg4);
        if (rc < 0) break;
        repeat_times--;
    }
}
