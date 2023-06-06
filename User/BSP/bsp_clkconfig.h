#ifndef __CLKCONFIG_H
#define	__CLKCONFIG_H

#include "stm32f10x.h"

int HSE_SetSysClock(uint32_t pllmul);
int HSI_SetSysClock(uint32_t pllmul);

#endif /* __CLKCONFIG_H */
