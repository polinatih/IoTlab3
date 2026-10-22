/**
 * @file lib_cond.h
 * @brief Библиотека кондиционирования сигнала (целочисленная, без float).
 *
 * Строительные блоки фильтров, которые использует драйвер датчика:
 *   - FIFO-буфер истории:   lib_cond_fifo_push_int()   (новейший — в [0])
 *   - копия буфера:         lib_cond_buf_copy_int()    (чтобы не испортить историю)
 *   - сортировка:           lib_cond_buf_sort_int()    (qsort)
 *   - медиана:              lib_cond_median_int()      -> фильтр «соль и перец»
 *   - взвешенное среднее:   lib_cond_weighted_average_int() -> фильтр нижних частот
 *   - насыщение:            lib_cond_saturate_int()
 *
 * Модуль не знает ничего о конкретном датчике — его можно переиспользовать
 * для любого сигнала (угол, температура, расстояние...).
 */
#ifndef LIB_COND_H
#define LIB_COND_H

#include <stdint.h>

/** Заполнить весь буфер одним значением (начальная инициализация истории). */
void lib_cond_buf_fill_int(int *buffer, uint8_t size, int value);

/** Сдвинуть буфер на одну позицию и записать новое значение в [0]. */
void lib_cond_fifo_push_int(int *buffer, uint8_t size, int value);

/** Скопировать size элементов из source в destination. */
void lib_cond_buf_copy_int(const int *source, int *destination, uint8_t size);

/** Отсортировать буфер по возрастанию (qsort). */
void lib_cond_buf_sort_int(int *buffer, uint8_t size);

/** Медиана УЖЕ отсортированного буфера — элемент из середины. */
int lib_cond_median_int(const int *sorted_buffer, uint8_t size);

/**
 * Взвешенное среднее: Σ(buffer[i] * weights[i]) / Σ(weights[i]), с округлением.
 * buffer[0] — новейшее значение, weights[0] — его вес.
 */
int lib_cond_weighted_average_int(const int *buffer, const int *weights, uint8_t size);

/** Насыщение: ограничить value отрезком [min_value, max_value]. */
int lib_cond_saturate_int(int value, int min_value, int max_value);

#endif // LIB_COND_H
