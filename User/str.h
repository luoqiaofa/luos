/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : str.h
 * 内容摘要   : 
 * 其它说明   : 
 * 版本       : 
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-02-03 02:41:26 PM
 * 
 * 修改记录1:
 *    修改日期: 2023-02-03
 *    版 本 号: 
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容: 
 * ===========================================================================
 */

#ifndef _STR_H_
#define _STR_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

/* ctype.h
 * int isalnum(int c);
 * int isalpha(int c);
 * int isascii(int c);
 * int isblank(int c);
 * int iscntrl(int c);
 * int isdigit(int c);
 * int isgraph(int c);
 * int islower(int c);
 * int isprint(int c);
 * int ispunct(int c);
 * int isspace(int c);
 * int isupper(int c);
 * int isxdigit(int c);
 * int toascii(int);
 * int toupper(int c);
 * int tolower(int c);
 */

int strCharDelete(char *s, int c);
int strSpaceDel(char *s);
bool isValidNumber(const char *s);
int strToInt(const char *str);
int64_t strToInt64(const char *str);
char * strReverse(char *s);
char * strToUpper(char *s);
char * strToLower(char *s);
char * strStrip(char *s, int c, int *plen);
char * strLeftStrip(char *s, int c, int *plen);
char * strRightStrip(char *s, int c, int *plen);
int strSplit(char *s, int sep, char *split[], int max_split);
char * strJoin(int nstr, char *strs[], int c, int *plen);
int strCount(const char *s, const char *sub);
char * strTitle(char *s);
char * strHexToBin(char *hex, int hexlen, char *bin, int *binlen);
char *strValidGet(char *line, int comment_char, int *plen);
char *strnDup(const char *str, int len);

/*
 * ===========================================================================
 * 函数名称: strSkipSpace
 * 功能描述: 略过行首空白字符
 * 输入参数: s 字符串 
 * 输出参数: 无 
 * 返 回 值: 略过行首空白字符后的字符串
 * 其它说明: 无
 * 修改日期  版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2023-02-03  V1.0       罗乔发         创建
 * ===========================================================================
 */
static inline char * strSpaceSkip(char *s)
{
    while (isspace(*s)) {s++;}
    return s;
}

/*
 * ===========================================================================
 * 函数名称: strCharSkip
 * 功能描述: 略过行首指定字符
 * 输入参数: s 字符串
 *           c 指定字符, 若 c<=0 则略过空白字符，否则为指定字符 
 * 输出参数: 无 
 * 返 回 值: 略过行首指定字符后的字符串指针
 * 其它说明: 无
 * 修改日期  版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2023-02-03  V1.0       罗乔发         创建
 * ===========================================================================
 */
static inline char * strCharSkip(char *s, int c)
{
    if (c <= 0) return strSpaceSkip(s);
    while (*s && (c == *s)) {s++;}
    return s;
}

static inline bool strIsUpper(const char *s)
{
    while (*s) {
        if (islower(*s)) return false;
        if (!isupper(*s)) return false;
        s++;
    }
    return true;
}

static inline bool strIsLower(const char *s)
{
    while (*s) {
        if (isupper(*s)) return false;
        if (!islower(*s)) return false;
        s++;
    }
    return true;
}

static inline bool isNumberPre(char ch)
{
    if (('+' == ch) || ('-' == ch) || isdigit(ch)) {
        return true;
    }
    return false;
}


static inline bool isbinary(char c)
{
    if (c == '0' || c == '1') {
        return true;
    }
    return false;
}

static inline bool isodigit(char c)
{
    if (c >= '0' && c <= '7') {
        return true;
    }
    return false;
}

static inline bool isbinarys(const char *s)
{
    if (*s != '0' && *s != '1') {
        return false;
    }
    s++;
    while (*s) {
        if (isspace(*s)) {
            break;
        }
        if (!isbinary(*s)) {
            return false;
        }
        s++;
    }
    return true;
}

static inline bool isxdigits(const char *s)
{
    if (!isxdigit(*s)) {
        return false;
    }
    s++;
    while (*s) {
        if (isspace(*s)) {
            break;
        }
        if (!isxdigit(*s)) {
            return false;
        }
        s++;
    }
    return true;
}

static inline bool isodigits(const char *s)
{
    if (!(*s >= '0' && *s <= '7')) {
        return false;
    }
    s++;
    while (*s) {
        if (isspace(*s)) {
            break;
        }
        if (!isodigit(*s)) {
            return false;
        }
        s++;
    }
    return true;
}

static inline bool isdigits(const char *s)
{
    if (!isdigit(*s)) {
        return false;
    }
    s++;
    while (*s) {
        if (isspace(*s)) {
            break;
        }
        if (!isdigit(*s)) {
            return false;
        }
        s++;
    }
    return true;
}

static inline const char *bool_tostring(bool bl)
{
    static const char *bools[2] = {"false", "true"};
    return bools[!!bl];
}

#endif /* #ifndef _STRING_H_ */

