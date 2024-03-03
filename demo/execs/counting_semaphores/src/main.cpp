#include <iterator>
#include "FreeRTOS.h"
#include "platform.hpp"
#include "semphr.h"
#include "task.h"

static void BusyDelay(portTickType delay)
{
    auto delayTo = xTaskGetTickCount() + delay;
    while(xTaskGetTickCount() < delayTo)
    {
        __asm__ volatile("nop");
    }
}

static StaticSemaphore_t FillingSemaphoreBuffer;
static SemaphoreHandle_t FillingSemaphore;

static void PeriodicFiller(void*)
{
    while(true)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));

        for(int i = 0; i < 10; i++)
        {
            xSemaphoreGive(FillingSemaphore);
        }
    }
}

static StackType_t PeriodicFillerStack[256];
static StaticTask_t PeriodicFillerTask;

static void Consumer(void*)
{
    while(true)
    {
        xSemaphoreTake(FillingSemaphore, portMAX_DELAY);
        BusyDelay(5);
    }
}

static StackType_t ConsumerStack[256];
static StaticTask_t ConsumerTask;

void demo_main()
{
    FillingSemaphore = xSemaphoreCreateCountingStatic(10, 0, &FillingSemaphoreBuffer);

    xTaskCreateStatic(
        PeriodicFiller, "PeriodicFiller", std::size(PeriodicFillerStack), nullptr, configMAX_PRIORITIES - 1, PeriodicFillerStack, &PeriodicFillerTask);
    xTaskCreateStatic(Consumer, "Consumer", std::size(ConsumerStack), nullptr, 1, ConsumerStack, &ConsumerTask);

    vTaskStartScheduler();
}