/**
 * @file app_lab_3_2.cpp
 * @brief Приложение лабы 3.2: запуск задач, отчёт по всем сигналам цепочки, команды.
 *
 * Serial (printf) — видно действие каждого шага:
 *   [   12.0 s] raw=1023 | mV=5000 | sat=1750 | med= 742 | flt= 742 | T= 24.2 C | NORMAL | SAT | NOISE
 * LCD 16x2 (fprintf):
 *   T= 24.2C NORMAL
 *   U5000 F 742 mV *      (U — напряжение до фильтров, F — после; * — помехи включены)
 */
#include "app_lab_3_2.h"
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

#define MS_PER_SECOND     1000UL
#define MS_PER_DECISEC    100UL
#define CMD_TOGGLE_NOISE  'n'
#define CMD_HELP          'h'

static void app_lab_3_2_print_help(void)
{
    printf_P(PSTR("Commands: 'n' - test noise on/off (salt&pepper + jitter), 'h' - help\n"));
    printf_P(PSTR("Columns: raw ADC | mV | sat=saturated | med=median | flt=weighted avg | T\n"));
}

/** Команды из Serial Monitor (читаем только если есть данные — задача не блокируется). */
static void app_lab_3_2_handle_commands(void)
{
    while (srv_serial_stdio_available() > 0) {
        int command = getchar();                       // STDIO-ввод
        if (command == CMD_TOGGLE_NOISE) {
            bool enabled = !dd_sns_temperature_get_test_noise();
            dd_sns_temperature_set_test_noise(enabled);
            printf_P(PSTR(">>> test noise %S\n"), enabled ? PSTR("ON") : PSTR("OFF"));
        } else if (command == CMD_HELP) {
            app_lab_3_2_print_help();
        }
    }
}

/** Отчёт в Serial: все сигналы цепочки кондиционирования. */
static void app_lab_3_2_print_serial(const dd_sns_temperature_signals_t *s,
                                     const char *temperature_text, app_alert_state_t state)
{
    unsigned long uptime = app_uptime_ms();
    printf_P(PSTR("[%5lu.%lu s] raw=%4d | mV=%4d | sat=%4d | med=%4d | flt=%4d | T=%5s C | %S"),
             uptime / MS_PER_SECOND, (uptime % MS_PER_SECOND) / MS_PER_DECISEC,
             s->raw, s->voltage_mv, s->saturated_mv, s->median_mv, s->filtered_mv,
             temperature_text, app_temp_alert_state_name(state));
    if (s->is_saturated) {
        printf_P(PSTR(" | SAT"));
    }
    if (s->test_noise) {
        printf_P(PSTR(" | NOISE"));
    }
    printf_P(PSTR("\n"));
}

/** Отчёт на LCD: температура и состояние, напряжение до и после фильтров. */
static void app_lab_3_2_print_lcd(const dd_sns_temperature_signals_t *s,
                                  const char *temperature_text, app_alert_state_t state)
{
    FILE *lcd = srv_lcd_stdio_stream();
    srv_lcd_stdio_set_cursor(0, 0);
    fprintf_P(lcd, PSTR("T=%5sC %-7S"), temperature_text, app_temp_alert_state_name(state));
    srv_lcd_stdio_set_cursor(0, 1);
    fprintf_P(lcd, PSTR("U%4d F%4d mV %c"), s->voltage_mv, s->filtered_mv,
              s->test_noise ? '*' : ' ');
}

static void app_lab_3_2_report_task(void *parameters)
{
    (void)parameters;
    unsigned int report_counter = 0;

    vTaskDelay(pdMS_TO_TICKS(APP_LAB_3_2_REPORT_OFFSET_MS));
    TickType_t last_wake_time = xTaskGetTickCount();

    for (;;) {
        app_lab_3_2_handle_commands();

        dd_sns_temperature_signals_t s;
        dd_sns_temperature_get_signals(&s);            // согласованный снимок всех сигналов
        app_alert_state_t state = app_temp_alert_get_state();

        char temperature_text[APP_DECI_TEXT_SIZE];
        app_format_deci(temperature_text, sizeof(temperature_text), s.temperature_deci);

        app_lab_3_2_print_serial(&s, temperature_text, state);
        app_lab_3_2_print_lcd(&s, temperature_text, state);

        if (++report_counter >= APP_LAB_3_2_DIAG_EVERY) {
            report_counter = 0;
            app_print_stack_diagnostics();
        }

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(APP_LAB_3_2_REPORT_REC_MS));
    }
}

void app_lab_3_2_setup(void)
{
    srv_serial_stdio_setup();
    srv_lcd_stdio_setup();

    printf_P(PSTR("\n=== Lab 3.2: Signal conditioning - TMP36 temperature (A0) ===\n"));
    printf_P(PSTR("raw -> mV -> saturation %d..%d mV -> median(%d) -> weighted avg(50/25/15/10%%) -> T\n"),
             DD_SNS_TEMP_SAT_MV_MIN, DD_SNS_TEMP_SAT_MV_MAX, DD_SNS_TEMP_MEDIAN_SIZE);
    app_lab_3_2_print_help();
    fprintf_P(srv_lcd_stdio_stream(), PSTR("\fLab 3.2 TMP36\nstarting..."));

    srv_heartbeat_setup();
    dd_sns_temperature_task_setup();
    app_temp_alert_setup(dd_sns_temperature_get_temperature);   // тревога — по отфильтрованной T
    BaseType_t created = xTaskCreate(app_lab_3_2_report_task, "Report", APP_LAB_3_2_REPORT_STACK,
                                     NULL, APP_LAB_3_2_REPORT_PRIORITY, NULL);
    if (created != pdPASS) {
        printf_P(PSTR("ERROR: not enough RAM to create Report task\n"));
    }
    app_print_free_ram();
}

void app_lab_3_2_loop(void)
{
    // Всё работает в задачах FreeRTOS.
}
