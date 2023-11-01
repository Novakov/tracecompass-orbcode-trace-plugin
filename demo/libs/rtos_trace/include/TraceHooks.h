#pragma once
#include <stdbool.h>

#if !defined(configUSE_TRACE_FACILITY) || (configUSE_TRACE_FACILITY == 0)
#    error "Enable FreeRTOS trace facilities using #define configUSE_TRACE_FACILITY 1"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    extern void TraceOnTaskSwitchedIn(void* tcb);
    extern void TraceOnTaskSwitchedOut(void* tcb, bool still_ready);
    extern void TraceOnTaskCreated(void* tcb, const char* name);
    extern void TraceOnTaskReadied(void* tcb);

    extern void TraceOnBinarySemaphoreCreate(void* queue);
    extern void TraceOnBinarySemaphoreLock(void* queue, bool isr, bool success);
    extern void TraceOnBinarySemaphoreUnlock(void* queue, bool isr, bool success);

    extern void TraceOnCountingSemaphoreCreate(void* semaphore, uint32_t maxCount, uint32_t initialCount);
    extern void TraceOnCountingSemaphoreTake(void* semaphore, bool isr, bool success, uint32_t newCount);
    extern void TraceOnCountingSemaphoreGive(void* semaphore, bool isr, bool success, uint32_t newCount);

    extern void TraceOnMutexCreate(void* mutex);
    extern void TraceOnMutexLock(void* mutex, bool isr, bool success);
    extern void TraceOnMutexUnlock(void* mutex, bool isr, bool success);

    extern void TraceOnQueueCreate(void* queue, uint32_t capacity);
    extern void TraceOnQueuePush(void* queue, bool isr, bool success, uint32_t updatedItemsCount);
    extern void TraceOnQueuePop(void* queue, bool isr, bool success, uint32_t updatedItemsCount);

    extern void OnTaskNotify(void* task, uint32_t index, bool isr, uint32_t action, uint32_t updatedValue);
    extern void OnTaskNotifyReceived(void* task, uint32_t index, uint32_t updatedValue);

#ifdef __cplusplus
}
#endif

__attribute__((unused)) static int xCopyPosition = 0;

enum SwitchReason
{
    SWITCH_REASON_TICK = 0,
    SWITCH_REASON_DELAYED = 1,
    SWITCH_REASON_BLOCKED_MUTEX = 2,
    SWITCH_REASON_BLOCKED_QUEUE_PUSH = 3,
    SWITCH_REASON_BLOCKED_QUEUE_POP = 4,
    SWITCH_REASON_BLOCKED_BINARY_SEMAPHORE = 5,
    SWITCH_REASON_BLOCKED_EVENT_GROUP = 6,
    SWITCH_REASON_COUNTING_SEMAPHORE_GIVE = 7,
    SWITCH_REASON_COUNTING_SEMAPHORE_RECEIVE = 8,
    SWITCH_REASON_TASK_NOTIFY_WAIT = 9,
    SWITCH_REASON_BLOCKED_OTHER = 0xF,
};

struct SwitchRecord
{
    enum SwitchReason Reason;
    void* BlockedOnObject;
};

extern struct SwitchRecord CurrentTaskSwitchRecord;

#define traceTASK_SWITCHED_IN()                          \
    CurrentTaskSwitchRecord.Reason = SWITCH_REASON_TICK; \
    CurrentTaskSwitchRecord.BlockedOnObject = NULL;      \
    TraceOnTaskSwitchedIn(pxCurrentTCB)
#define traceTASK_SWITCHED_OUT()                                                                      \
    TraceOnTaskSwitchedOut(                                                                           \
        pxCurrentTCB,                                                                                 \
        (pxCurrentTCB->xStateListItem.pxContainer == &pxReadyTasksLists[pxCurrentTCB->uxPriority]) || \
            (pxCurrentTCB->xStateListItem.pxContainer == NULL))

#define traceTASK_CREATE(task) TraceOnTaskCreated(task, task->pcTaskName)

#define traceQUEUE_CREATE(queue)                        \
    taskENTER_CRITICAL();                               \
    if(ucQueueType == queueQUEUE_TYPE_BINARY_SEMAPHORE) \
    {                                                   \
        TraceOnBinarySemaphoreCreate(queue);            \
    }                                                   \
    else if(ucQueueType == queueQUEUE_TYPE_BASE)        \
    {                                                   \
        TraceOnQueueCreate(queue, queue->uxLength);     \
    }                                                   \
    taskEXIT_CRITICAL();

#define traceCREATE_COUNTING_SEMAPHORE()                                 \
    taskENTER_CRITICAL();                                                \
    TraceOnCountingSemaphoreCreate(xHandle, uxMaxCount, uxInitialCount); \
    taskEXIT_CRITICAL();

#define traceQUEUE_RECEIVE(queue)                                                                                     \
    if(queue->ucQueueType == queueQUEUE_TYPE_BINARY_SEMAPHORE)                                                        \
    {                                                                                                                 \
        TraceOnBinarySemaphoreLock(queue, false, true);                                                               \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_COUNTING_SEMAPHORE)                                                 \
    {                                                                                                                 \
        TraceOnCountingSemaphoreTake(queue, false, true, queue->uxMessagesWaiting - 1);                               \
    }                                                                                                                 \
    else if((queue->ucQueueType == queueQUEUE_TYPE_MUTEX) || (queue->ucQueueType == queueQUEUE_TYPE_RECURSIVE_MUTEX)) \
    {                                                                                                                 \
        TraceOnMutexLock(queue, false, true);                                                                         \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_BASE)                                                               \
    {                                                                                                                 \
        TraceOnQueuePop(queue, false, true, queue->uxMessagesWaiting - 1);                                            \
    }

#define traceQUEUE_RECEIVE_FAILED(queue)                                                                              \
    taskENTER_CRITICAL();                                                                                             \
    if(queue->ucQueueType == queueQUEUE_TYPE_BINARY_SEMAPHORE)                                                        \
    {                                                                                                                 \
        TraceOnBinarySemaphoreLock(queue, false, false);                                                              \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_COUNTING_SEMAPHORE)                                                 \
    {                                                                                                                 \
        TraceOnCountingSemaphoreTake(queue, false, false, queue->uxMessagesWaiting);                                  \
    }                                                                                                                 \
    else if((queue->ucQueueType == queueQUEUE_TYPE_MUTEX) || (queue->ucQueueType == queueQUEUE_TYPE_RECURSIVE_MUTEX)) \
    {                                                                                                                 \
        TraceOnMutexLock(queue, false, false);                                                                        \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_BASE)                                                               \
    {                                                                                                                 \
        TraceOnQueuePop(queue, false, false, queue->uxMessagesWaiting);                                               \
    }                                                                                                                 \
    taskEXIT_CRITICAL();

#define traceQUEUE_RECEIVE_FROM_ISR(queue)                                                                            \
    if(queue->ucQueueType == queueQUEUE_TYPE_BINARY_SEMAPHORE)                                                        \
    {                                                                                                                 \
        TraceOnBinarySemaphoreLock(queue, true, true);                                                                \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_COUNTING_SEMAPHORE)                                                 \
    {                                                                                                                 \
        TraceOnCountingSemaphoreTake(queue, true, true, queue->uxMessagesWaiting - 1);                                \
    }                                                                                                                 \
    else if((queue->ucQueueType == queueQUEUE_TYPE_MUTEX) || (queue->ucQueueType == queueQUEUE_TYPE_RECURSIVE_MUTEX)) \
    {                                                                                                                 \
        TraceOnMutexLock(queue, true, true);                                                                          \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_BASE)                                                               \
    {                                                                                                                 \
        TraceOnQueuePop(queue, true, true, queue->uxMessagesWaiting - 1);                                             \
    }

#define traceQUEUE_RECEIVE_FROM_ISR_FAILED(queue)                                                                     \
    UBaseType_t traceSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();                                            \
    if(queue->ucQueueType == queueQUEUE_TYPE_BINARY_SEMAPHORE)                                                        \
    {                                                                                                                 \
        TraceOnBinarySemaphoreLock(queue, true, false);                                                               \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_COUNTING_SEMAPHORE)                                                 \
    {                                                                                                                 \
        TraceOnCountingSemaphoreTake(queue, true, true, queue->uxMessagesWaiting);                                    \
    }                                                                                                                 \
    else if((queue->ucQueueType == queueQUEUE_TYPE_MUTEX) || (queue->ucQueueType == queueQUEUE_TYPE_RECURSIVE_MUTEX)) \
    {                                                                                                                 \
        TraceOnMutexLock(queue, true, false);                                                                         \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_BASE)                                                               \
    {                                                                                                                 \
        TraceOnQueuePop(queue, true, false, queue->uxMessagesWaiting);                                                \
    }                                                                                                                 \
    taskEXIT_CRITICAL_FROM_ISR(traceSavedInterruptStatus);

#define traceQUEUE_SEND(queue)                                                                                        \
    if(queue->ucQueueType == queueQUEUE_TYPE_BINARY_SEMAPHORE)                                                        \
    {                                                                                                                 \
        TraceOnBinarySemaphoreUnlock(queue, false, true);                                                             \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_COUNTING_SEMAPHORE)                                                 \
    {                                                                                                                 \
        TraceOnCountingSemaphoreGive(queue, false, true, queue->uxMessagesWaiting + 1);                               \
    }                                                                                                                 \
    else if((queue->ucQueueType == queueQUEUE_TYPE_MUTEX) || (queue->ucQueueType == queueQUEUE_TYPE_RECURSIVE_MUTEX)) \
    {                                                                                                                 \
        TraceOnMutexUnlock(queue, false, true);                                                                       \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_BASE)                                                               \
    {                                                                                                                 \
        if((queue->uxMessagesWaiting == queue->uxLength) && (xCopyPosition == queueOVERWRITE))                        \
        {                                                                                                             \
            TraceOnQueuePush(queue, false, true, queue->uxMessagesWaiting);                                           \
        }                                                                                                             \
        else                                                                                                          \
        {                                                                                                             \
            TraceOnQueuePush(queue, false, true, queue->uxMessagesWaiting + 1);                                       \
        }                                                                                                             \
    }

#define traceQUEUE_SEND_FAILED(queue)                                                                                 \
    taskENTER_CRITICAL();                                                                                             \
    if(queue->ucQueueType == queueQUEUE_TYPE_BINARY_SEMAPHORE)                                                        \
    {                                                                                                                 \
        TraceOnBinarySemaphoreUnlock(queue, false, false);                                                            \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_COUNTING_SEMAPHORE)                                                 \
    {                                                                                                                 \
        TraceOnCountingSemaphoreGive(queue, false, false, queue->uxMessagesWaiting);                                  \
    }                                                                                                                 \
    else if((queue->ucQueueType == queueQUEUE_TYPE_MUTEX) || (queue->ucQueueType == queueQUEUE_TYPE_RECURSIVE_MUTEX)) \
    {                                                                                                                 \
        TraceOnMutexUnlock(queue, false, false);                                                                      \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_BASE)                                                               \
    {                                                                                                                 \
        TraceOnQueuePush(queue, false, false, queue->uxMessagesWaiting);                                              \
    }                                                                                                                 \
    taskEXIT_CRITICAL();

#define traceQUEUE_SEND_FROM_ISR(queue)                                                                               \
    if(queue->ucQueueType == queueQUEUE_TYPE_BINARY_SEMAPHORE)                                                        \
    {                                                                                                                 \
        TraceOnBinarySemaphoreUnlock(queue, true, true);                                                              \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_COUNTING_SEMAPHORE)                                                 \
    {                                                                                                                 \
        TraceOnCountingSemaphoreGive(queue, true, true, queue->uxMessagesWaiting + 1);                                \
    }                                                                                                                 \
    else if((queue->ucQueueType == queueQUEUE_TYPE_MUTEX) || (queue->ucQueueType == queueQUEUE_TYPE_RECURSIVE_MUTEX)) \
    {                                                                                                                 \
        TraceOnMutexUnlock(queue, true, true);                                                                        \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_BASE)                                                               \
    {                                                                                                                 \
        if((queue->uxMessagesWaiting == queue->uxLength) && (xCopyPosition == queueOVERWRITE))                        \
        {                                                                                                             \
            TraceOnQueuePush(queue, true, true, queue->uxMessagesWaiting);                                            \
        }                                                                                                             \
        else                                                                                                          \
        {                                                                                                             \
            TraceOnQueuePush(queue, true, true, queue->uxMessagesWaiting + 1);                                        \
        }                                                                                                             \
    }

#define traceQUEUE_SEND_FROM_ISR_FAILED(queue)                                                                        \
    UBaseType_t traceSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();                                            \
    if(queue->ucQueueType == queueQUEUE_TYPE_BINARY_SEMAPHORE)                                                        \
    {                                                                                                                 \
        TraceOnBinarySemaphoreUnlock(queue, true, false);                                                             \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_COUNTING_SEMAPHORE)                                                 \
    {                                                                                                                 \
        TraceOnCountingSemaphoreGive(queue, true, false, queue->uxMessagesWaiting);                                   \
    }                                                                                                                 \
    else if((queue->ucQueueType == queueQUEUE_TYPE_MUTEX) || (queue->ucQueueType == queueQUEUE_TYPE_RECURSIVE_MUTEX)) \
    {                                                                                                                 \
        TraceOnMutexUnlock(queue, true, false);                                                                       \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_BASE)                                                               \
    {                                                                                                                 \
        TraceOnQueuePush(queue, true, false, queue->uxMessagesWaiting);                                               \
    }                                                                                                                 \
    taskEXIT_CRITICAL_FROM_ISR(traceSavedInterruptStatus);

#define traceCREATE_MUTEX(mutex) \
    taskENTER_CRITICAL();        \
    TraceOnMutexCreate(mutex);   \
    taskEXIT_CRITICAL();

#define tracePOST_MOVED_TASK_TO_READY_STATE(task) TraceOnTaskReadied(task)

#define traceTASK_DELAY() CurrentTaskSwitchRecord.Reason = SWITCH_REASON_DELAYED;

#define traceTASK_DELAY_UNTIL(xTimeToWake) CurrentTaskSwitchRecord.Reason = SWITCH_REASON_DELAYED;

#define traceBLOCKING_ON_QUEUE_SEND(queue)                                      \
    CurrentTaskSwitchRecord.BlockedOnObject = queue;                            \
    if(queue->ucQueueType == queueQUEUE_TYPE_COUNTING_SEMAPHORE)                \
    {                                                                           \
        CurrentTaskSwitchRecord.Reason = SWITCH_REASON_COUNTING_SEMAPHORE_GIVE; \
    }                                                                           \
    else if(queue->ucQueueType == queueQUEUE_TYPE_BASE)                         \
    {                                                                           \
        CurrentTaskSwitchRecord.Reason = SWITCH_REASON_BLOCKED_QUEUE_PUSH;      \
    }                                                                           \
    else                                                                        \
    {                                                                           \
        CurrentTaskSwitchRecord.Reason = SWITCH_REASON_BLOCKED_OTHER;           \
    }

#define traceBLOCKING_ON_QUEUE_RECEIVE(queue)                                                                         \
    CurrentTaskSwitchRecord.BlockedOnObject = queue;                                                                  \
    if(queue->ucQueueType == queueQUEUE_TYPE_BINARY_SEMAPHORE)                                                        \
    {                                                                                                                 \
        CurrentTaskSwitchRecord.Reason = SWITCH_REASON_BLOCKED_BINARY_SEMAPHORE;                                      \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_COUNTING_SEMAPHORE)                                                 \
    {                                                                                                                 \
        CurrentTaskSwitchRecord.Reason = SWITCH_REASON_COUNTING_SEMAPHORE_RECEIVE;                                    \
    }                                                                                                                 \
    else if((queue->ucQueueType == queueQUEUE_TYPE_MUTEX) || (queue->ucQueueType == queueQUEUE_TYPE_RECURSIVE_MUTEX)) \
    {                                                                                                                 \
        CurrentTaskSwitchRecord.Reason = SWITCH_REASON_BLOCKED_MUTEX;                                                 \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_BASE)                                                               \
    {                                                                                                                 \
        CurrentTaskSwitchRecord.Reason = SWITCH_REASON_BLOCKED_QUEUE_POP;                                             \
    }                                                                                                                 \
    else                                                                                                              \
    {                                                                                                                 \
        CurrentTaskSwitchRecord.Reason = SWITCH_REASON_BLOCKED_OTHER;                                                 \
    }

#define traceEVENT_GROUP_WAIT_BITS_BLOCK(event_group, bits) \
    CurrentTaskSwitchRecord.BlockedOnObject = event_group;  \
    CurrentTaskSwitchRecord.Reason = SWITCH_REASON_BLOCKED_EVENT_GROUP;
#define traceEVENT_GROUP_SYNC_BLOCK(event_group, set_bits, wait_bits) \
    CurrentTaskSwitchRecord.BlockedOnObject = event_group;            \
    CurrentTaskSwitchRecord.Reason = SWITCH_REASON_BLOCKED_EVENT_GROUP;

#define traceTASK_NOTIFY_WAIT_BLOCK(index)                  \
    CurrentTaskSwitchRecord.BlockedOnObject = (void*)index; \
    CurrentTaskSwitchRecord.Reason = SWITCH_REASON_TASK_NOTIFY_WAIT;

#define traceTASK_NOTIFY(index) OnTaskNotify(pxTCB, index, false, eAction, pxTCB->ulNotifiedValue[index]);
#define traceTASK_NOTIFY_FROM_ISR(index) OnTaskNotify(pxTCB, index, true, eAction, pxTCB->ulNotifiedValue[index]);
#define traceTASK_NOTIFY_WAIT(index) OnTaskNotifyReceived(pxCurrentTCB, index, pxCurrentTCB->ulNotifiedValue[index] & ~ulBitsToClearOnExit);
