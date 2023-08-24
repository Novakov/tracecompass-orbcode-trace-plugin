#include <atomic>
#include <cstdint>
#include <string_view>
#include "FreeRTOS.h"
#include "trace.hpp"

#define TRACE_EVENT_ID_SHIFT 28u
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

#define CYCCNT_MASK 0x0FFF'FFFFu

static std::uint32_t GetUs();

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
    ITMWrite32(2, TRACE_EVENT_TASK_SWITCHED_IN | (GetUs() & CYCCNT_MASK));
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(tcb));
    TraceFooter();
}

void TraceOnTaskSwitchedOut(void* tcb, bool still_ready)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_TASK_SWITCHED_OUT | (GetUs() & CYCCNT_MASK));
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(tcb));
    ITMWrite32(2, (CurrentTaskSwitchRecord.Reason << 28));
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(CurrentTaskSwitchRecord.BlockedOnObject));
    ITMWrite8(2, (still_ready ? 1 : 0) << 0);
    TraceFooter();
}

void TraceOnTaskReadied(void* tcb)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_TASK_READIED | (GetUs() & CYCCNT_MASK));
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(tcb));
    TraceFooter();
}

void TraceOnTaskCreated(void* tcb, const char* name)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_TASK_CREATED | (GetUs() & CYCCNT_MASK));
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(tcb));
    std::string_view nameView{name};
    ITMWrite8(2, nameView.size());
    ITMWriteBuffer(2, reinterpret_cast<const std::byte*>(nameView.data()), nameView.size());
    TraceFooter();
}

void TraceOnBinarySemaphoreCreate(void* queue)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_BINARY_SEM_CREATED | (GetUs() & CYCCNT_MASK));
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(queue));
    TraceFooter();
}

void TraceOnBinarySemaphoreLock(void* queue, bool isr, bool success)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_BINARY_SEM_LOCKING | (GetUs() & CYCCNT_MASK));
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(queue));
    ITMWrite8(2, (1 << 0) | ((isr ? 1 : 0) << 1) | ((success ? 1 : 0) << 2));
    TraceFooter();
}

extern void TraceOnBinarySemaphoreUnlock(void* queue, bool isr, bool success)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_BINARY_SEM_LOCKING | (GetUs() & CYCCNT_MASK));
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(queue));
    ITMWrite8(2, (0 << 0) | ((isr ? 1 : 0) << 1) | ((success ? 1 : 0) << 2));
    TraceFooter();
}

void TraceOnCountingSemaphoreCreate(void* semaphore, uint32_t maxCount, uint32_t initialCount)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_COUNTING_SEM_CREATED | (GetUs() & CYCCNT_MASK));
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(semaphore));
    ITMWrite32(2, maxCount);
    ITMWrite32(2, initialCount);
    TraceFooter();
}

void TraceOnCountingSemaphoreTake(void* semaphore, bool isr, bool success, uint32_t newCount)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_COUNTING_SEM_GIVE_TAKE | (GetUs() & CYCCNT_MASK));
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(semaphore));
    ITMWrite32(2, (1 << 0) | ((isr ? 1 : 0) << 1) | ((success ? 1 : 0) << 2) | ((newCount & 0xFFFF) << 16));
    TraceFooter();
}

void TraceOnCountingSemaphoreGive(void* semaphore, bool isr, bool success, uint32_t newCount)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_COUNTING_SEM_GIVE_TAKE | (GetUs() & CYCCNT_MASK));
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(semaphore));
    ITMWrite32(2, (0 << 0) | ((isr ? 1 : 0) << 1) | ((success ? 1 : 0) << 2) | ((newCount & 0xFFFF) << 16));
    TraceFooter();
}

void TraceOnMutexCreate(void* mutex)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_MUTEX_CREATED | (GetUs() & CYCCNT_MASK));
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(mutex));
    TraceFooter();
}

void TraceOnMutexLock(void* mutex, bool isr, bool success)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_MUTEX_LOCKING | (GetUs() & CYCCNT_MASK));
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(mutex));
    ITMWrite8(2, (1 << 0) | ((isr ? 1 : 0) << 1) | ((success ? 1 : 0) << 2));
    TraceFooter();
}

void TraceOnMutexUnlock(void* mutex, bool isr, bool success)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_MUTEX_LOCKING | (GetUs() & CYCCNT_MASK));
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(mutex));
    ITMWrite8(2, (0 << 0) | ((isr ? 1 : 0) << 1) | ((success ? 1 : 0) << 2));
    TraceFooter();
}

void TraceOnQueueCreate(void* queue, uint32_t capacity)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_QUEUE_CREATED | (GetUs() & CYCCNT_MASK));
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(queue));
    ITMWrite16(2, capacity);
    TraceFooter();
}

void TraceOnQueuePush(void* queue, bool isr, bool success, uint32_t updatedItemsCount)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_QUEUE_PUSH_POP | (GetUs() & CYCCNT_MASK));
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(queue));
    ITMWrite32(2, (0 << 0) | ((isr ? 1 : 0) << 1) | ((success ? 1 : 0) << 2) | ((updatedItemsCount & 0xFFFF) << 16));
    TraceFooter();
}

void TraceOnQueuePop(void* queue, bool isr, bool success, uint32_t updatedItemsCount)
{
    TraceHeader();
    ITMWrite32(2, TRACE_EVENT_QUEUE_PUSH_POP | (GetUs() & CYCCNT_MASK));
    ITMWrite32(2, reinterpret_cast<std::uintptr_t>(queue));
    ITMWrite32(2, (1 << 0) | ((isr ? 1 : 0) << 1) | ((success ? 1 : 0) << 2) | ((updatedItemsCount & 0xFFFF) << 16));
    TraceFooter();
}

void SetupTrace()
{
    TpiuOptions tpiu;
    tpiu.TracePortWidth = 4;
    tpiu.FormattingEnabled = true;
    tpiu.Protocol = TpiuProtocolParallel;
    tpiu.SwoPrescaler = 0;

    ITMOptions itm;
    itm.EnabledStimulusPorts = ITM_ENABLE_STIMULUS_PORTS_ALL;
    itm.EnableLocalTimestamp = false;
    itm.EnableSyncPacket = false;
    itm.ForwardDWT = false;
    itm.GlobalTimestampFrequency = ITMGlobalTimestampFrequencyDisabled;
    itm.LocalTimestampPrescaler = ITMLocalTimestampPrescalerNoPrescaling;
    itm.TraceBusID = 1;

    DWTOptions dwt;
    dwt.CycleTap = DWTCycleTap10;
    dwt.CPICounterEvent = false;
    dwt.ExceptionOverheadCounterEvent = false;
    dwt.ExceptionTrace = false;
    dwt.FoldedInstructionCounterEvent = false;
    dwt.LSUCounterEvent = false;
    dwt.PCSampling = false;
    dwt.SamplingPrescaler = 1;
    dwt.SleepCounterEvent = false;
    dwt.SyncTap = DWTSyncTap28;

    TpiuSetup(&tpiu);
    ITMSetup(&itm);
    DWTSetup(&dwt);
}

static inline std::uint32_t GetUs()
{
    return DWT->CYCCNT;
}