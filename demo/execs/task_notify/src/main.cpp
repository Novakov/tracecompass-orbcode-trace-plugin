#include <iterator>
#include "FreeRTOS.h"
#include "platform.hpp"
#include "task.h"

static void WaitForNotify(void*)
{
    while(true)
    {
        xTaskNotifyWait(0, 0x000F, nullptr, portMAX_DELAY);
    }
}

static StackType_t WaitForNotifyStack[256];
static StaticTask_t WaitForNotifyTask;
static TaskHandle_t WaitForNotifyHandle;

static void PeriodicNotifier(void*)
{
    int value = 0;
    while(true)
    {
        value++;
        vTaskDelay(pdMS_TO_TICKS(200));
        xTaskNotify(WaitForNotifyHandle, value, eSetValueWithOverwrite);
    }
}

static StackType_t PeriodicNotifierStack[256];
static StaticTask_t PeriodicNotifierTask;

void demo_main()
{
    WaitForNotifyHandle =
        xTaskCreateStatic(WaitForNotify, "WaitForNotify", std::size(WaitForNotifyStack), nullptr, 1, WaitForNotifyStack, &WaitForNotifyTask);
    xTaskCreateStatic(
        PeriodicNotifier, "PeriodicNotifier", std::size(PeriodicNotifierStack), nullptr, 1, PeriodicNotifierStack, &PeriodicNotifierTask);

    vTaskStartScheduler();
}