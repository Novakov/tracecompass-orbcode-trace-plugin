#include <iterator>
#include "FreeRTOS.h"
#include "platform.hpp"
#include "task.h"
#include "trace.hpp"

static void Blink1(void*)
{
    while(true)
    {
        ToggleLed(Led::Led1);
        auto waitUntil = xTaskGetTickCount() + pdMS_TO_TICKS(250);
        while(xTaskGetTickCount() < waitUntil)
        {
            asm volatile("nop");
        }
    }
}

static StackType_t Blink1Stack[256];
static StaticTask_t Blink1Task;

static void Blink2(void*)
{
    while(true)
    {
        ToggleLed(Led::Led2);
        auto waitUntil = xTaskGetTickCount() + pdMS_TO_TICKS(250);
        while(xTaskGetTickCount() < waitUntil)
        {
            asm volatile("nop");
        }
    }
}

static StackType_t Blink2Stack[256];
static StaticTask_t Blink2Task;

void demo_main()
{
    TpiuOptions tpiu = {
        .Protocol = TpiuProtocolParallel,
        .FormattingEnabled = true,
        .TracePortWidth = 4,
    };

    ITMOptions itm = {
        .TraceBusID = 1,
        .GlobalTimestampFrequency = ITMGlobalTimestampFrequencyDisabled,
        .LocalTimestampPrescaler = ITMLocalTimestampPrescalerNoPrescaling,
        .EnableLocalTimestamp = true,
        .ForwardDWT = true,
        .EnableSyncPacket = true,
        .EnabledStimulusPorts = ITM_ENABLE_STIMULUS_PORTS_ALL,
    };

    TpiuSetup(&tpiu);
    ITMSetup(&itm);

    xTaskCreateStatic(Blink1, "Blink1", std::size(Blink1Stack), nullptr, 1, Blink1Stack, &Blink1Task);
    xTaskCreateStatic(Blink2, "Blink2", std::size(Blink2Stack), nullptr, 1, Blink2Stack, &Blink2Task);

    vTaskStartScheduler();
}