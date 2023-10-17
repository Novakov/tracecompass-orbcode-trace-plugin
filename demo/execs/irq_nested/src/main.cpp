#include <iterator>
#include "FreeRTOS.h"
#include "interrupts.hpp"
#include "platform.hpp"
#include "task.h"
#include "trace.hpp"

extern "C" void Interrupt1_IRQHandler()
{
    NVIC_ClearPendingIRQ(Interrupt1_IRQn);

    for(int i = 0; i < 10'000; i++)
    {
        __asm__ volatile("nop");
    }
    NVIC_SetPendingIRQ(Interrupt2_IRQn);

    for(int i = 0; i < 10'000; i++)
    {
        __asm__ volatile("nop");
    }
    NVIC_SetPendingIRQ(Interrupt2_IRQn);

    for(int i = 0; i < 10'000; i++)
    {
        __asm__ volatile("nop");
    }
}

extern "C" void Interrupt2_IRQHandler()
{
    NVIC_ClearPendingIRQ(Interrupt2_IRQn);

    for(int i = 0; i < 20'000; i++)
    {
        __asm__ volatile("nop");
    }
    NVIC_SetPendingIRQ(Interrupt3_IRQn);
    for(int i = 0; i < 20'000; i++)
    {
        __asm__ volatile("nop");
    }
    NVIC_SetPendingIRQ(Interrupt3_IRQn);
    for(int i = 0; i < 20'000; i++)
    {
        __asm__ volatile("nop");
    }
}

extern "C" void Interrupt3_IRQHandler()
{
    NVIC_ClearPendingIRQ(Interrupt3_IRQn);
    for(int i = 0; i < 30'000; i++)
    {
        __asm__ volatile("nop");
    }
}

static void Blink(void*)
{
    NVIC_SetPriority(Interrupt1_IRQn, 2);
    NVIC_SetPriority(Interrupt2_IRQn, 1);
    NVIC_SetPriority(Interrupt3_IRQn, 0);

    NVIC_EnableIRQ(Interrupt1_IRQn);
    NVIC_EnableIRQ(Interrupt2_IRQn);
    NVIC_EnableIRQ(Interrupt3_IRQn);

    char c = 'A';
    while(true)
    {
        ToggleLed(Led::Led1);
        ToggleLed(Led::Led2);
        ITMWrite8(4, c);
        c++;
        if(c > 'Z')
        {
            c = 'A';
        }

        NVIC_SetPendingIRQ(Interrupt1_IRQn);

        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

static StackType_t BlinkStack[256];
static StaticTask_t BlinkTask;

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

    DWTOptions dwt = {
        .ExceptionTrace = true,
    };

    TpiuSetup(&tpiu);
    ITMSetup(&itm);
    DWTSetup(&dwt);

    xTaskCreateStatic(Blink, "Blink", std::size(BlinkStack), nullptr, 1, BlinkStack, &BlinkTask);

    vTaskStartScheduler();
}