/**
 * @file dd_sns_temperature_task.cpp
 * @brief Реализация задачи датчика температуры.
 */
#include "dd_sns_temperature_task.h"
#include "dd_sns_temperature.h"

static TaskHandle_t temperature_task_handle = NULL;

static void dd_sns_temperature_task(void *parameters)
{
    (void)parameters;

    dd_sns_temperature_setup();                                  // инициализация драйвера
    vTaskDelay(pdMS_TO_TICKS(DD_SNS_TEMP_TASK_OFFSET_MS));        // смещение (offset)
    TickType_t last_wake_time = xTaskGetTickCount();

    for (;;) {
        dd_sns_temperature_loop();                               // съём + кондиционирование
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(DD_SNS_TEMP_TASK_REC_MS)); // период
    }
}

void dd_sns_temperature_task_setup(void)
{
    xTaskCreate(dd_sns_temperature_task, "TempSensor", DD_SNS_TEMP_TASK_STACK, NULL,
                DD_SNS_TEMP_TASK_PRIORITY, &temperature_task_handle);
}

TaskHandle_t dd_sns_temperature_task_get_handle(void)
{
    return temperature_task_handle;
}
