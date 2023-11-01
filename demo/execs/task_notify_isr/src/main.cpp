#include <iterator>
#include "FreeRTOS.h"
#include "interrupts.hpp"
#include "platform.hpp"
#include "task.h"
#include "trace.hpp"

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

extern "C" void Interrupt1_IRQHandler()
{
    static int i = 0;
    i++;
    portBASE_TYPE yielded = false;
    xTaskNotifyFromISR(WaitForNotifyHandle, i, eSetValueWithOverwrite, &yielded);
    NVIC_ClearPendingIRQ(Interrupt1_IRQn);
    portYIELD_FROM_ISR(yielded);
}

static void PeriodicNotifier(void*)
{
    NVIC_SetPriority(Interrupt1_IRQn, 7);
    NVIC_EnableIRQ(Interrupt1_IRQn);

    while(true)
    {
        vTaskDelay(pdMS_TO_TICKS(200));
        NVIC_SetPendingIRQ(Interrupt1_IRQn);
    }
}

static StackType_t PeriodicNotifierStack[256];
static StaticTask_t PeriodicNotifierTask;

void demo_main()
{
    WaitForNotifyHandle =
        xTaskCreateStatic(WaitForNotify, "WaitForNotify", std::size(WaitForNotifyStack), nullptr, 2, WaitForNotifyStack, &WaitForNotifyTask);
    xTaskCreateStatic(
        PeriodicNotifier, "PeriodicNotifier", std::size(PeriodicNotifierStack), nullptr, 1, PeriodicNotifierStack, &PeriodicNotifierTask);

    vTaskStartScheduler();
}