/**
 * @file srv_heartbeat.cpp
 * @brief Реализация задачи heartbeat.
 */
#include "srv_heartbeat.h"
#include <Arduino.h>

static TaskHandle_t heartbeat_task_handle = NULL;

static void srv_heartbeat_task(void *parameters)
{
    (void)parameters;
    bool led_state = false;

    pinMode(SRV_HEARTBEAT_PIN, OUTPUT);
    vTaskDelay(pdMS_TO_TICKS(SRV_HEARTBEAT_OFFSET_MS));          // смещение (offset)
    TickType_t last_wake_time = xTaskGetTickCount();

    for (;;) {
        led_state = !led_state;
        digitalWrite(SRV_HEARTBEAT_PIN, led_state ? HIGH : LOW);
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(SRV_HEARTBEAT_REC_MS)); // период (recurrence)
    }
}

void srv_heartbeat_setup(void)
{
    xTaskCreate(srv_heartbeat_task, "Heartbeat", SRV_HEARTBEAT_STACK, NULL,
                SRV_HEARTBEAT_PRIORITY, &heartbeat_task_handle);
}

TaskHandle_t srv_heartbeat_get_task_handle(void)
{
    return heartbeat_task_handle;
}
