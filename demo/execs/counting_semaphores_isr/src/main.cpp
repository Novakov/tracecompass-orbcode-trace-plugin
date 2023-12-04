#include <iterator>
#include "FreeRTOS.h"
#include "interrupts.hpp"
#include "platform.hpp"
#include "semphr.h"
#include "task.h"
#include "trace.hpp"

namespace isr_give_task_receive
{
    static StaticSemaphore_t SemaphoreBuffer;
    static SemaphoreHandle_t Semaphore;

    static void Receiver(void*)
    {
        while(true)
        {
            vTaskDelay(pdMS_TO_TICKS(1500));

            for(int i = 0; i < 12; i++)
            {
                xSemaphoreTake(Semaphore, 2);
            }
        }
    }

    static StackType_t ReceiverStack[256];
    static StaticTask_t ReceiverTask;

    extern "C" void Interrupt1_IRQHandler()
    {
        portBASE_TYPE yielded = false;
        xSemaphoreGiveFromISR(Semaphore, &yielded);
        NVIC_ClearPendingIRQ(Interrupt1_IRQn);
        portYIELD_FROM_ISR(yielded);
    }

    static void Giver(void*)
    {
        NVIC_SetPriority(Interrupt1_IRQn, 7);
        NVIC_EnableIRQ(Interrupt1_IRQn);

        while(true)
        {
            NVIC_SetPendingIRQ(Interrupt1_IRQn);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }

    static StackType_t GiverStack[256];
    static StaticTask_t GiverTask;
}

namespace isr_receive_task_give
{
    static StaticSemaphore_t SemaphoreBuffer;
    static SemaphoreHandle_t Semaphore;

    static void Giver(void*)
    {
        while(true)
        {
            xQueueSendToBack(Semaphore, nullptr, 2);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }

    static StackType_t GiverStack[256];
    static StaticTask_t GiverTask;

    extern "C" void Interrupt2_IRQHandler()
    {
        portBASE_TYPE yielded = false;
        xSemaphoreTakeFromISR(Semaphore, &yielded);
        NVIC_ClearPendingIRQ(Interrupt2_IRQn);
        portYIELD_FROM_ISR(yielded);
    }

    static void Receiver(void*)
    {
        NVIC_SetPriority(Interrupt2_IRQn, 7);
        NVIC_EnableIRQ(Interrupt2_IRQn);

        while(true)
        {
            vTaskDelay(pdMS_TO_TICKS(1500));

            for(int i = 0; i < 12; i++)
            {
                NVIC_SetPendingIRQ(Interrupt2_IRQn);
            }
        }
    }

    static StackType_t ReceiverStack[256];
    static StaticTask_t ReceiverTask;

}

void demo_main()
{
    isr_give_task_receive::Semaphore = xSemaphoreCreateCountingStatic(10, 0, &isr_give_task_receive::SemaphoreBuffer);

    xTaskCreateStatic(
        isr_give_task_receive::Receiver,
        "IGTR_R",
        std::size(isr_give_task_receive::ReceiverStack),
        nullptr,
        configMAX_PRIORITIES - 1,
        isr_give_task_receive::ReceiverStack,
        &isr_give_task_receive::ReceiverTask);
    xTaskCreateStatic(
        isr_give_task_receive::Giver,
        "IGTR_G",
        std::size(isr_give_task_receive::GiverStack),
        nullptr,
        1,
        isr_give_task_receive::GiverStack,
        &isr_give_task_receive::GiverTask);

    isr_receive_task_give::Semaphore = xSemaphoreCreateCountingStatic(10, 0, &isr_receive_task_give::SemaphoreBuffer);

    xTaskCreateStatic(
        isr_receive_task_give::Giver,
        "IRTG_G",
        std::size(isr_receive_task_give::GiverStack),
        nullptr,
        1,
        isr_receive_task_give::GiverStack,
        &isr_receive_task_give::GiverTask);
    xTaskCreateStatic(
        isr_receive_task_give::Receiver,
        "IRTG_R",
        std::size(isr_receive_task_give::ReceiverStack),
        nullptr,
        configMAX_PRIORITIES - 1,
        isr_receive_task_give::ReceiverStack,
        &isr_receive_task_give::ReceiverTask);

    vTaskStartScheduler();
}