向 Linux, vxWorks, µC/OS 致敬与学习
本RTOS 特性:
1.  参考µC/OS-III, 支持基于优先级调度算法,查找最高优先级任务的算法时间复杂度为常数, 即任务数不受限制,<p>
    且任务调度时间复杂度任务数多少无关.<p>
2.  支持同优先级下多个任务, 同优先级下的任务默认采用时间片轮转调度算法RR, 也可配置为<p>
    先来先服务FIFO调度算法, FIFO只用于特殊场合, 如相同优先级时,多个任务顺序执行
3.  RTOS函数调用参考vxWorks 的库函数命名, 尽可能使用相同的函数名及参数
4.  信号量(记数型,二进制,互斥) semGive,semTake 支持基于FIFO及优先级的队列阻塞(被挂起高优先级优先获取信号).
    另外为防止死锁, 互斥型信号量实现优先级反转, 即若低优先级先获得锁, 高优先级申请导致挂起时, 会提升低优先级的任务优先级<p>
    释放该信号量后会恢复该到原来优先级
5.  目前成stm32f103ze MCU上验证OK, DEMO板为野火STM32F103霸道_V1<p>
6.  支持类似vxWorks中函数及即命令的shell命令行功能, 不过远没vxWorks shehll 命令行解释器那般强大, 努力接近...<p>
    启动后打印如下, <p>
    ![image](https://github.com/luoqiaofa/luos/assets/11310157/ecc42d89-da48-4a74-9425-0d819e8fa175)
    <p>
        
    命令格式使用帮助<p>
        
     ![image](https://github.com/luoqiaofa/luos/assets/11310157/e4faeace-e90d-4a9e-a4b9-8baa8a8252c9)
    <p>命令参数若为字符串, 字符串必须使用双引号""包括起来<p>
    若命令参数为符号表中的符号, 如函数, 或全局变量名, 则不需要双引号包括, shell 解释器支持类似C语法的解析
    函数命令分两种形式, 如 fun(1,2, "abc", "def") 或 fun 1 2 "abc" "def"<p>
    一种为类似C语言的函数调用或变量赋值格式, 一种为通过空格分隔 <p>
      
     ![image](https://github.com/luoqiaofa/luos/assets/11310157/3506b717-e5b3-431a-8eea-a0f06c4cd27d)
      <p>

      ![image](https://github.com/luoqiaofa/luos/assets/11310157/7d1e3c88-02a2-49f4-802b-ebbed429510b)
      <p>

      查找命令示例如下<p>

      ![image](https://github.com/luoqiaofa/luos/assets/11310157/5eb567a5-6c89-4404-a066-2ed808b90373)
    
      命令(符号表)查找命令示例如下:<p>
      ![image](https://github.com/luoqiaofa/luos/assets/11310157/d885b1e9-f1a1-4f71-9fae-1e48e6d0dd1e)

    任务基本状态信息打印如下:<p>
    ![image](https://github.com/luoqiaofa/luos/assets/11310157/a46ce29a-5271-44c5-84d5-d88c8d1032e5)




