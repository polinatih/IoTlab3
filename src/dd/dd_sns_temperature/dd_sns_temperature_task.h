/**
 * @file dd_sns_temperature_task.h
 * @brief Обёртка FreeRTOS: драйвер датчика температуры работает в отдельной задаче.
 *
 * Задача выполняет dd_sns_temperature_loop() строго периодически:
 *   vTaskDelay(OFFSET)      — смещение старта относительно других задач;
 *   vTaskDelayUntil(REC)    — период (recurrence) без накопления ошибки.
 */
#ifndef DD_SNS_TEMPERATURE_TASK_H
#define DD_SNS_TEMPERATURE_TASK_H

#include <Arduino_FreeRTOS.h>
#include <task.h>

// ВНИМАНИЕ: тик FreeRTOS на Arduino Uno ≈ 16 мс (сторожевой таймер, 62 тика/с).
// pdMS_TO_TICKS() округляет вниз, поэтому времена фактически кратны 16 мс:
// 50 мс -> 3 тика (48 мс), 20 мс -> 1 тик (16 мс). Меньше 16 мс задать нельзя.
#define DD_SNS_TEMP_TASK_REC_MS       50     // ~20 измерений в секунду
#define DD_SNS_TEMP_TASK_OFFSET_MS    20
#define DD_SNS_TEMP_TASK_STACK        104
#define DD_SNS_TEMP_TASK_PRIORITY     2      // выше, чем отчёт: измерения важнее

/** Создаёт задачу датчика. Вызывать в setup() приложения. */
void dd_sns_temperature_task_setup(void);

TaskHandle_t dd_sns_temperature_task_get_handle(void);

#endif // DD_SNS_TEMPERATURE_TASK_H
