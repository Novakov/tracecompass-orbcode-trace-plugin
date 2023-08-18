#pragma once
#include <stdint.h>

extern uint32_t SystemCoreClock;
#define configCPU_CLOCK_HZ SystemCoreClock

#define configTICK_RATE_HZ 100

#define configUSE_PREEMPTION 1
#define configMAX_PRIORITIES 5
#define configMINIMAL_STACK_SIZE 130

#define configUSE_16_BIT_TICKS 0

#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0

#define configUSE_TRACE_FACILITY 1

#define configMAX_SYSCALL_INTERRUPT_PRIORITY (4 << 5)

#define configSUPPORT_STATIC_ALLOCATION 1
#define configSUPPORT_DYNAMIC_ALLOCATION 0

#define INCLUDE_vTaskDelay 1

#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler
#define vPortSVCHandler SVC_Handler

#define configASSERT(x)           \
    if((x) == 0)                  \
    {                             \
        taskDISABLE_INTERRUPTS(); \
        for(;;)                   \
            ;                     \
    }

#if __has_include("rtos_tune.h")
#    include "rtos_tune.h"
#endif

#include "TraceHooks.h"