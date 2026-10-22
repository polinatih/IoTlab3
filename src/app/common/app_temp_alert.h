/**
 * @file app_temp_alert.h
 * @brief Управление сигналами состояния: предупреждение при перегреве.
 *
 * «Дополнительное поведение» лабораторной: если температура >= 30.0 °C,
 * система переходит в состояние WARNING (красный LED), иначе NORMAL (зелёный).
 * Гистерезис 1 °C: назад в NORMAL только когда T < 29.0 °C — так состояние
 * не «дребезжит», когда температура колеблется около порога.
 */
#ifndef APP_TEMP_ALERT_H
#define APP_TEMP_ALERT_H

#include <Arduino_FreeRTOS.h>
#include <task.h>
#include <avr/pgmspace.h>

#define APP_ALERT_LED_NORMAL_PIN     6      // зелёный
#define APP_ALERT_LED_WARNING_PIN    7      // красный
#define APP_ALERT_WARNING_DECI       300    // 30.0 °C
#define APP_ALERT_HYSTERESIS_DECI    10     // 1.0 °C

#define APP_ALERT_TASK_REC_MS        100
#define APP_ALERT_TASK_OFFSET_MS     40     // после задачи датчика
#define APP_ALERT_TASK_STACK         80
#define APP_ALERT_TASK_PRIORITY      2

typedef enum {
    APP_ALERT_NORMAL = 0,
    APP_ALERT_WARNING
} app_alert_state_t;

/** Источник температуры (0.1 °C): для 3.1 — без фильтров, для 3.2 — после фильтров. */
typedef int (*app_temp_source_t)(void);

void app_temp_alert_setup(app_temp_source_t temperature_source);

app_alert_state_t app_temp_alert_get_state(void);
PGM_P             app_temp_alert_state_name(app_alert_state_t state); ///< строка во Flash, печатать %S
TaskHandle_t      app_temp_alert_get_task_handle(void);

#endif // APP_TEMP_ALERT_H
