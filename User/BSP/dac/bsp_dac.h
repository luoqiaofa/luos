#ifndef __DAC_H
#define	__DAC_H


#include "stm32f10x.h"
#define DAC_CHAN_NUM 2

//DAC DHR12RD�Ĵ�����12λ���Ҷ��롢˫ͨ��
#define DAC_DHR12RD_ADDRESS      (DAC_BASE+0x20)


void DAC_Mode_Init(void);
void BspDacRawVolt(uint16_t dac_chan, uint16_t milli_volt);


#endif /* __DAC_H */

