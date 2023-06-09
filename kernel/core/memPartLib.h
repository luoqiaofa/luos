/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : memPartLib.h
 * 内容摘要   :
 * 其它说明   :
 * 版本       :
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-05-23 09:43:34 AM
 *
 * 修改记录1:
 *    修改日期: 2023-05-23
 *    版 本 号:
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容:
 * ===========================================================================
 */

#ifndef __MEMPARTLIB_H__
#define __MEMPARTLIB_H__

#include "osTypes.h"

#ifndef CONFIG_MEM_HEAP_SIZE
#define CONFIG_MEM_HEAP_SIZE    (5 * 1024) /* 10KB size */
#endif
#define MIN_BLK_SIZE            64
#define MIN_ALLOC_BLK_SZ        16

typedef struct mem_blk {
    TLIST   list;
    size_t  numBlock;
    size_t  totalSize;
} MEM_BLK; /* sizeof(MEM_BLK) must be power of 2, ie. 4,8,16,32, 64, ...  */

typedef struct mem_part {
    TLIST   freeListHdr;
    size_t  freeListSize;
    void*   memBase;
    void*   freeBase;
    size_t  freeSize;
    size_t  totalSize;
    size_t  blkMinSize;
} MEM_PART;
typedef MEM_PART * PART_ID;

typedef struct mem_object {
    TLIST    memList;
    PART_ID  partId;
    void *   allocated;
    void *   freeListPtr;
    void *   owner; /* who created it */
    uint16_t numBlkFree;
    uint16_t blockSize;
    uint16_t refCnt;
    uint16_t options;
} MEM_OBJ;

STATUS memPartLibInit(void);
STATUS memPartInit(PART_ID partId, char *mem_pool, size_t size, size_t blksize);
/*  memory partition to allocate from
 *  number of bytes to allocate
 */
void *memPartAlloc(PART_ID partId, size_t nBytes);
STATUS memPartFree(PART_ID partId, void* pBlk);
void *malloc(size_t nBytes);             /*  number of bytes to allocate */
void free(void *ptr);             /*  number of bytes to allocate */

#endif /* #ifndef __MEMPARTLIB_H__ */


