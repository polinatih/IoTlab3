/**
 * @file lib_cond.cpp
 * @brief Реализация библиотеки кондиционирования сигнала.
 */
#include "lib_cond.h"
#include <stdlib.h>

void lib_cond_buf_fill_int(int *buffer, uint8_t size, int value)
{
    for (uint8_t i = 0; i < size; i++) {
        buffer[i] = value;
    }
}

void lib_cond_fifo_push_int(int *buffer, uint8_t size, int value)
{
    // Сдвигаем историю: самое старое значение (в конце) теряется
    for (uint8_t i = size - 1; i > 0; i--) {
        buffer[i] = buffer[i - 1];
    }
    buffer[0] = value;          // новейшее значение — в начало
}

void lib_cond_buf_copy_int(const int *source, int *destination, uint8_t size)
{
    for (uint8_t i = 0; i < size; i++) {
        destination[i] = source[i];
    }
}

/** Функция сравнения для qsort: <0 если a<b, 0 если равны, >0 если a>b. */
static int lib_cond_compare_int(const void *a, const void *b)
{
    int value_a = *(const int *)a;
    int value_b = *(const int *)b;
    return (value_a > value_b) - (value_a < value_b);   // без переполнения
}

void lib_cond_buf_sort_int(int *buffer, uint8_t size)
{
    qsort(buffer, size, sizeof(int), lib_cond_compare_int);
}

int lib_cond_median_int(const int *sorted_buffer, uint8_t size)
{
    // Для нечётного size (например 5) это ровно средний элемент: индекс 2
    return sorted_buffer[size / 2];
}

int lib_cond_weighted_average_int(const int *buffer, const int *weights, uint8_t size)
{
    long weighted_sum = 0;      // long (32 бита): 5000 мВ * 100 % не влезает в int (16 бит)
    long weight_sum   = 0;

    for (uint8_t i = 0; i < size; i++) {
        weighted_sum += (long)buffer[i] * weights[i];
        weight_sum   += weights[i];
    }
    if (weight_sum == 0) {
        return buffer[0];       // защита от деления на ноль
    }
    // Округление до ближайшего целого (значения у нас неотрицательные)
    return (int)((weighted_sum + weight_sum / 2) / weight_sum);
}

int lib_cond_saturate_int(int value, int min_value, int max_value)
{
    if (value > max_value) {
        return max_value;       // срезаем сверху
    }
    if (value < min_value) {
        return min_value;       // срезаем снизу
    }
    return value;
}
