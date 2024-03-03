#include <iterator>
#include "FreeRTOS.h"
#include "platform.hpp"
#include "queue.h"
#include "semphr.h"
#include "task.h"

static constexpr std::size_t QueueSize = 16;

static StaticQueue_t QueueBuffer;
static std::uint32_t QueueBufferStorage[QueueSize];
static QueueHandle_t Queue;

static StaticSemaphore_t SemaphoreBuffer;
static SemaphoreHandle_t Semaphore;

[[noreturn]] static void QueueFillerProc(void*)
{
    std::uint32_t x = 0;
    while(true)
    {
        xSemaphoreTake(Semaphore, portMAX_DELAY);
        for(std::size_t i = 0; i < QueueSize; i++)
        {
            xQueueSendToBack(Queue, &x, portMAX_DELAY);
            x++;
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }
}

[[noreturn]] static void BlinkerProc(void*)
{
    while(true)
    {
        for(std::size_t i = 0; i < QueueSize; i++)
        {
            std::uint32_t x;
            xQueueReceive(Queue, &x, portMAX_DELAY);
            vTaskDelay(pdMS_TO_TICKS(50));
        }

        ToggleLed(Led::Led1);

        xSemaphoreGive(Semaphore);
    }
}

static StaticTask_t QueueFillerTcb;
static StackType_t QueueFillerStack[256];

static StaticTask_t BlinkerTcb;
static StackType_t BlinkerStack[256];

void demo_main()
{
    Queue = xQueueCreateStatic(QueueSize, sizeof(std::uint32_t), reinterpret_cast<std::uint8_t*>(QueueBufferStorage), &QueueBuffer);
    Semaphore = xSemaphoreCreateBinaryStatic(&SemaphoreBuffer);
    xSemaphoreGive(Semaphore);

    xTaskCreateStatic(
        QueueFillerProc, "QueueFiller", std::size(QueueFillerStack), nullptr, configMAX_PRIORITIES - 1, QueueFillerStack, &QueueFillerTcb);
    xTaskCreateStatic(BlinkerProc, "Blinker", std::size(BlinkerStack), nullptr, configMAX_PRIORITIES - 2, BlinkerStack, &BlinkerTcb);

    vTaskStartScheduler();
}