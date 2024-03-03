#include <iterator>
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

static StaticSemaphore_t SuccessMutexBuffer;
static SemaphoreHandle_t SuccessMutex;

[[noreturn]] void LockSuccessProc(void*)
{
    while(true)
    {
        xSemaphoreTake(SuccessMutex, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(100));
        xSemaphoreGive(SuccessMutex);
    }
}

static StackType_t LockSuccessStack[512];
static StaticTask_t LockSuccessTask;

static StaticSemaphore_t FailMutexBuffer;
static SemaphoreHandle_t FailMutex;

[[noreturn]] void LockFailProc(void*)
{
    while(true)
    {
        xSemaphoreTake(FailMutex, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(100));
        xSemaphoreTake(FailMutex, pdMS_TO_TICKS(50)); // will fail
        xSemaphoreGive(FailMutex);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

static StackType_t LockFailStack[512];
static StaticTask_t LockFailTask;

void demo_main()
{
    SuccessMutex = xSemaphoreCreateMutexStatic(&SuccessMutexBuffer);
    FailMutex = xSemaphoreCreateMutexStatic(&FailMutexBuffer);

    xTaskCreateStatic(
        LockSuccessProc, "LockSuccess", std::size(LockSuccessStack), nullptr, configMAX_PRIORITIES - 1, LockSuccessStack, &LockSuccessTask);
    xTaskCreateStatic(LockFailProc, "LockFail", std::size(LockFailStack), nullptr, configMAX_PRIORITIES - 1, LockFailStack, &LockFailTask);

    vTaskStartScheduler();
}