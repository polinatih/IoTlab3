/**
 * @file app_lab_3_2.h
 * @brief Лабораторная 3.2 — Датчики: кондиционирование сигнала (TMP36).
 *
 * Цепочка: raw -> мВ -> насыщение 100..1750 мВ -> «соль и перец» (медиана 5)
 *          -> взвешенное среднее (50/25/15/10 %) -> °C -> насыщение -40..+125 °C.
 *
 * Задачи FreeRTOS — как в 3.1; задача отчёта дополнительно принимает команды:
 *   n — включить/выключить тестовые помехи (для демонстрации фильтров)
 *   h — подсказка
 */
#ifndef APP_LAB_3_2_H
#define APP_LAB_3_2_H

#define APP_LAB_3_2_REPORT_REC_MS     500
#define APP_LAB_3_2_REPORT_OFFSET_MS  100
#define APP_LAB_3_2_REPORT_STACK      208
#define APP_LAB_3_2_REPORT_PRIORITY   1
#define APP_LAB_3_2_DIAG_EVERY        20

void app_lab_3_2_setup(void);
void app_lab_3_2_loop(void);

#endif // APP_LAB_3_2_H
