#include <atomic>
#include <cstdint>
#include <string_view>
#include "FreeRTOS.h"
#include "trace.hpp"

#define TRACE_EVENT_ID_SHIFT 16u
#define TRACE_EVENT_TASK_SWITCHED_IN (1u << TRACE_EVENT_ID_SHIFT)
#define TRACE_EVENT_TASK_SWITCHED_OUT (2u << TRACE_EVENT_ID_SHIFT)
#define TRACE_EVENT_TASK_CREATED (3u << TRACE_EVENT_ID_SHIFT)
#define TRACE_EVENT_BINARY_SEM_CREATED (4u << TRACE_EVENT_ID_SHIFT)
#define TRACE_EVENT_BINARY_SEM_LOCKING (5u << TRACE_EVENT_ID_SHIFT)
#define TRACE_EVENT_MUTEX_CREATED (6u << TRACE_EVENT_ID_SHIFT)
#define TRACE_EVENT_MUTEX_LOCKING (7u << TRACE_EVENT_ID_SHIFT)
#define TRACE_EVENT_QUEUE_CREATED (8u << TRACE_EVENT_ID_SHIFT)
#define TRACE_EVENT_QUEUE_PUSH_POP (9u << TRACE_EVENT_ID_SHIFT)
#define TRACE_EVENT_TASK_READIED (10u << TRACE_EVENT_ID_SHIFT)
#define TRACE_EVENT_COUNTING_SEM_CREATED (11u << TRACE_EVENT_ID_SHIFT)
#define TRACE_EVENT_COUNTING_SEM_GIVE_TAKE (12u << TRACE_EVENT_ID_SHIFT)
#define TRACE_EVENT_TASK_NOTIFY (13u << TRACE_EVENT_ID_SHIFT)
#define TRACE_EVENT_TASK_NOTIFY_RECEIVED (14u << TRACE_EVENT_ID_SHIFT)

SwitchRecord CurrentTaskSwitchRecord;

void TraceHeader()
{
    ITMWrite32(2, 0xDEADBEEF);
}

void TraceFooter()
{
    ITMWrite32(2, 0xCAFEBABE);
}

void TraceOnTaskSwitchedIn(void* tcb)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_TASK_SWITCHED_IN);
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(tcb));
    TraceFooter();
}

void TraceOnTaskSwitchedOut(void* tcb, bool still_ready)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_TASK_SWITCHED_OUT);
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(tcb));
    ITMWrite32(2, (CurrentTaskSwitchRecord.Reason << 28));
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(CurrentTaskSwitchRecord.BlockedOnObject));
    ITMWrite8(2, (still_ready ? 1 : 0) << 0);
    TraceFooter();
}

void TraceOnTaskReadied(void* tcb)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_TASK_READIED);
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(tcb));
    TraceFooter();
}

void TraceOnTaskCreated(void* tcb, const char* name)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_TASK_CREATED);
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(tcb));
    std::string_view nameView{name};
    ITMWrite8(2, nameView.size());
    ITMWriteBuffer(2, reinterpret_cast<const std::byte*>(nameView.data()), nameView.size());
    TraceFooter();
}

void TraceOnBinarySemaphoreCreate(void* queue)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_BINARY_SEM_CREATED);
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(queue));
    TraceFooter();
}

void TraceOnBinarySemaphoreLock(void* queue, bool isr, bool success)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_BINARY_SEM_LOCKING);
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(queue));
    ITMWrite8(2, (1 << 0) | ((isr ? 1 : 0) << 1) | ((success ? 1 : 0) << 2));
    TraceFooter();
}

extern void TraceOnBinarySemaphoreUnlock(void* queue, bool isr, bool success)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_BINARY_SEM_LOCKING);
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(queue));
    ITMWrite8(2, (0 << 0) | ((isr ? 1 : 0) << 1) | ((success ? 1 : 0) << 2));
    TraceFooter();
}

void TraceOnCountingSemaphoreCreate(void* semaphore, uint32_t maxCount, uint32_t initialCount)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_COUNTING_SEM_CREATED);
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(semaphore));
    ITMWrite32(2, maxCount);
    ITMWrite32(2, initialCount);
    TraceFooter();
}

void TraceOnCountingSemaphoreTake(void* semaphore, bool isr, bool success, uint32_t newCount)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_COUNTING_SEM_GIVE_TAKE);
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(semaphore));
    ITMWrite32(2, (1 << 0) | ((isr ? 1 : 0) << 1) | ((success ? 1 : 0) << 2) | ((newCount & 0xFFFF) << 16));
    TraceFooter();
}

void TraceOnCountingSemaphoreGive(void* semaphore, bool isr, bool success, uint32_t newCount)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_COUNTING_SEM_GIVE_TAKE);
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(semaphore));
    ITMWrite32(2, (0 << 0) | ((isr ? 1 : 0) << 1) | ((success ? 1 : 0) << 2) | ((newCount & 0xFFFF) << 16));
    TraceFooter();
}

void TraceOnMutexCreate(void* mutex)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_MUTEX_CREATED);
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(mutex));
    TraceFooter();
}

void TraceOnMutexLock(void* mutex, bool isr, bool success)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_MUTEX_LOCKING);
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(mutex));
    ITMWrite8(2, (1 << 0) | ((isr ? 1 : 0) << 1) | ((success ? 1 : 0) << 2));
    TraceFooter();
}

void TraceOnMutexUnlock(void* mutex, bool isr, bool success)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_MUTEX_LOCKING);
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(mutex));
    ITMWrite8(2, (0 << 0) | ((isr ? 1 : 0) << 1) | ((success ? 1 : 0) << 2));
    TraceFooter();
}

void TraceOnQueueCreate(void* queue, uint32_t capacity)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_QUEUE_CREATED);
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(queue));
    ITMWrite16(2, capacity);
    TraceFooter();
}

void TraceOnQueuePush(void* queue, bool isr, bool success, uint32_t updatedItemsCount)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_QUEUE_PUSH_POP);
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(queue));
    ITMWrite32(2, (0 << 0) | ((isr ? 1 : 0) << 1) | ((success ? 1 : 0) << 2) | ((updatedItemsCount & 0xFFFF) << 16));
    TraceFooter();
}

void TraceOnQueuePop(void* queue, bool isr, bool success, uint32_t updatedItemsCount)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_QUEUE_PUSH_POP);
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(queue));
    ITMWrite32(2, (1 << 0) | ((isr ? 1 : 0) << 1) | ((success ? 1 : 0) << 2) | ((updatedItemsCount & 0xFFFF) << 16));
    TraceFooter();
}

extern void OnTaskNotify(void* task, uint32_t index, uint32_t action, uint32_t updatedValue)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_TASK_NOTIFY);
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(task));
    ITMWrite32(2, (index & 0xFFFF) | ((action & 0xFFFF)) << 16);
    ITMWrite32(2, updatedValue);
    TraceFooter();
}

extern void OnTaskNotifyReceived(void* task, uint32_t index, uint32_t updatedValue)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_TASK_NOTIFY_RECEIVED);
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(task));
    ITMWrite16(2, index);
    ITMWrite32(2, updatedValue);
    TraceFooter();
}
