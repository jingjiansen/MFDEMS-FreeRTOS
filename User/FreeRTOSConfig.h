#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "stm32f10x.h"
#include "usart/usart_com.h"
#include <stdio.h>

/* 
    针对不同的编译器调用不同的stdint.h文件
    在MDK中，默认使用 __CC_ARM 编译器
 */
#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)
#include <stdint.h> /* 包含标准整数类型定义 */
extern uint32_t SystemCoreClock; /* 系统时钟频率，寻找外部定义，STM32F103C8T6为72MHz */
#endif

/* 断言 */
#define vAssertCalled(char, int) printf("Error:%s, %d\r\n", char, int)
#define configASSERT(x) if((x) == 0) vAssertCalled(__FILE__, __LINE__)

/*************************************************************************/
/*************************FreeRTOS基础配置选项****************************/
/*************************************************************************/

/* 置1：RTOS使用抢占式调度器；置0：RTOS使用协作式调度器（时间片） */
/* 在多任务管理机制上，操作系统可分为抢占式和协作式两种，协作式
   操作系统是任务主动释放CPU后，切换到下一个任务，任务切换的时机
   完全取决于正在运行的任务
*/
#define configUSE_PREEMPTION		1

/* 1：使能时间片调度（默认是使能的） */
#define configUSE_TIME_SLICEING		1

/* 某些运行FreeRTOS的硬件有两种方法选择下一个要执行的任务：
   通用方法和特定与硬件的方法（以下简称特殊方法）
   
   通用方法：软件方法扫描就绪链表的方法
            1：configUSE_PORT_OPTIMISED_TASK_SELECTION 为 0 或者硬件不支持这种特殊方法
            2：可以用于所有FreeRTOS支持的硬件
            3：完全用C实现，效率略低于特殊方法
            4：不强制要求限制最大可用优先级数目
    
    特殊方法：
            1：必须将configUSE_PORT_OPTIMISED_TASK_SELECTION 设为 1
            2：依赖一个或多个特定架构的汇编指令（一般是类似计算前导零[CLZ]指令）
            3：比通用方法更高效
            4：一般强制限定最大可用优先级数目为32
    一般是硬件计算前导零指令，如果所使用的MCU没有这些硬件指令的话此宏应该设置为0
*/
#define configUSE_PORT_OPTIMISED_TASK_SELECTION	1

/* 
    1：使能低功耗tickless模式；
    2：保持系统节拍（tick）中断一直运行
*/
#define configUSE_TICKLESS_IDLE		1

/*
    写入实际的CPU内核时钟频率，也就是CPU指令执行频率，通常称为Fclk
    Fclk为供给CPU内核的时钟信号，主频即该时钟信号，相应1/Fclk即为时钟周期
*/
#define configCPU_CLOCK_HZ			( SystemCoreClock )

/* 
    RTOS 系统节拍中断的频率，即一秒中断的次数，每次中断RTOS都会进行任务调度
    在STM32裸机程序经常使用的SysTick时钟是MCU的内核定时器，通常都使用该定时
    器产生操作系统的时钟节拍。在FreeRTOS中，系统延时和阻塞时间都是以tick为单
    位，配置该选项的值就可以改变中断的频率，从而间接改变FreeRTOS的时钟周期（T=1/f）。
    设置为1000即FreeRTOS的时钟周期为1ms，过高的系统节拍中断频率也意味着FreeRTOS
    内核占用更多的CPU时间，因此会降低效率，通常为100-1000即可。
*/
#define configTICK_RATE_HZ			( ( TickType_t ) 1000 )

/* 
    可使用的最大优先级
    每个任务都有一个优先级，优先级范围为0到(configMAX_PRIORITIES-1)，
    数值越大优先级越高。空闲任务的优先级为0（tskIDLE_PRIORITY）。FreeRTOS
    调度器将确保处于就绪态的高优先级任务比同样处于就绪态的低优先级任务先执行。
 */
#define configMAX_PRIORITIES		( 32 )

/* 
    空闲任务使用的堆栈大小，默认为128字
    堆栈大小不是以字节为单位而是以字为单位，如在32位架构下，栈大小为100表示栈内存使用400字节空间
*/
#define configMINIMAL_STACK_SIZE	( ( unsigned short ) 128 )

/* 任务名字字符串长度 */
#define configMAX_TASK_NAME_LEN		( 16 )

/* 系统节拍计数器变量数据类型：
    1：16位无符号整型
    0：32位无符号整型
    STM32是32位架构，该值决定了tick的数目，32位可计算2^32个tick，约1193个小时
    溢出之后就会充值
*/
#define configUSE_16_BIT_TICKS		0

/* 
    空闲任务放弃CPU使用权给其他同优先级任务
    仅在满足下列条件后才会起作用：启用了抢占式调度，用户优先级与空闲任务优先级相同，一般不建议使用该功能
 */
#define configIDLE_SHOULD_YIELD		1

/* 启用消息队列，这是FreeRTOS提供的一种用于任务间通信（IPC）的机制 */
#define configUSE_QUEUE_SETS		1

/* 
    开启任务通知功能，默认开启
    每个FreeRTOS任务具有一个32位的通知值，FreeRTOS任务通知是直接向任务发送一个事件，并且
    接收任务的通知值是可以选择的，任务通过接收到的任务通知值来解除任务的阻塞状态（假如因
    等待该任务通知而进入阻塞状态）。相对于队列、二进制信号量、计数信号量或事件组等PIC通信，
    使用任务通知显然更灵活，官方说明：相比于使用信号量，使用任务通知可以快45%（使用GCC编译器，
    -o2优化级别），并且使用更少的RAM。
 */
#define configUSE_TASK_NOTIFICATIONS	1

/* 使用互斥信号量 */
#define configUSE_MUTEXES			1

/* 使用递归互斥信号量 */
#define configUSE_RECURSIVE_MUTEXES		1

/* 1：使用计数信号量 */
#define configUSE_COUNTING_SEMAPHORES	1

/* 设置可以注册的信号量和消息队列个数 */
#define configQUEUE_REGISTRY_SIZE		10
#define configUSE_APPLICATION_TASK_TAG	0


/*************************************************************************/
/*************************FreeRTOS内存配置选项****************************/
/*************************************************************************/

/* 支持动态内存申请 */
#define configSUPPORT_DYNAMIC_ALLOCATION	1

/* 支持静态内存申请 */
#define configSUPPORT_STATIC_ALLOCATION		0

/* 
   系统所有总的堆大小，即FreeRTOS内核总可用内存大小，不能超过芯片的RAM大小，一般来说
   用户可用的内存大小会小于configTOTAL_HEAP_SIZE定义的大小，因为系统本身就需要内存。
   每当创建任务、队列、互斥量、软件定时器或信号量时，FreeRTOS内核会为这些内核对象分配RAM，
   这里的RAM都属于configTOTAL_HEAP_SIZE指定的内存区。
*/
#define configTOTAL_HEAP_SIZE		( ( size_t ) ( 16 * 1024 ) )

/*************************************************************************/
/*************************FreeRTOS钩子函数选项****************************/
/*************************************************************************/
/*
    1：使用空闲钩子（Idle Hook类似于回调函数）；
    0：忽略空闲钩子
    
    空闲任务钩子是一个函数，这个函数由用户实现
    FreeRTOS规定了函数的名称和参数：void vApplicationIdleHook(void)
    这个函数在每个空闲任务周期都会被调用
    对于已经删除的RTOS任务，空闲任务可以释放分配给它们的堆栈内存
    因此必须保证空闲任务可以被CPU执行
    使用空闲钩子函数设置CPU进入低功耗模式是很常见的
    不可以调用会引起空闲任务阻塞的API函数
    */
#define configUSE_IDLE_HOOK			0

/*
    1：使用时间片钩子（Tick Hook）；
    0：忽略时间片钩子

    时间片钩子是一个函数，这个函数由用户实现，
    FreeRTOS规定了函数的名字和参数：void vApplicationTickHook(void)
    时间片中断可以周期性的调用
    函数必须非常短小，不能大量使用堆栈
    不能调用以'FromISR'或'FROM_ISR'结尾的API函数
*/
#define configUSE_TICK_HOOK			0

/* 使用内存申请失败钩子函数 */
#define configUSE_MALLOC_FAILED_HOOK		0

/* 
    大于0时启用堆栈溢出检测功能，如果使用此功能，用户必须提供一个栈溢出
    钩子函数，如果使用的话，此值可为1或2，因为有两种栈溢出检测方法
*/
#define configCHECK_FOR_STACK_OVERFLOW		0


/*************************************************************************/
/*************************FreeRTOS运行时间和任务状态收集选项***************/
/*************************************************************************/

/* 
    1：启用运行时间统计功能；
    0：忽略运行时间统计功能 
*/
#define configGENERATE_RUN_TIME_STATS		0

/* 
    1：启用可视化跟踪调试
    0：忽略可视化跟踪调试
*/
#define configUSE_TRACE_FACILITY		0
/*
    宏configUSE_TRACE_FACILITY为1时会编译下面3个函数
    prvWriteNameToBuffer()
    vTaskList()
    vTaskGetRunTimeStats()
*/
#define configUSE_STATS_FORMATTING_FUNCTIONS	1


/*************************************************************************/
/*************************FreeRTOS与协程有关的配置项**********************/
/*************************************************************************/

/* 
    1：启用协程功能，启用后必须添加文件coroutine.c
    0：忽略协程功能
*/
#define configUSE_CO_ROUTINES		0

/* 协程的有效优先级数目 */
#define configMAX_CO_ROUTINE_PRIORITIES		( 2 )


/*************************************************************************/
/*************************FreeRTOS与软件定时器有关的配置项*****************/
/*************************************************************************/

/* 1：启用软件定时器；0：忽略软件定时器 */
#define configUSE_TIMERS			1

/* 软件定时器优先级为最高优先级 */
#define configTIMER_TASK_PRIORITY		( configMAX_PRIORITIES - 1 )

/* 
    软件定时器队列长度，也就是允许配置多少个软件定时器的数量，
    理论上软件定时器的数量可无限，因为其不依赖于硬件 
*/
#define configTIMER_QUEUE_LENGTH		( 10 )

/* 软件定时器任务栈大小 */
#define configTIMER_TASK_STACK_DEPTH		( configMINIMAL_STACK_SIZE * 2 )


/*************************************************************************/
/*************************FreeRTOS可选函数配置项**************************/
/*************************************************************************/

#define INCLUDE_xTaskGetSchedulerState		1
#define INCLUDE_vTaskPrioritySet				1
#define INCLUDE_uxTaskPriorityGet		1
#define INCLUDE_vTaskDelete			1
#define INCLUDE_vTaskCleanUpResources		1
#define INCLUDE_vTaskSuspend			1
/* 该函数用于中断中恢复任务与vTaskSuspend搭配使用，通常不建议在中断中恢复任务 */
// #define INCLUDE_xTaskResumeFromISR		1 
#define INCLUDE_vTaskDelayUntil			1
#define INCLUDE_vTaskDelay			1
#define INCLUDE_eTaskGetState			1
#define INCLUDE_xTimerPendFunctionCall		1


/*************************************************************************/
/*************************FreeRTOS中断相关配置项**************************/
/*************************************************************************/

#ifdef __NVIC_PRIO_BITS
#define configPRIO_BITS				__NVIC_PRIO_BITS
#else
/* 定义__NVIC_PRIO_BITS表示配置FreeRTOS使用多少位作为中断优先级，在STM32中使用4位 */
#define configPRIO_BITS				4
#endif

/* 
    中断最低优先级，中断优先级的数值越小，优先级越高，这里是中断优先级，
    而非任务优先级，任务优先级恰好相反。
 */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY		15

/* 
    系统可管理的最高中断优先级为5
    该宏定义用于配置basepri寄存器，当寄存器值设置为某个值时，会让系统不响应
    比该优先级低的中断，而优先级比该优先级高的中断会被响应，也就是说，当设置
    为5时候，0，1，2，3，4这5个中断优先级是不受FreeRTOS管理的，不可被屏蔽，
    也不能调用FreeRTOS中的API函数接口，而中断优先级在5-15的这些中断时受到
    系统管理，可以被屏蔽的。
 */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY		5

/* 
    对需要配置的SysTick和PendSV进行偏移（因为高4位才有效）
    在port.c中会用到configKERNEL_INTERRUPT_PRIORITY这个宏定义来配置SCB_SHPR3
    （系统处理优先级寄存器，地址为：0xE000ED20）
*/
#define configKERNEL_INTERRUPT_PRIORITY		( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/*
    该宏定义用于配置basepri寄存器，让FreeRTOS屏蔽优先级数值大于这个宏定义的中断
    （数值越大，中断优先级越低），而basepri的有效位为高4位，因此需要进行偏移，因为STM32
    只使用了优先级寄存器中的4位，所以要以最高有效位对齐。
*/
#define configMAX_SYSCALL_INTERRUPT_PRIORITY		( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )


/*************************************************************************/
/*************************FreeRTOS中断服务函数相关配置项*******************/
/*************************************************************************/

#define xPortPendSVHandler			PendSV_Handler /* 系统调度产生PendSV异常，在PendSV_Handler中实现任务切换 */
#define vPortSVCHandler			    SVC_Handler 

/* 以下为使用 Percepio Tracealyzer 时的配置项，不需要时将 configUSE_TRACER_FACILITY 设为 0 */
#if (configUSE_TRACE_FACILITY == 1)
#include "trcRecorder.h"
#define INCLUDE_xTaskGetCurrentTaskHandle		1
/* 启用一个可选函数，该函数被Trace源码使用，默认该值为0，表示不用 */
#endif

#endif /* FREERTOS_CONFIG_H */
