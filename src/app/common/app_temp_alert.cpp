/**
 * @file app_temp_alert.cpp
 * @brief Реализация задачи состояния (NORMAL / WARNING) с гистерезисом.
 */
#include "app_temp_alert.h"
#include "../../dd/dd_led/dd_led.h"

static app_temp_source_t          temperature_source = NULL;
static volatile app_alert_state_t alert_state = APP_ALERT_NORMAL;   // 1 байт — атомарно
static TaskHandle_t               alert_task_handle = NULL;

/** Новый уровень по температуре с учётом гистерезиса. */
static app_alert_state_t app_temp_alert_evaluate(app_alert_state_t current, int temperature_deci)
{
    if (temperature_deci >= APP_ALERT_WARNING_DECI) {
        return APP_ALERT_WARNING;                                  // порог превышен
    }
    if (current == APP_ALERT_WARNING &&
        temperature_deci >= APP_ALERT_WARNING_DECI - APP_ALERT_HYSTERESIS_DECI) {
        return APP_ALERT_WARNING;                                  // ещё в зоне гистерезиса
    }
    return APP_ALERT_NORMAL;
}

static void app_temp_alert_task(void *parameters)
{
    (void)parameters;

    dd_led_setup(APP_ALERT_LED_NORMAL_PIN);
    dd_led_setup(APP_ALERT_LED_WARNING_PIN);

    vTaskDelay(pdMS_TO_TICKS(APP_ALERT_TASK_OFFSET_MS));
    TickType_t last_wake_time = xTaskGetTickCount();

    for (;;) {
        alert_state = app_temp_alert_evaluate(alert_state, temperature_source());

        // Индикация состояния
        dd_led_set(APP_ALERT_LED_NORMAL_PIN,  alert_state == APP_ALERT_NORMAL);
        dd_led_set(APP_ALERT_LED_WARNING_PIN, alert_state == APP_ALERT_WARNING);

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(APP_ALERT_TASK_REC_MS));
    }
}

void app_temp_alert_setup(app_temp_source_t source)
{
    temperature_source = source;
    xTaskCreate(app_temp_alert_task, "TempAlert", APP_ALERT_TASK_STACK, NULL,
                APP_ALERT_TASK_PRIORITY, &alert_task_handle);
}

app_alert_state_t app_temp_alert_get_state(void)
{
    return alert_state;
}

PGM_P app_temp_alert_state_name(app_alert_state_t state)
{
    return (state == APP_ALERT_WARNING) ? PSTR("WARNING") : PSTR("NORMAL");
}

TaskHandle_t app_temp_alert_get_task_handle(void)
{
    return alert_task_handle;
}
