/**
 * @file app_lab_3_1.h
 * @brief Лабораторная 3.1 — Датчики: снятие сигнала (аналоговый датчик температуры TMP36).
 *
 * Задачи FreeRTOS:
 *   | Задача       | Период | Смещение | Приоритет | Роль                                |
 *   |--------------|--------|----------|-----------|-------------------------------------|
 *   | Heartbeat    | 500 мс |   0 мс   |     1     | LED D13 мигает — система жива       |
 *   | TempSensor   |  50 мс |  20 мс   |     2     | analogRead -> мВ -> °C              |
 *   | TempAlert    | 100 мс |  40 мс   |     2     | NORMAL/WARNING, LED D6/D7           |
 *   | Report       | 500 мс | 100 мс   |     1     | отчёт printf в Serial и на LCD      |
 */
#ifndef APP_LAB_3_1_H
#define APP_LAB_3_1_H

#define APP_LAB_3_1_REPORT_REC_MS     500
#define APP_LAB_3_1_REPORT_OFFSET_MS  100
#define APP_LAB_3_1_REPORT_STACK      208
#define APP_LAB_3_1_REPORT_PRIORITY   1
#define APP_LAB_3_1_DIAG_EVERY        20     // строка диагностики раз в 10 с

void app_lab_3_1_setup(void);
void app_lab_3_1_loop(void);

#endif // APP_LAB_3_1_H
