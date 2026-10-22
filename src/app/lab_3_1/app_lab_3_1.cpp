/**
 * @file app_lab_3_1.cpp
 * @brief Приложение лабы 3.1: запуск задач и периодический отчёт через STDIO.
 *
 * Serial (printf):
 *   [   12.0 s] #0250 | raw= 152 | U= 742 mV | T= 24.2 C | state=NORMAL
 * LCD 16x2 (fprintf):
 *   T= 24.2C NORMAL
 *   ADC 152   742mV
 */
#include "app_lab_3_1.h"
#include "../common/app_temp_alert.h"
#include "../common/app_report_utils.h"
#include "../../srv/srv_serial_stdio/srv_serial_stdio.h"
#include "../../srv/srv_lcd_stdio/srv_lcd_stdio.h"
#include "../../srv/srv_heartbeat/srv_heartbeat.h"
#include "../../dd/dd_sns_temperature/dd_sns_temperature.h"
#include "../../dd/dd_sns_temperature/dd_sns_temperature_task.h"
#include <Arduino_FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#define MS_PER_SECOND   1000UL
#define MS_PER_DECISEC  100UL

/** Отчёт в Serial. */
static void app_lab_3_1_print_serial(const dd_sns_temperature_signals_t *s,
                                     const char *temperature_text, app_alert_state_t state)
{
    unsigned long uptime = app_uptime_ms();
    printf_P(PSTR("[%5lu.%lu s] #%04lu | raw=%4d | U=%4d mV | T=%5s C | state=%S\n"),
             uptime / MS_PER_SECOND, (uptime % MS_PER_SECOND) / MS_PER_DECISEC,
             s->sample_count, s->raw, s->voltage_mv, temperature_text,
             app_temp_alert_state_name(state));

    if (state == APP_ALERT_WARNING) {
        printf_P(PSTR("            !!! WARNING: overheating (on at >= %d C, off below %d C)\n"),
                 APP_ALERT_WARNING_DECI / DD_SNS_TEMP_DECI_PER_DEG,
                 (APP_ALERT_WARNING_DECI - APP_ALERT_HYSTERESIS_DECI) / DD_SNS_TEMP_DECI_PER_DEG);
    }
}

/** Отчёт на LCD: пишем поверх старого текста (без clear — нет мерцания). */
static void app_lab_3_1_print_lcd(const dd_sns_temperature_signals_t *s,
                                  const char *temperature_text, app_alert_state_t state)
{
    FILE *lcd = srv_lcd_stdio_stream();
    srv_lcd_stdio_set_cursor(0, 0);
    fprintf_P(lcd, PSTR("T=%5sC %-7S"), temperature_text, app_temp_alert_state_name(state));
    srv_lcd_stdio_set_cursor(0, 1);
    fprintf_P(lcd, PSTR("ADC%4d  %4dmV "), s->raw, s->voltage_mv);
}

static void app_lab_3_1_report_task(void *parameters)
{
    (void)parameters;
    unsigned int report_counter = 0;

    vTaskDelay(pdMS_TO_TICKS(APP_LAB_3_1_REPORT_OFFSET_MS));
    TickType_t last_wake_time = xTaskGetTickCount();

    for (;;) {
        // Consumer: берём согласованный снимок сигналов драйвера
        dd_sns_temperature_signals_t s;
        dd_sns_temperature_get_signals(&s);
        app_alert_state_t state = app_temp_alert_get_state();

        char temperature_text[APP_DECI_TEXT_SIZE];
        app_format_deci(temperature_text, sizeof(temperature_text), s.temperature_unfiltered_deci);

        app_lab_3_1_print_serial(&s, temperature_text, state);
        app_lab_3_1_print_lcd(&s, temperature_text, state);

        if (++report_counter >= APP_LAB_3_1_DIAG_EVERY) {
            report_counter = 0;
            app_print_stack_diagnostics();
        }

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(APP_LAB_3_1_REPORT_REC_MS));
    }
}

void app_lab_3_1_setup(void)
{
    srv_serial_stdio_setup();
    srv_lcd_stdio_setup();

    printf_P(PSTR("\n=== Lab 3.1: Sensor signal acquisition - TMP36 temperature (A0) ===\n"));
    printf_P(PSTR("raw 0..1023 -> U 0..5000 mV -> T = (U - 500) / 10 C; WARNING at >= %d C\n"),
             APP_ALERT_WARNING_DECI / DD_SNS_TEMP_DECI_PER_DEG);
    fprintf_P(srv_lcd_stdio_stream(), PSTR("\fLab 3.1 TMP36\nstarting..."));

    // Запускаем «программы» параллельно — каждая в своей задаче FreeRTOS
    srv_heartbeat_setup();
    dd_sns_temperature_task_setup();
    app_temp_alert_setup(dd_sns_temperature_get_temperature_unfiltered);
    BaseType_t created = xTaskCreate(app_lab_3_1_report_task, "Report", APP_LAB_3_1_REPORT_STACK,
                                     NULL, APP_LAB_3_1_REPORT_PRIORITY, NULL);
    if (created != pdPASS) {
        printf_P(PSTR("ERROR: not enough RAM to create Report task\n"));
    }
    app_print_free_ram();
}

void app_lab_3_1_loop(void)
{
    // Всё работает в задачах FreeRTOS.
}
