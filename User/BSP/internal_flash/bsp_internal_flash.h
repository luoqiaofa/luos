#ifndef __INTERNAL_FLASH_H
#define	__INTERNAL_FLASH_H

#include "stm32f10x.h"

/* STM32大容量产品每页大小2KByte，中、小容量产品每页大小1KByte */
#if defined (STM32F10X_HD) || defined (STM32F10X_HD_VL) || defined (STM32F10X_CL) || defined (STM32F10X_XL)
  #define FLASH_PAGE_SIZE    (0x800)	//2048
#else
  #define FLASH_PAGE_SIZE    (0x400)	//1024
#endif

//写入的起始地址与结束地址
#define FLASH_ADDR_BEGIN        0x8000000
#define FLASH_ADDR_END          0x8040000
#define WRITE_NUM_SIZE     (4 * 1024)
#if (WRITE_NUM_SIZE % FLASH_PAGE_SIZE) != 0
#error "WRITE_NUM_SIZE is not aligned with page size"
#endif
#define WRITE_START_ADDR  ((uint32_t)(FLASH_ADDR_END - WRITE_NUM_SIZE))
#define WRITE_END_ADDR    ((uint32_t)FLASH_ADDR_END)



typedef enum
{
	FAILED = 0,
  PASSED = !FAILED
} TestStatus;


int InternalFlash_Test(void);
int InternalFlash_prog(uint32_t offset, uint32_t *Data, uint32_t num);
int BspDacValueSave(void);
int BspDacValueLoad(void);



#endif /* __INTERNAL_FLASH_H */

