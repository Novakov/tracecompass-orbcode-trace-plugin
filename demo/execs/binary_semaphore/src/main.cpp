#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

static StaticSemaphore_t ObservedSemaphoreBuffer;
static SemaphoreHandle_t ObservedSemaphore;
static StaticSemaphore_t SwitchingSemaphoreBuffer;
static SemaphoreHandle_t SwitchingSemaphore;

static void Taker(void*)
{
    while(true)
    {
        xQueueSemaphoreTake(SwitchingSemaphore, portMAX_DELAY);

        xSemaphoreTake(ObservedSemaphore, 2);
        vTaskDelay(pdMS_TO_TICKS(100));
        xSemaphoreTake(ObservedSemaphore, 2);
        vTaskDelay(pdMS_TO_TICKS(100));

        xSemaphoreGive(SwitchingSemaphore);
    }
}

static StackType_t TakerStack[256];
static StaticTask_t TakerTask;

static void Giver(void*)
{
    xQueueSemaphoreTake(SwitchingSemaphore, portMAX_DELAY);
    xTaskCreateStatic(Taker, "Taker", sizeof(TakerStack) / sizeof(TakerStack[0]), nullptr, 1, TakerStack, &TakerTask);

    while(true)
    {
        xQueueSendToBack(ObservedSemaphore, nullptr, 2);
        vTaskDelay(pdMS_TO_TICKS(100));
        xQueueSendToBack(ObservedSemaphore, nullptr, 2);
        vTaskDelay(pdMS_TO_TICKS(100));

        xSemaphoreGive(SwitchingSemaphore);

        vTaskDelay(pdMS_TO_TICKS(500));
        xQueueSemaphoreTake(SwitchingSemaphore, portMAX_DELAY);
    }
}

static StackType_t GiverStack[256];
static StaticTask_t GiverTask;

void demo_main()
{
    ObservedSemaphore = xSemaphoreCreateBinaryStatic(&ObservedSemaphoreBuffer);
    SwitchingSemaphore = xSemaphoreCreateBinaryStatic(&SwitchingSemaphoreBuffer);

    xSemaphoreGive(SwitchingSemaphore);

    xTaskCreateStatic(Giver, "Giver", sizeof(GiverStack) / sizeof(GiverStack[0]), nullptr, 1, GiverStack, &GiverTask);

    vTaskStartScheduler();
}