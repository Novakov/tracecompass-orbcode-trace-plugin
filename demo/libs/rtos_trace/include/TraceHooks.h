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
    extern void TraceOnBinarySemaphoreLock(void* queue, bool isr);
    extern void TraceOnBinarySemaphoreUnlock(void* queue, bool isr);
    extern void TraceOnBinarySemaphoreLockFailed(void* queue, bool isr);
    extern void TraceOnBinarySemaphoreUnlockFailed(void* queue, bool isr);

    extern void TraceOnCountingSemaphoreCreate(void* semaphore, uint32_t maxCount, uint32_t initialCount);
    extern void TraceOnCountingSemaphoreTake(void* semaphore, bool isr, uint32_t newCount);
    extern void TraceOnCountingSemaphoreGive(void* semaphore, bool isr, uint32_t newCount);
    extern void TraceOnCountingSemaphoreTakeFailed(void* semaphore, bool isr);
    extern void TraceOnCountingSemaphoreGiveFailed(void* semaphore, bool isr);

    extern void TraceOnMutexCreate(void* mutex);
    extern void TraceOnMutexLocked(void* mutex, bool isr);
    extern void TraceOnMutexUnlocked(void* mutex, bool isr);
    extern void TraceOnMutexLockFailed(void* mutex, bool isr);
    extern void TraceOnMutexUnlockFailed(void* mutex, bool isr);

    extern void TraceOnQueueCreate(void* queue, uint32_t capacity);
    extern void TraceOnQueuePush(void* queue, bool isr, uint32_t updatedItemsCount);
    extern void TraceOnQueuePop(void* queue, bool isr, uint32_t updatedItemsCount);
    extern void TraceOnQueuePushFailed(void* queue, bool isr);
    extern void TraceOnQueuePopFailed(void* queue, bool isr);

    extern void OnTaskNotify(void* task, uint32_t index, bool isr, uint32_t action, uint32_t updatedValue);
    extern void OnTaskNotifyReceived(void* task, uint32_t index, uint32_t updatedValue);

#ifdef __cplusplus
}
#endif

__attribute__((unused)) static int xCopyPosition = 0;

enum SwitchReasonObject
{
    SWITCH_REASON_OBJECT_BINARY_SEMAPHORE = 1 << 4,
    SWITCH_REASON_OBJECT_QUEUE = 2 << 4,
    SWITCH_REASON_OBJECT_COUNTING_SEMAPHORE = 3 << 4,
    SWITCH_REASON_OBJECT_MUTEX = 4 << 4,
};

enum SwitchReason
{
    SWITCH_REASON_TICK = 0,
    SWITCH_REASON_DELAYED = 1,
    SWITCH_REASON_TASK_NOTIFY_WAIT = 3,

    SWITCH_REASON_BLOCKED_QUEUE_PUSH = SWITCH_REASON_OBJECT_QUEUE | 0,
    SWITCH_REASON_BLOCKED_QUEUE_POP = SWITCH_REASON_OBJECT_QUEUE | 1,

    SWITCH_REASON_BLOCKED_BINARY_SEMAPHORE_RECEIVE = SWITCH_REASON_OBJECT_BINARY_SEMAPHORE | 0,
    SWITCH_REASON_BLOCKED_BINARY_SEMAPHORE_GIVE = SWITCH_REASON_OBJECT_BINARY_SEMAPHORE | 1,

    SWITCH_REASON_BLOCKED_EVENT_GROUP = 6,
    SWITCH_REASON_COUNTING_SEMAPHORE_GIVE = SWITCH_REASON_OBJECT_COUNTING_SEMAPHORE | 0,
    SWITCH_REASON_COUNTING_SEMAPHORE_TAKE = SWITCH_REASON_OBJECT_COUNTING_SEMAPHORE | 1,

    SWITCH_REASON_BLOCKED_MUTEX_LOCK = SWITCH_REASON_OBJECT_MUTEX | 0,
    SWITCH_REASON_BLOCKED_MUTEX_UNLOCK = SWITCH_REASON_OBJECT_MUTEX | 1,

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
        TraceOnBinarySemaphoreLock(queue, false);                                                                     \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_COUNTING_SEMAPHORE)                                                 \
    {                                                                                                                 \
        TraceOnCountingSemaphoreTake(queue, false, queue->uxMessagesWaiting - 1);                                     \
    }                                                                                                                 \
    else if((queue->ucQueueType == queueQUEUE_TYPE_MUTEX) || (queue->ucQueueType == queueQUEUE_TYPE_RECURSIVE_MUTEX)) \
    {                                                                                                                 \
        TraceOnMutexLocked(queue, false);                                                                             \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_BASE)                                                               \
    {                                                                                                                 \
        TraceOnQueuePop(queue, false, queue->uxMessagesWaiting - 1);                                                  \
    }

#define traceQUEUE_RECEIVE_FAILED(queue)                                                                              \
    taskENTER_CRITICAL();                                                                                             \
    if(queue->ucQueueType == queueQUEUE_TYPE_BINARY_SEMAPHORE)                                                        \
    {                                                                                                                 \
        TraceOnBinarySemaphoreLockFailed(queue, false);                                                               \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_COUNTING_SEMAPHORE)                                                 \
    {                                                                                                                 \
        TraceOnCountingSemaphoreTakeFailed(queue, false);                                                             \
    }                                                                                                                 \
    else if((queue->ucQueueType == queueQUEUE_TYPE_MUTEX) || (queue->ucQueueType == queueQUEUE_TYPE_RECURSIVE_MUTEX)) \
    {                                                                                                                 \
        TraceOnMutexLockFailed(queue, false);                                                                         \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_BASE)                                                               \
    {                                                                                                                 \
        TraceOnQueuePopFailed(queue, false);                                                                          \
    }                                                                                                                 \
    taskEXIT_CRITICAL();

#define traceQUEUE_RECEIVE_FROM_ISR(queue)                                                                            \
    if(queue->ucQueueType == queueQUEUE_TYPE_BINARY_SEMAPHORE)                                                        \
    {                                                                                                                 \
        TraceOnBinarySemaphoreLock(queue, true);                                                                      \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_COUNTING_SEMAPHORE)                                                 \
    {                                                                                                                 \
        TraceOnCountingSemaphoreTake(queue, true, queue->uxMessagesWaiting - 1);                                      \
    }                                                                                                                 \
    else if((queue->ucQueueType == queueQUEUE_TYPE_MUTEX) || (queue->ucQueueType == queueQUEUE_TYPE_RECURSIVE_MUTEX)) \
    {                                                                                                                 \
        TraceOnMutexLocked(queue, true);                                                                              \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_BASE)                                                               \
    {                                                                                                                 \
        TraceOnQueuePop(queue, true, queue->uxMessagesWaiting - 1);                                                   \
    }

#define traceQUEUE_RECEIVE_FROM_ISR_FAILED(queue)                                                                     \
    UBaseType_t traceSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();                                            \
    if(queue->ucQueueType == queueQUEUE_TYPE_BINARY_SEMAPHORE)                                                        \
    {                                                                                                                 \
        TraceOnBinarySemaphoreLockFailed(queue, true);                                                                \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_COUNTING_SEMAPHORE)                                                 \
    {                                                                                                                 \
        TraceOnCountingSemaphoreTakeFailed(queue, true);                                                              \
    }                                                                                                                 \
    else if((queue->ucQueueType == queueQUEUE_TYPE_MUTEX) || (queue->ucQueueType == queueQUEUE_TYPE_RECURSIVE_MUTEX)) \
    {                                                                                                                 \
        TraceOnMutexLockFailed(queue, true);                                                                          \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_BASE)                                                               \
    {                                                                                                                 \
        TraceOnQueuePopFailed(queue, true);                                                                           \
    }                                                                                                                 \
    taskEXIT_CRITICAL_FROM_ISR(traceSavedInterruptStatus);

#define traceQUEUE_SEND(queue)                                                                                        \
    if(queue->ucQueueType == queueQUEUE_TYPE_BINARY_SEMAPHORE)                                                        \
    {                                                                                                                 \
        TraceOnBinarySemaphoreUnlock(queue, false);                                                                   \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_COUNTING_SEMAPHORE)                                                 \
    {                                                                                                                 \
        TraceOnCountingSemaphoreGive(queue, false, queue->uxMessagesWaiting + 1);                                     \
    }                                                                                                                 \
    else if((queue->ucQueueType == queueQUEUE_TYPE_MUTEX) || (queue->ucQueueType == queueQUEUE_TYPE_RECURSIVE_MUTEX)) \
    {                                                                                                                 \
        TraceOnMutexUnlocked(queue, false);                                                                           \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_BASE)                                                               \
    {                                                                                                                 \
        if((queue->uxMessagesWaiting == queue->uxLength) && (xCopyPosition == queueOVERWRITE))                        \
        {                                                                                                             \
            TraceOnQueuePush(queue, false, queue->uxMessagesWaiting);                                                 \
        }                                                                                                             \
        else                                                                                                          \
        {                                                                                                             \
            TraceOnQueuePush(queue, false, queue->uxMessagesWaiting + 1);                                             \
        }                                                                                                             \
    }

#define traceQUEUE_SEND_FAILED(queue)                                                                                 \
    taskENTER_CRITICAL();                                                                                             \
    if(queue->ucQueueType == queueQUEUE_TYPE_BINARY_SEMAPHORE)                                                        \
    {                                                                                                                 \
        TraceOnBinarySemaphoreUnlockFailed(queue, false);                                                             \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_COUNTING_SEMAPHORE)                                                 \
    {                                                                                                                 \
        TraceOnCountingSemaphoreGiveFailed(queue, false);                                                             \
    }                                                                                                                 \
    else if((queue->ucQueueType == queueQUEUE_TYPE_MUTEX) || (queue->ucQueueType == queueQUEUE_TYPE_RECURSIVE_MUTEX)) \
    {                                                                                                                 \
        TraceOnMutexUnlockFailed(queue, false);                                                                       \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_BASE)                                                               \
    {                                                                                                                 \
        TraceOnQueuePushFailed(queue, false);                                                                         \
    }                                                                                                                 \
    taskEXIT_CRITICAL();

#define traceQUEUE_SEND_FROM_ISR(queue)                                                                               \
    if(queue->ucQueueType == queueQUEUE_TYPE_BINARY_SEMAPHORE)                                                        \
    {                                                                                                                 \
        TraceOnBinarySemaphoreUnlock(queue, true);                                                                    \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_COUNTING_SEMAPHORE)                                                 \
    {                                                                                                                 \
        TraceOnCountingSemaphoreGive(queue, true, queue->uxMessagesWaiting + 1);                                      \
    }                                                                                                                 \
    else if((queue->ucQueueType == queueQUEUE_TYPE_MUTEX) || (queue->ucQueueType == queueQUEUE_TYPE_RECURSIVE_MUTEX)) \
    {                                                                                                                 \
        TraceOnMutexUnlocked(queue, true);                                                                            \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_BASE)                                                               \
    {                                                                                                                 \
        if((queue->uxMessagesWaiting == queue->uxLength) && (xCopyPosition == queueOVERWRITE))                        \
        {                                                                                                             \
            TraceOnQueuePush(queue, true, queue->uxMessagesWaiting);                                                  \
        }                                                                                                             \
        else                                                                                                          \
        {                                                                                                             \
            TraceOnQueuePush(queue, true, queue->uxMessagesWaiting + 1);                                              \
        }                                                                                                             \
    }

#define traceQUEUE_SEND_FROM_ISR_FAILED(queue)                                                                        \
    UBaseType_t traceSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();                                            \
    if(queue->ucQueueType == queueQUEUE_TYPE_BINARY_SEMAPHORE)                                                        \
    {                                                                                                                 \
        TraceOnBinarySemaphoreUnlockFailed(queue, true);                                                              \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_COUNTING_SEMAPHORE)                                                 \
    {                                                                                                                 \
        TraceOnCountingSemaphoreGiveFailed(queue, true);                                                              \
    }                                                                                                                 \
    else if((queue->ucQueueType == queueQUEUE_TYPE_MUTEX) || (queue->ucQueueType == queueQUEUE_TYPE_RECURSIVE_MUTEX)) \
    {                                                                                                                 \
        TraceOnMutexUnlockFailed(queue, true);                                                                        \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_BASE)                                                               \
    {                                                                                                                 \
        TraceOnQueuePushFailed(queue, true);                                                                          \
    }                                                                                                                 \
    taskEXIT_CRITICAL_FROM_ISR(traceSavedInterruptStatus);

#define traceCREATE_MUTEX(mutex) \
    taskENTER_CRITICAL();        \
    TraceOnMutexCreate(mutex);   \
    taskEXIT_CRITICAL();

#define tracePOST_MOVED_TASK_TO_READY_STATE(task) TraceOnTaskReadied(task)

#define traceTASK_DELAY() CurrentTaskSwitchRecord.Reason = SWITCH_REASON_DELAYED;

#define traceTASK_DELAY_UNTIL(xTimeToWake) CurrentTaskSwitchRecord.Reason = SWITCH_REASON_DELAYED;

#define traceBLOCKING_ON_QUEUE_SEND(queue)                                                                            \
    CurrentTaskSwitchRecord.BlockedOnObject = queue;                                                                  \
    if(queue->ucQueueType == queueQUEUE_TYPE_COUNTING_SEMAPHORE)                                                      \
    {                                                                                                                 \
        CurrentTaskSwitchRecord.Reason = SWITCH_REASON_COUNTING_SEMAPHORE_GIVE;                                       \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_BINARY_SEMAPHORE)                                                   \
    {                                                                                                                 \
        CurrentTaskSwitchRecord.Reason = SWITCH_REASON_BLOCKED_BINARY_SEMAPHORE_GIVE;                                 \
    }                                                                                                                 \
    else if((queue->ucQueueType == queueQUEUE_TYPE_MUTEX) || (queue->ucQueueType == queueQUEUE_TYPE_RECURSIVE_MUTEX)) \
    {                                                                                                                 \
        CurrentTaskSwitchRecord.Reason = SWITCH_REASON_BLOCKED_MUTEX_UNLOCK;                                          \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_BASE)                                                               \
    {                                                                                                                 \
        CurrentTaskSwitchRecord.Reason = SWITCH_REASON_BLOCKED_QUEUE_PUSH;                                            \
    }                                                                                                                 \
    else                                                                                                              \
    {                                                                                                                 \
        CurrentTaskSwitchRecord.Reason = SWITCH_REASON_BLOCKED_OTHER;                                                 \
    }

#define traceBLOCKING_ON_QUEUE_RECEIVE(queue)                                                                         \
    CurrentTaskSwitchRecord.BlockedOnObject = queue;                                                                  \
    if(queue->ucQueueType == queueQUEUE_TYPE_BINARY_SEMAPHORE)                                                        \
    {                                                                                                                 \
        CurrentTaskSwitchRecord.Reason = SWITCH_REASON_BLOCKED_BINARY_SEMAPHORE_RECEIVE;                              \
    }                                                                                                                 \
    else if(queue->ucQueueType == queueQUEUE_TYPE_COUNTING_SEMAPHORE)                                                 \
    {                                                                                                                 \
        CurrentTaskSwitchRecord.Reason = SWITCH_REASON_COUNTING_SEMAPHORE_TAKE;                                       \
    }                                                                                                                 \
    else if((queue->ucQueueType == queueQUEUE_TYPE_MUTEX) || (queue->ucQueueType == queueQUEUE_TYPE_RECURSIVE_MUTEX)) \
    {                                                                                                                 \
        CurrentTaskSwitchRecord.Reason = SWITCH_REASON_BLOCKED_MUTEX_LOCK;                                            \
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
