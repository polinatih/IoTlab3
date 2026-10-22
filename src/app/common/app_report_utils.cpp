/**
 * @file app_report_utils.cpp
 * @brief Реализация вспомогательных функций отчёта.
 */
#include "app_report_utils.h"
#include "app_temp_alert.h"
#include "../../srv/srv_heartbeat/srv_heartbeat.h"
#include "../../dd/dd_sns_temperature/dd_sns_temperature_task.h"
#include <Arduino_FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#define DECI_DIVIDER 10

void app_format_deci(char *buffer, size_t size, int deci_value)
{
    const char *sign = (deci_value < 0) ? "-" : "";   // знак отдельно: -0.5 не потеряет минус
    unsigned int abs_value = (unsigned int)abs(deci_value);
    snprintf_P(buffer, size, PSTR("%s%u.%u"), sign,
               abs_value / DECI_DIVIDER, abs_value % DECI_DIVIDER);
}

// Границы памяти из avr-libc: начало кучи, текущая вершина кучи и предел роста кучи
extern char  __heap_start;
extern char *__brkval;
extern char *__malloc_heap_end;
extern size_t __malloc_margin;

// Сколько куча должна ещё выдать при старте планировщика:
// задача Idle (стек 192 + TCB) + задача Timer (стек 92 + TCB) + очередь таймеров
#define APP_SCHEDULER_RAM_NEEDED  480

void app_print_free_ram(void)
{
    char  stack_marker;                                 // адрес переменной = текущий стек
    char *heap_top   = (__brkval != 0) ? __brkval : &__heap_start;
    char *heap_limit = (__malloc_heap_end != 0) ? __malloc_heap_end
                                                : &stack_marker - __malloc_margin;
    int   free_heap  = (int)(heap_limit - heap_top);
    printf_P(PSTR("Free heap before scheduler start: %d bytes (FreeRTOS needs ~%d) -> %S\n"),
             free_heap, APP_SCHEDULER_RAM_NEEDED,
             (free_heap >= APP_SCHEDULER_RAM_NEEDED) ? PSTR("OK") : PSTR("NOT ENOUGH RAM!"));
}

unsigned long app_uptime_ms(void)
{
    return (unsigned long)xTaskGetTickCount() * portTICK_PERIOD_MS;
}

static unsigned int app_free_stack(TaskHandle_t handle)
{
    return (handle != NULL) ? (unsigned int)uxTaskGetStackHighWaterMark(handle) : 0;
}

void app_print_stack_diagnostics(void)
{
    printf_P(PSTR("[diag] free stack, bytes: heartbeat=%u sensor=%u alert=%u report=%u\n"),
             app_free_stack(srv_heartbeat_get_task_handle()),
             app_free_stack(dd_sns_temperature_task_get_handle()),
             app_free_stack(app_temp_alert_get_task_handle()),
             app_free_stack(xTaskGetCurrentTaskHandle()));
}
