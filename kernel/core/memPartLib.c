/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : memPartLib.c
 * 内容摘要   :
 * 其它说明   :
 * 版本       :
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-05-23 02:51:10 PM
 *
 * 修改记录1:
 *    修改日期: 2023-05-23
 *    版 本 号:
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容:
 * ===========================================================================
 */

/******************************************************************************
 *                #include (依次为标准库头文件、非标准库头文件)               *
 ******************************************************************************/
#include "memPartLib.h"
#include "coreLib.h"

static BOOL memPartLibInstalled = false;
static MEM_PART sysMemAllocHeap;
static PART_ID sysMemPartId = &sysMemAllocHeap;
static MEM_BLK sysMemPool[CONFIG_MEM_HEAP_SIZE / sizeof(MEM_BLK)];

STATUS memPartLibInit()
{
    int rc;
    char *mem_pool = (char *)&sysMemPool[0];
    size_t size = CONFIG_MEM_HEAP_SIZE;
    size_t blksize = MIN_ALLOC_BLK_SZ;

    sysMemPartId = &sysMemAllocHeap;
    rc = memPartInit(sysMemPartId, mem_pool, size, blksize);
    if (0 == rc) {
        memPartLibInstalled = true;
    }
    return rc;
}

STATUS memPartInit(PART_ID partId, char *mem_pool, size_t size, size_t blksize)
{
    unsigned long int uintptr;
    if (NULL == partId || NULL == mem_pool) {
        return -1;
    }
    if (0 == size || 0 == blksize) {
        return -2;
    }
    memset(partId, 0, sizeof(*partId));
    INIT_LIST_HEAD(&partId->freeListHdr);
    partId->totalSize  = size;
    partId->memBase    = mem_pool;
    partId->blkMinSize = blksize;
    partId->freeBase   = mem_pool;
    uintptr = (unsigned long int)mem_pool;
    uintptr += sizeof(MEM_BLK) - 1;
    uintptr &= (~(sizeof(MEM_BLK) - 1));
    partId->freeBase   = (void *)uintptr;
    partId->freeSize   = size - (size_t)(partId->freeBase - partId->memBase);
    return 0;
}

void *memPartAlloc(PART_ID partId, size_t nBytes)
{
    size_t sz;
    TLIST *node;
    MEM_BLK *blk = NULL;
    void *ptr = NULL;

    if (NULL == partId || 0 == nBytes) {
        return NULL;
    }
    if (partId->freeListSize >= nBytes) {
        list_for_each(node, &partId->freeListHdr) {
            blk = list_entry(node, MEM_BLK, list);
            if (blk->totalSize >= nBytes) {
                ptr = (void *)(blk + 1);
                break;
            }
        }
        if (NULL != ptr) {
            list_del(node);
            INIT_LIST_HEAD(node);
            return ptr;
        }
    }

    sz = (nBytes + sizeof(MEM_BLK) + partId->blkMinSize - 1) & (~(partId->blkMinSize - 1));
    if (partId->freeSize < sz) {
        return NULL;
    }
    blk = (MEM_BLK *)partId->freeBase;
    INIT_LIST_HEAD(&blk->list);
    blk->totalSize = sz - sizeof(MEM_BLK);
    blk->numBlock = blk->totalSize / partId->blkMinSize;
    partId->freeSize -= sz;
    ptr = (void *)(blk + 1);
    partId->freeBase = partId->freeBase + sz;
    return ptr;
}

STATUS memPartFree(PART_ID partId, void* pBlk)
{
    void *ptr;
    void *ptr2;
    TLIST *node;
    MEM_BLK *blk;
    MEM_BLK *blk2;
    bool  need_merge = false;

    if (NULL == partId || NULL == pBlk) {
        return -1;
    }
    blk = (MEM_BLK *)pBlk;
    blk--;

    if (!list_empty_careful(&blk->list)) {
        return -2;
    }
    need_merge = false;
    list_for_each(node, &partId->freeListHdr) {
        ptr = (void *)blk;
        blk2 = list_entry(node, MEM_BLK, list);
        ptr2 = (void *)blk2;
        if (ptr < ptr2) {
            ptr += sizeof(MEM_BLK) + blk->totalSize;
            if (ptr == ptr2) {
                need_merge = true;
                break;
            }
        } else {
            ptr2 += sizeof(MEM_BLK) + blk2->totalSize;
            if (ptr == ptr2) {
                need_merge = true;
                break;
            }
        }
    }
    if (need_merge) {
        list_del(node);
        if (blk < blk2) {
            blk->numBlock  += blk2->numBlock;
            blk->totalSize += blk2->totalSize + sizeof(MEM_BLK);
            ptr = (void *)blk;
            node = &blk->list;
        } else {
            blk2->numBlock  += blk->numBlock;
            blk2->totalSize += blk->totalSize + sizeof(MEM_BLK);
            ptr = (void *)blk2;
        }
        blk = (void *)ptr;
        ptr2 = ptr + blk->totalSize + sizeof(MEM_BLK);
        if (ptr2 == partId->freeBase) {
            partId->freeBase = ptr;
            partId->freeSize += sizeof(MEM_BLK) + blk->totalSize;
            partId->freeListSize -= blk2->totalSize;
        } else {
            list_add_tail(node, &partId->freeListHdr);
            partId->freeListSize += blk->totalSize;
        }
    } else {
        ptr = (void *)blk;
        ptr2 = ptr + sizeof(MEM_BLK) + blk->totalSize;
        if (ptr2 == partId->freeBase) {
            partId->freeBase = ptr;
            partId->freeSize += sizeof(MEM_BLK) + blk->totalSize;
        } else {
            partId->freeListSize += blk->totalSize;
            list_add(&blk->list, &partId->freeListHdr);
        }
    }
    return 0;
}

void *memObjMalloc(PART_ID partId, size_t nBytes)
{
    void *p1;
    /* void *p2; */
    /* void **pp; */
    MEM_OBJ *pmem;
    MEM_BLK  *blk;
    LUOS_TCB *tcb = currentTask();

    pmem = (MEM_OBJ *)memPartAlloc(partId, nBytes + sizeof(MEM_OBJ));
    if (NULL == pmem) {
        return NULL;
    }
    blk = (MEM_BLK *)pmem;
    blk--;
    p1 = (void *)(pmem + 1);
    pmem->partId = partId;
    pmem->freeListPtr = p1;
    pmem->allocated   = p1;
    pmem->blockSize   = MIN_ALLOC_BLK_SZ;
    pmem->numBlkFree  = blk->totalSize/pmem->blockSize;
    INIT_LIST_HEAD(&pmem->memList);
    list_add(&pmem->memList, &tcb->memListHdr);
#if 0
    if (nBytes > pmem->blockSize) {
        pmem->freeListPtr = NULL;
        pmem->numBlkFree  = 0;
        return p1;
    }
    idx = 0;
    pp = p1;
    while (idx < pmem->numBlkFree) {
        p2 = p1 + ((idx + 1) * pmem->blockSize);
        *pp = (void **)p2;
        pp = p2;
        idx++;
    }
#endif
    return p1;
}

STATUS memObjFree(PART_ID partId, void *ptr)
{
    MEM_OBJ *pmem;
    /* LUOS_TCB *tcb = currentTask(); */
    if (NULL == partId || NULL == ptr) {
        return -1;
    }
    pmem = (MEM_OBJ *)ptr;
    pmem--;
    list_del(&pmem->memList);
    INIT_LIST_HEAD(&pmem->memList);
    return memPartFree(pmem->partId, (void *)pmem);
}


void *malloc(size_t nbytes)
{
    if (memPartLibInstalled) {
        return memPartAlloc(sysMemPartId, nbytes);
    }
    return NULL;
}

void free(void *ptr)
{
    if (memPartLibInstalled) {
        memPartFree(sysMemPartId, ptr);
    }
}

