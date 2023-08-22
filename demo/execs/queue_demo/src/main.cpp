#include <iterator>
#include "FreeRTOS.h"
#include "platform.hpp"
#include "queue.h"
#include "task.h"

// Scenario 1:
//  S1T1 - high priority, pushes elements to queue in loop
//  S1T2 - low priority, pops elements from queue in loop

static StaticQueue_t Scenario1QueueHeader;
static QueueHandle_t Scenario1Queue;
static int Scenario1QueueStorage[10];
static void Scenario1Task1Proc(void*)
{
    int value = 0;
    while(true)
    {
        value++;
        xQueueSendToBack(Scenario1Queue, reinterpret_cast<std::uint8_t*>(&value), portMAX_DELAY);

        // simulate generation of next element
        for(int i = 0; i < 10'000; i++)
        {
            __asm__ volatile("nop");
        }
    }
}

static StackType_t Scenario1Task1Stack[256];
static StaticTask_t Scenario1Task1;

static void Scenario1Task2Proc(void*)
{
    while(true)
    {
        std::uint8_t buf[sizeof(int)];
        xQueueReceive(Scenario1Queue, &buf, portMAX_DELAY);

        // simulate processing of each element
        for(int i = 0; i < 10'000; i++)
        {
            __asm__ volatile("nop");
        }
    }
}

static StackType_t Scenario1Task2Stack[256];
static StaticTask_t Scenario1Task2;

void demo_main()
{
    Scenario1Queue = xQueueCreateStatic(
        std::size(Scenario1QueueStorage), sizeof(int), reinterpret_cast<std::uint8_t*>(Scenario1QueueStorage), &Scenario1QueueHeader);

    xTaskCreateStatic(
        Scenario1Task1Proc, "S1T1", std::size(Scenario1Task1Stack), nullptr, configMAX_PRIORITIES - 1, Scenario1Task1Stack, &Scenario1Task1);
    xTaskCreateStatic(Scenario1Task2Proc, "S1T2", std::size(Scenario1Task2Stack), nullptr, 1, Scenario1Task2Stack, &Scenario1Task2);

    vTaskStartScheduler();
}