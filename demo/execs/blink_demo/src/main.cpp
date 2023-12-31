#include <iterator>
#include "FreeRTOS.h"
#include "platform.hpp"
#include "task.h"
#include "trace.hpp"

static void Blink(void*)
{
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

    TpiuSetup(&tpiu);
    ITMSetup(&itm);

    xTaskCreateStatic(Blink, "Blink", std::size(BlinkStack), nullptr, 1, BlinkStack, &BlinkTask);

    vTaskStartScheduler();
}