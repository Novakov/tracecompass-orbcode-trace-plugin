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
    xTaskCreateStatic(Blink, "Blink", std::size(BlinkStack), nullptr, 1, BlinkStack, &BlinkTask);

    vTaskStartScheduler();
}