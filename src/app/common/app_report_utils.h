/**
 * @file app_report_utils.h
 * @brief Вспомогательные функции для отчётов printf.
 */
#ifndef APP_REPORT_UTILS_H
#define APP_REPORT_UTILS_H

#include <stdint.h>
#include <stddef.h>

#define APP_DECI_TEXT_SIZE  8   // "-123.4" + '\0'

/** Значение в десятых долях -> текст "123.4" (printf на AVR не умеет %f). */
void app_format_deci(char *buffer, size_t size, int deci_value);

/** Время работы системы в мс (по тикам FreeRTOS). */
unsigned long app_uptime_ms(void);

/**
 * Сколько куча ещё может выдать (байт). Вызывать в конце setup():
 * FreeRTOS при старте планировщика берёт ещё ~480 байт (задачи Idle и Timer).
 */
void app_print_free_ram(void);

/** Строка диагностики: свободный стек (байт) каждой задачи. */
void app_print_stack_diagnostics(void);

#endif // APP_REPORT_UTILS_H
