向 Linux, vxWorks, µC/OS 致敬与学习
本RTOS 特性:
1.  参考µC/OS-III, 支持基于优先级调度算法,查找最高优先级任务的算法时间复杂度为常数
2.  支持同优先级下多个任务, 同优先级下的任务默认采用时间片轮转算法RR, 也可配置为
    先来先服务FIFO度算法, FIFO只用于特殊场合, 如相同优先级时,多个任务顺序执行
3.  RTOS函数调用参考vxWorks 的库函数命名, 尽可能使用相同的函数名及参数
4.  目前成stm32f103ze MCU上验证OK, DEMO板为野火STM32F103霸道_V1
![image](https://github.com/luoqiaofa/luos/assets/11310157/39af0ec2-21f6-49de-87ff-7ca27ae1c790)
