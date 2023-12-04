#include <iterator>
#include "FreeRTOS.h"
#include "platform.hpp"
#include "queue.h"
#include "task.h"

static void BusyDelay(portTickType delay)
{
    auto delayTo = xTaskGetTickCount() + delay;
    while(xTaskGetTickCount() < delayTo)
    {
        __asm__ volatile("nop");
    }
}

static StaticQueue_t FillingQueueBuffer;
static int FillingQueueBufferStorage[10];
static QueueHandle_t FillingQueue;

static void PeriodicFiller(void*)
{
    while(true)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));

        for(int i = 0; i < 10; i++)
        {
            int x = 0;
            xQueueSendToBack(FillingQueue, &x, portMAX_DELAY);
        }
    }
}

static StackType_t PeriodicFillerStack[256];
static StaticTask_t PeriodicFillerTask;

static void Consumer(void*)
{
    while(true)
    {
        int x;
        xQueueReceive(FillingQueue, &x, portMAX_DELAY);
        BusyDelay(5);
    }
}

static StackType_t ConsumerStack[256];
static StaticTask_t ConsumerTask;

static StaticQueue_t ConsumingQueueBuffer;
static int ConsumingQueueBufferStorage[10];
static QueueHandle_t ConsumingQueue;

static void PeriodicConsumer(void*)
{
    while(true)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));

        for(int i = 0; i < 10; i++)
        {
            int x;
            xQueueReceive(ConsumingQueue, &x, 0);
        }
    }
}

static StackType_t PeriodicConsumerStack[256];
static StaticTask_t PeriodicConsumerTask;

static void Producer(void*)
{
    while(true)
    {
        int x = 0;
        xQueueSendToBack(ConsumingQueue, &x, portMAX_DELAY);
        BusyDelay(5);
    }
}

static StackType_t ProducerStack[256];
static StaticTask_t ProducerTask;

void demo_main()
{
    FillingQueue = xQueueCreateStatic(
        std::size(FillingQueueBufferStorage), sizeof(int), reinterpret_cast<std::uint8_t*>(FillingQueueBufferStorage), &FillingQueueBuffer);

    xTaskCreateStatic(
        PeriodicFiller, "PeriodicFiller", std::size(PeriodicFillerStack), nullptr, configMAX_PRIORITIES - 1, PeriodicFillerStack, &PeriodicFillerTask);
    xTaskCreateStatic(Consumer, "Consumer", std::size(ConsumerStack), nullptr, 1, ConsumerStack, &ConsumerTask);

    ConsumingQueue = xQueueCreateStatic(
        std::size(ConsumingQueueBufferStorage), sizeof(int), reinterpret_cast<std::uint8_t*>(ConsumingQueueBufferStorage), &ConsumingQueueBuffer);

    xTaskCreateStatic(
            IsrGiveTaskReceiveSemaphore_Receiver, "Receiver",
            std::size(PeriodicConsumerStack), nullptr, configMAX_PRIORITIES - 1, PeriodicConsumerStack,
            &PeriodicConsumerTask);
    xTaskCreateStatic(IsrGiveTaskReceiveSemaphore_Giver, "Giver", std::size(ProducerStack),
                      nullptr, 1, ProducerStack, &ProducerTask);

    vTaskStartScheduler();
}