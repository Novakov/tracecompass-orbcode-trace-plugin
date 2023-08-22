#include <iterator>
#include "FreeRTOS.h"
#include "platform.hpp"
#include "task.h"

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
    xTaskCreateStatic(Blink1, "Blink1", std::size(Blink1Stack), nullptr, 1, Blink1Stack, &Blink1Task);
    xTaskCreateStatic(Blink2, "Blink2", std::size(Blink2Stack), nullptr, 1, Blink2Stack, &Blink2Task);

    vTaskStartScheduler();
}