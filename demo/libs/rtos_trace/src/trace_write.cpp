#include <string_view>
#include "FreeRTOS.h"
#include "trace.hpp"

enum class TraceEvent : std::uint8_t
{
    TaskSwitchedIn = 1u,
    TaskSwitchedOut = 2u,
    TaskCreated = 3u,
    TaskReadied = 4u,
    BinarySemCreated = 5u,
    BinarySemLocked = 6u,
    BinarySemUnlocked = 7u,
    BinarySemLockFailed = 8u,
    BinarySemUnlockFailed = 9u,
    QueueCreated = 10u,
    QueuePushed = 11u,
    QueuePopped = 12u,
    QueuePushFailed = 13u,
    QueuePopFailed = 14u,
    CountingSemCreated = 15u,
    CountingSemGiven = 16u,
    CountingSemTaken = 17u,
    CountingSemGiveFailed = 18u,
    CountingSemTakeFailed = 19u,
    TaskNotify = 20u,
    TaskNotifyReceived = 21u,
    MutexCreated = 22u,
    MutexLocked = 23u,
    MutexUnlocked = 24u,
    MutexLockFailed = 25u,
    MutexUnlockFailed = 26u,
};

SwitchRecord CurrentTaskSwitchRecord;

class EventPacket
{
public:
    explicit EventPacket(TraceEvent eventId) : _eventId{eventId}
    {
        ITMWrite16(3, (0xBE << 8) | static_cast<std::uint8_t>(eventId));
    }

    ~EventPacket()
    {
        ITMWrite16(3, (0xEF << 8) | static_cast<std::uint8_t>(_eventId));
    }

private:
    TraceEvent _eventId;
};

void TraceOnTaskSwitchedIn(void* tcb)
{
    EventPacket packet{TraceEvent::TaskSwitchedIn};
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(tcb));
}

void TraceOnTaskSwitchedOut(void* tcb, bool still_ready)
{
    EventPacket packet{TraceEvent::TaskSwitchedOut};
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(tcb));
    ITMWrite8(2, CurrentTaskSwitchRecord.Reason);
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(CurrentTaskSwitchRecord.BlockedOnObject));
    ITMWrite8(2, (still_ready ? 1 : 0) << 0);
}

void TraceOnTaskReadied(void* tcb)
{
    EventPacket packet{TraceEvent::TaskReadied};
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(tcb));
}

void TraceOnTaskCreated(void* tcb, const char* name)
{
    EventPacket packet{TraceEvent::TaskCreated};
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(tcb));
    std::string_view nameView{name};
    ITMWrite8(2, nameView.size());
    ITMWriteBuffer(2, reinterpret_cast<const std::byte*>(nameView.data()), nameView.size());
}

void TraceOnBinarySemaphoreCreate(void* queue)
{
    EventPacket packet{TraceEvent::BinarySemCreated};
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(queue));
}

void TraceOnBinarySemaphoreLock(void* queue, bool isr)
{
    EventPacket packet{TraceEvent::BinarySemLocked};
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(queue));
    ITMWrite8(2, ((isr ? 1 : 0) << 0));
}

void TraceOnBinarySemaphoreUnlock(void* queue, bool isr)
{
    EventPacket packet{TraceEvent::BinarySemUnlocked};
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(queue));
    ITMWrite8(2, ((isr ? 1 : 0) << 1));
}

void TraceOnBinarySemaphoreLockFailed(void* queue, bool isr)
{
    EventPacket packet{TraceEvent::BinarySemLockFailed};
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(queue));
    ITMWrite8(2, ((isr ? 1 : 0) << 0));
}

void TraceOnBinarySemaphoreUnlockFailed(void* queue, bool isr)
{
    EventPacket packet{TraceEvent::BinarySemUnlockFailed};
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(queue));
    ITMWrite8(2, ((isr ? 1 : 0) << 1));
}

void TraceOnCountingSemaphoreCreate(void* semaphore, uint32_t maxCount, uint32_t initialCount)
{
    EventPacket packet{TraceEvent::CountingSemCreated};
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(semaphore));
    ITMWrite32(2, (maxCount & 0xFFFF) | ((initialCount & 0xFFFF) << 16));
}

void TraceOnCountingSemaphoreTake(void* semaphore, bool isr, uint32_t newCount)
{
    EventPacket packet{TraceEvent::CountingSemTaken};
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(semaphore));
    ITMWrite32(2, ((isr ? 1 : 0) << 0) | ((newCount & 0xFFFF) << 16));
}

void TraceOnCountingSemaphoreGive(void* semaphore, bool isr, uint32_t newCount)
{
    EventPacket packet{TraceEvent::CountingSemGiven};
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(semaphore));
    ITMWrite32(2, ((isr ? 1 : 0) << 0) | ((newCount & 0xFFFF) << 16));
}

void TraceOnCountingSemaphoreTakeFailed(void* semaphore, bool isr)
{
    EventPacket packet{TraceEvent::CountingSemTakeFailed};
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(semaphore));
    ITMWrite8(2, (isr ? 1 : 0) << 0);
}

void TraceOnCountingSemaphoreGiveFailed(void* semaphore, bool isr)
{
    EventPacket packet{TraceEvent::CountingSemGiveFailed};
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(semaphore));
    ITMWrite8(2, (isr ? 1 : 0) << 0);
}

void TraceOnMutexCreate(void* mutex)
{
    EventPacket packet{TraceEvent::MutexCreated};
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(mutex));
}

void TraceOnMutexLocked(void* mutex, bool isr)
{
    EventPacket packet{TraceEvent::MutexLocked};
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(mutex));
    ITMWrite8(2, (isr ? 1 : 0) << 0);
}

void TraceOnMutexLockFailed(void* mutex, bool isr)
{
    EventPacket packet{TraceEvent::MutexLockFailed};
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(mutex));
    ITMWrite8(2, (isr ? 1 : 0) << 0);
}

void TraceOnMutexUnlocked(void* mutex, bool isr)
{
    EventPacket packet{TraceEvent::MutexUnlocked};
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(mutex));
    ITMWrite8(2, (isr ? 1 : 0) << 0);
}

void TraceOnMutexUnlockFailed(void* mutex, bool isr)
{
    EventPacket packet{TraceEvent::MutexUnlockFailed};
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(mutex));
    ITMWrite8(2, (isr ? 1 : 0) << 0);
}

void TraceOnQueueCreate(void* queue, uint32_t capacity)
{
    EventPacket packet{TraceEvent::QueueCreated};
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(queue));
    ITMWrite16(2, capacity);
}

void TraceOnQueuePush(void* queue, bool isr, uint32_t updatedItemsCount)
{
    EventPacket packet{TraceEvent::QueuePushed};
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(queue));
    ITMWrite32(2, ((isr ? 1 : 0) << 0) | ((updatedItemsCount & 0xFFFF) << 16));
}

void TraceOnQueuePop(void* queue, bool isr, uint32_t updatedItemsCount)
{
    EventPacket packet{TraceEvent::QueuePopped};
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(queue));
    ITMWrite32(2, ((isr ? 1 : 0) << 0) | ((updatedItemsCount & 0xFFFF) << 16));
}

void TraceOnQueuePushFailed(void* queue, bool isr)
{
    EventPacket packet{TraceEvent::QueuePushFailed};
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(queue));
    ITMWrite32(2, ((isr ? 1 : 0) << 0));
}

void TraceOnQueuePopFailed(void* queue, bool isr)
{
    EventPacket packet{TraceEvent::QueuePopFailed};
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(queue));
    ITMWrite32(2, ((isr ? 1 : 0) << 0));
}

extern void OnTaskNotify(void* task, uint32_t index, bool isr, uint32_t action, uint32_t updatedValue)
{
    EventPacket packet{TraceEvent::TaskNotify};
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(task));
    uint32_t flags = 0;
    flags |= index & 0xFFFF;
    flags |= (action & 0xF) << 16;
    flags |= (isr ? 1 : 0) << 24;
    ITMWrite32(2, flags);
    ITMWrite32(2, updatedValue);
}

extern void OnTaskNotifyReceived(void* task, uint32_t index, uint32_t updatedValue)
{
    EventPacket packet{TraceEvent::TaskNotifyReceived};
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(task));
    ITMWrite16(2, index);
    ITMWrite32(2, updatedValue);
}
