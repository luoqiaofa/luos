/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : str.c
 * 内容摘要   :
 * 其它说明   :
 * 版本       :
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-02-03 02:39:48 PM
 *
 * 修改记录1:
 *    修改日期: 2023-02-03
 *    版 本 号:
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容:
 * ===========================================================================
 */

/******************************************************************************
 *                #include (依次为标准库头文件、非标准库头文件)               *
 ******************************************************************************/
#include "str.h"
#ifndef STR_NUM_SPLIT
#define STR_NUM_SPLIT 128
#endif
/*
 * ===========================================================================
 * 函数名称: strDelChar
 * 功能描述: 删除指定字符
 * 输入参数: s 字符串
 * 输出参数: 无
 * 返 回 值: 删除指定字符后的字符串长度
 * 其它说明: 无
 * 修改日期  版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2023-02-03  V1.0       罗乔发         创建
 * ===========================================================================
 */
int strCharDelete(char *s, int c)
{
    int  i, j=0;

    if (0 == c) {
        return strSpaceDel(s);
    }
    for (i = 0; s[i] != '\0'; i++) {
        if (s[i] != c) {
            s[j++]=s[i];
        }
    }
    s[j]='\0';
    return j;
}

/*
 * ===========================================================================
 * 函数名称: strSpaceDel
 * 功能描述: 删除指定字符
 * 输入参数: s 字符串
 * 输出参数: 无
 * 返 回 值: 删除指定字符后的字符串长度
 * 其它说明: 无
 * 修改日期  版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2023-02-03  V1.0       罗乔发         创建
 * ===========================================================================
 */
int strSpaceDel(char *s)
{
    int  i, j=0;

    for (i = 0; s[i] != '\0'; i++) {
        if (!isspace(s[i])) {
            s[j++]=s[i];
        }
    }
    s[j]='\0';
    return j;
}

bool isValidNumber(const char *s)
{
    char ch;

    if (!isNumberPre(*s)) {
        return false;
    }
    if (('+' == *s) || ('-' == *s)) {
        s++;
        if (!*s) {return false;}
    }
    if ('0' == *s) {
        s++;
        if ('\0' == *s) {
            return true;
        }
        ch = tolower(*s);
        if ('x' == ch || 'o' == ch || 'b' == ch || 'd' == ch) {
            if (!s[1]) {return false;}
            s++;
            if ('x' == ch) {
                return isxdigits(s);
            } else if ('o' == ch) {
                return isodigits(s);
            } else if ('b' == ch) {
                return isbinarys(s);
            } else if ('d' == ch) {
                return isdigits(s);
            } else {
                return false;
            }
        } else {
            /* default as decimal */
            return isdigits(s);
        }
    }
    /* default as decimal */
    return isdigits(s);
}


/*
 * ===========================================================================
 * 函数名称： strToInt
 * 功能描述： 字符串转换为整数,支持十/十六(0x)/八(0o)/二(0b)进制
 * 输入参数： pStr   指向要转换的字符串
 * 输出参数： 无
 * 返 回 值： 相应整数
 * 其它说明： 无
 * 修改日期        版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2011年08月24日  V1.0       罗乔发         创建
 * ===========================================================================
 */
int strToInt(const char *s)
{
	int value = 0;
	int sign = 1;
	int radix = 10;
    char ch;

	if (NULL == s) {
		return 0;
	}

	if ('-' == *s) {
		sign = -1;
		s++;
	} else if ('+' == *s) {
		s++;
    } else {
    }

    if ('0' == *s) {
        ch = tolower(s[1]);
        if (isdigit(ch)) {
            radix = 10;
        } else if ('x' == ch) {
            radix = 16;
            s += 2;
        } else if ('o' == ch) {
            radix = 8;
            s += 2;
        } else if ('b' == ch) {
            radix = 2;
            s += 2;
        } else if ('d' == ch) {
            radix = 10;
            s += 2;
        } else {
            return 0;
        }
    }

    if (10 == radix) {
        /* (10 == radix) */
        while (isdigit(*s)) {
            value = value * radix + (*s - '0');
            s++;
        }
    } else if (16 == radix) {
        while (isxdigit(*s)) {
            ch = tolower(*s);
            if (isdigit(*s)) {
                value = value * radix + (ch - '0');
            } else {
                value = value * radix + (ch - 'a' + 10);
            }
            s++;
        }
    } else if (2 == radix) {
        while (isbinary(*s)) {
            value = value * radix + (*s - '0');
            s++;
        }
    } else if (8 == radix) {
        while (isodigit(*s)) {
            value = value * radix + (*s - '0');
            s++;
        }
    } else {
        return 0;
    }
	value *= sign;
	return value;
}        /* -----  end of function sToInt  ----- */

/*
 * ===========================================================================
 * 函数名称： strToInt64
 * 功能描述： 字符串转换为64bit整数,支持十/十六(0x)/八(0o)/二(0b)进制
 * 输入参数： pStr   指向要转换的字符串
 * 输出参数： 无
 * 返 回 值： 相应整数
 * 其它说明： 无
 * 修改日期        版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2011年08月24日  V1.0       罗乔发         创建
 * ===========================================================================
 */
int64_t strToInt64(const char *s)
{
	int64_t value = 0;
	int64_t sign = 1;
	int64_t radix = 10;
    char ch;

	if (NULL == s) {
		return 0;
	}

	if ('-' == *s) {
		sign = -1;
		s++;
	} else if ('+' == *s) {
		s++;
    } else {
    }

    if ('0' == *s) {
        ch = tolower(s[1]);
        if (isdigit(ch)) {
            radix = 10;
        } else if ('x' == ch) {
            radix = 16;
            s += 2;
        } else if ('o' == ch) {
            radix = 8;
            s += 2;
        } else if ('b' == ch) {
            radix = 2;
            s += 2;
        } else if ('d' == ch) {
            radix = 10;
            s += 2;
        } else {
            return 0;
        }
    }

    if (10 == radix) {
        /* (10 == radix) */
        while (isdigit(*s)) {
            value = value * radix + (*s - '0');
            s++;
        }
    } else if (16 == radix) {
        while (isxdigit(*s)) {
            ch = tolower(*s);
            if (isdigit(*s)) {
                value = value * radix + (ch - '0');
            } else {
                value = value * radix + (ch - 'a' + 10);
            }
            s++;
        }
    } else if (2 == radix) {
        while (isbinary(*s)) {
            value = value * radix + (*s - '0');
            s++;
        }
    } else if (8 == radix) {
        while (isodigit(*s)) {
            value = value * radix + (*s - '0');
            s++;
        }
    } else {
        return 0;
    }
	value *= sign;
	return value;
} /* -----  end of function strToInt64  ----- */

/*
 * ===========================================================================
 * 函数名称： strToUpper
 * 功能描述： 把字符串中所有小写字母转换为大写字母
 * 输入参数： s 指向要转换的字符串
 * 输出参数： 无
 * 返 回 值： 转换后字符串指针
 * 其它说明： 无
 * 修改日期        版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2011年08月24日  V1.0       罗乔发         创建
 * ===========================================================================
 */
char * strToUpper(char *s) {
    char *str = s;

    while (*s) {
        if (islower(*s)) {
            *s -= ('a' - 'A');
        }
        s++;
    }
    return str;
}


/*
 * ===========================================================================
 * 函数名称： strToLower
 * 功能描述： 把字符串中所有大写字母转换为小写字母
 * 输入参数： s 指向要转换的字符串
 * 输出参数： 无
 * 返 回 值： 转换后字符串指针
 * 其它说明： 无
 * 修改日期        版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2011年08月24日  V1.0       罗乔发         创建
 * ===========================================================================
 */
char * strToLower(char *s) {
    char *str = s;

    while (*s) {
        if (isupper(*s)) {
            *s += ('a' - 'A');
        }
        s++;
    }
    return str;
}

/*
 * ===========================================================================
 * 函数名称: strStrip
 * 功能描述: 删除行首及行尾指定字符
 * 输入参数: s  字符串指针
 *           c  需要删除的字符, 若 c<=0, 视为删除空白字符, 否则则为指定字符
 * 输出参数: plen 删除行首及行尾指定字符后新字符串的长度
 * 返 回 值: 新字符串指针
 * 其它说明: 无
 * 修改日期    版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2023-02-03  V1.0       罗乔发         创建
 * ===========================================================================
 */
char * strStrip(char *s, int c, int *plen)
{
    int  i, j = 0, len;
    bool is_space_sep = false;
    char *p;

    if (NULL == s || '\0' == *s) {
        return NULL;
    }
    if (0 == c) {
        is_space_sep = true;
    }
    j = strlen(s);
    len = j;
    if (is_space_sep) {
        while (j > 0) {
            j--;
            if (!isspace(s[j])) {
                break;
            }
            s[j] = '\0';
            len = j;
        }
    } else {
        while (j > 0) {
            j--;
            if (c != s[j]) {
                break;
            }
            s[j] = '\0';
            len = j;
        }
    }
    if (is_space_sep) {
        p = strSpaceSkip(s);
    } else {
        p = strCharSkip(s, c);
    }
    j = 0;
    i = p - s;
    if (i > 0) {
        while (s[i]) s[j++] = s[i++];
        s[j] = '\0';
        len = j;
    }

    if (NULL != plen) {
        *plen = len;
    }
    return s;
}        /* -----  end of function strStrip  ----- */

/*
 * ===========================================================================
 * 函数名称: strLeftStrip
 * 功能描述: 删除行首指定字符
 * 输入参数: s  字符串指针
 *           c  需要删除的字符, 若 c<=0, 视为删除空白字符, 否则则为指定字符
 * 输出参数: plen 删除行首指定字符后新字符串的长度
 * 返 回 值: 新字符串指针
 * 其它说明: 无
 * 修改日期    版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2023-02-03  V1.0       罗乔发         创建
 * ===========================================================================
 */
char * strLeftStrip(char *s, int c, int *plen)
{
    int  i, j;
    bool is_space_sep = false;
    char *p;

    if (NULL == s || '\0' == *s) {
        return NULL;
    }
    if (c <= 0) {
        is_space_sep = true;
    }
    if (is_space_sep) {
        p = strSpaceSkip(s);
    } else {
        p = strCharSkip(s, c);
    }
    i = p - s;
    if (i > 0) {
        j = 0;
        while (s[i]) s[j++] = s[i++];
        s[j] = '\0';
        if (NULL != plen) {
            *plen = j;
        }
    } else {
        if (NULL != plen) {
            *plen = strlen(s);
        }
    }
    return s;
}        /* -----  end of function strLeftStrip  ----- */

/*
 * ===========================================================================
 * 函数名称: strRightStrip
 * 功能描述: 删除行尾指定字符
 * 输入参数: s  字符串指针
 *           c  需要删除的字符, 若 c<=0, 视为删除空白字符, 否则则为指定字符
 * 输出参数: plen 删除行尾指定字符后新字符串的长度
 * 返 回 值: 新字符串指针
 * 其它说明: 无
 * 修改日期    版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2023-02-03  V1.0       罗乔发         创建
 * ===========================================================================
 */
char * strRightStrip(char *s, int c, int *plen)
{
    int  i;
    bool is_space_sep = false;
    int len;

    if (NULL == s || '\0' == *s) {
        return NULL;
    }
    if (0 == c) {
        is_space_sep = true;
    }
    i = strlen(s);
    len = i;
    if (is_space_sep) {
        while (i > 0 && isspace(s[i - 1])) {
            i--;
            s[i] = '\0';
            len = i;
        }
    } else {
        while (i > 0 && (c == s[i - 1])) {
            i--;
            s[i] = '\0';
            len = i;
        }
    }
    if (NULL != plen) {
        *plen = len;
    }
    return s;
}        /* -----  end of function strRightStrip  ----- */

/*
 * ===========================================================================
 * 函数名称: strSplit
 * 功能描述: 根据指定分隔字符把该字符串分割成一个或多个字符串
 * 输入参数: s   需要分割的字符串
 *           sep 指定分隔字符, 若 sep <= 0 则与空白字符为分隔符，否则为指定字符
 *           max_split 最大可能分割数, <= 0, 则最大不超过 STR_NUM_SPLIT 所定义
 * 输出参数: split 存储分割后的字符串指针
 * 返 回 值: 分割成的字符串个数
 * 其它说明: 无
 * 修改日期    版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2023-02-03  V1.0       罗乔发         创建
 * ===========================================================================
 */
int strSplit(char *s, int sep, char *split[], int max_split)
{
    int nseg = 0;
    bool is_space_sep = false;
    char *p, *p1, *p2;

    if (max_split <= 0) {
        max_split = STR_NUM_SPLIT;
    }

    if (sep <= 0) {
        is_space_sep = true;
    }
    p = strCharSkip(s, sep);
    if (is_space_sep) {
        p = strSpaceSkip(s);
    }
    p1 = p;
    while (*p) {
        if (is_space_sep) {
            if (!isspace(*p)) {
                p++;
                continue;
            }
        } else {
            if (*p != sep) {
                p++;
                continue;
            }
        }
        p2 = p;
        *p2 = '\0';
        split[nseg++] = p1;
        if (nseg >= max_split) {
            return nseg;
        }
        p++;
        if (is_space_sep) {
            p = strSpaceSkip(p);
        } else {
            p = strCharSkip(p, sep);
        }
        p1 = p;
    }
    if (*p1) {
        p2 = p;
        *p2 = '\0';
        split[nseg++] = p1;
    }
    return nseg;
}        /* -----  end of function strSplit  ----- */

/*
 * ===========================================================================
 * 函数名称: strJoin
 * 功能描述: 根据指定拼接字符把一个或多个字符串拼接成一个新的字符串
 * 输入参数: nstr 需要拼接的字符串个数
 *           strs 需要拼接的字符串指针数组
 *           c    指定的拼接字符,若 sep<=0 则空格为拼接字符，否则为指定字符
 * 输出参数: plen 拼接后的新字符串长度
 * 返 回 值: 拼接后的新字符串指针
 * 其它说明: 拼接后的新字符串指针由malloc申请, 使用完成需要free
 * 修改日期    版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2023-02-03  V1.0       罗乔发         创建
 * ===========================================================================
 */
char * strJoin(int nstr, char *strs[], int c, int *plen)
{
    int sum = 0, seq_len[nstr], idx;
    char *buf, ch;

    if (nstr <= 0 || NULL == strs) {
        return NULL;
    }
    ch = c;
    if (0 == c) {
        ch = ' ';
    }
    for (sum = nstr, idx = 0; idx < nstr; idx++) {
        seq_len[idx] = strlen(strs[idx]);
        sum += seq_len[idx];
    }
    buf = (char *)malloc(sum + 1);
    if (NULL == buf) {
        return NULL;
    }
    for (sum = 0, idx = 0; idx < nstr; idx++) {
        memcpy(buf + sum, strs[idx], seq_len[idx]);
        sum += seq_len[idx];
        buf[sum] = ch;
        sum++;
    }
    sum--;
    buf[sum] = '\0';
    if (NULL != plen) {
        *plen = sum;
    }
    return buf;
}

/*
 * ===========================================================================
 * 函数名称: strCount
 * 功能描述: 统计指定字符串中包含多少个字串
 * 输入参数: s   字符串指针
 *           sub 子字符串指针
 * 输出参数: 无
 * 返 回 值: 指定字符串中包含多少个字串数
 * 其它说明: 无
 * 修改日期    版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2023-02-03  V1.0       罗乔发         创建
 * ===========================================================================
 */
int strCount(const char *s, const char *sub)
{
    int n = 0, la, lb;
    const char *p1 = s, *p2;

    if (NULL == s || NULL == sub) {
        return 0;
    }
    la = strlen(s);
    lb = strlen(sub);
    if (la < 1 || lb < 1 || la < lb) {
        return 0;
    }
    p2 = strstr(p1, sub);
    while (NULL != p2) {
        n++;
        p1 = p2 + lb;
        if (p1 >= (s+la)) {
            break;
        }
        p2 = strstr(p1, sub);
    }
    return n;
}

/*
 * ===========================================================================
 * 函数名称: strTitle
 * 功能描述: 字符串中每个单词的首字母大写
 * 输入参数: s 字符串指针
 * 输出参数: 无
 * 返 回 值: 字符串指针s
 * 其它说明: 无
 * 修改日期    版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2023-02-03  V1.0       罗乔发         创建
 * ===========================================================================
 */
char * strTitle(char *s)
{
    char *p1, *p2;

    p1 = strSpaceSkip(s);
    while (*p1) {
        *p1 = toupper(*p1);
        p1++;
        p2 = p1;
        while (*p2) {
            if (isspace(*p2)) {
                break;
            }
            p2++;
        }
        if (!*p2) {
            break;
        }
        p1 = strSpaceSkip(p2);
    }
    return s;
}

/*
 * ===========================================================================
 * 函数名称: strHexToBin
 * 功能描述: 把由16进制字符[0-9a-fA-F]串转换为二进制数据
 * 输入参数: hex 16进制字符串指针
 * 输出参数: bin 保存转换为二进制的数据指针
 *           binlen 转换为二进制后的数据长度
 * 返 回 值: 保存转换为二进制的数据指针 bin
 * 其它说明: 若 bin 传入为空,则会malloc申请内存,并返回,调用者需要free释放
 * 修改日期    版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2023-02-03  V1.0       罗乔发         创建
 * ===========================================================================
 */
char * strHexToBin(char *hex, int hexlen, char *bin, int *binlen)
{
    int i, j;
    bool pad;

    if (NULL == hex) {
        return NULL;
    }
    if (hexlen <= 0) {
        hexlen = strlen(hex);
    }
    if (hexlen < 1) {
        return NULL;
    }
    pad = (bool)(hexlen % 2);
    if (NULL == bin) {
        bin = (char *)malloc(hexlen / 2 + 1);
    }
    if (NULL == bin) {
        return NULL;
    }
    for (i = 0, j = 0; i <= hexlen - 2; i += 2, j++) {
        if (!isxdigit(hex[i])) {
            pad = false;
            break;
        }
        if (isdigit(hex[i])) {
            bin[j] = hex[i] - '0';
        } else {
            bin[j] = hex[i] - 'a' + 10;
        }
        bin[j] *= 16;

        if (!isxdigit(hex[i+1])) {
            pad = false;
            break;
        }
        if (isdigit(hex[i + 1])) {
            bin[j] += hex[i + 1] - '0';
        } else {
            bin[j] += hex[i + 1] - 'a' + 10;
        }
    }
    if (pad) {
        if (isdigit(hex[i])) {
            bin[j] = hex[i] - '0';
        } else {
            bin[j] = hex[i] - 'a' + 10;
        }
        j++;
    }
    if (NULL != binlen) {
        *binlen = j;
    }
    return bin;
}

/*
 * ===========================================================================
 * 函数名称: strReverse
 * 功能描述: 反转字符串(字符串逆序)
 * 输入参数: s 字符串指针
 * 输出参数: 无
 * 返 回 值: 字符串指针, 即 s 本身
 * 其它说明: 无
 * 修改日期    版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2023-02-03  V1.0       罗乔发         创建
 * ===========================================================================
 */
char * strReverse(char *s)
{
    int l, r;
    char t;

    r = strlen(s) - 1;
    for (l = 0; l < r; l++, r--) {
        t = s[l];
        s[l] = s[r];
        s[r] = t;
    }
    return s;
}        /* -----  end of function strReverse  ----- */

/*
 * ===========================================================================
 * 函数名称: strValidGet
 * 功能描述: 获取带有注释的有效字符串
 * 输入参数: line 原始字符串
 *           comment_char 行注释字符
 * 输出参数: plen 有效字符串长度
 * 返 回 值: 有效字符串指针,若有效长度为0, 长返回NULL
 * 其它说明: 无
 * 修改日期    版本号     修改人         修改内容
 * ----------------------------------------------------------------------------
 * 2023-02-03  V1.0       罗乔发         创建
 * ===========================================================================
 */
char *strValidGet(char *line, int comment_char, int *plen)
{
    char *p1, *p2;
    int len;

    if (NULL == line) {
        return NULL;
    }
    if (NULL != plen) {
        *plen = 0;
    }
    p1 = strSpaceSkip(line);
    /* white space include \t, \n \r, space, backspace */
    if (comment_char == *p1 || '\0' == *p1) {
        /* comment line */
        return NULL;
    }
    p2 = p1;
    p2++;
    while ('\0' != *p2) {
        /* check line end or not */
        if (comment_char == *p2 || '\n' == *p2 || '\r' == *p2) {
            /* comment line */
            *p2 = '\0';
            break;
        }
        p2++;
    }
    p2--;
    while (isspace(*p2)) {
        *p2 = '\0';
        p2--;
    }
    len = p2 - p1 + 1;
    if (NULL != plen) {
        *plen = len;
    }
    return p1;
}

char *strnDup(const char *str, int len)
{
    char *p;

    if (NULL == str) {
        return NULL;
    }
    if (len <= 0) {
        len = strlen(str);
    }
    p = malloc(len + 1);
    if (NULL == p) {
        return p;
    }
    memcpy(p, str, len);
    p[len] = '\0';
    return p;
}


