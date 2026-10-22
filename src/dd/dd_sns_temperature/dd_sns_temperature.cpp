/**
 * @file dd_sns_temperature.cpp
 * @brief Реализация драйвера TMP36: съём + кондиционирование сигнала.
 */
#include "dd_sns_temperature.h"
#include "../../lib/lib_cond/lib_cond.h"
#include <Arduino_FreeRTOS.h>
#include <stdlib.h>

#define PERCENT_MAX 100

// ---------------------------------------------------------------- Сигналы
static dd_sns_temperature_signals_t signals;   // опубликованные значения (для getters)
static volatile bool test_noise_enabled = false;

// ---------------------------------------------------------------- Буферы фильтров
static int  median_buffer[DD_SNS_TEMP_MEDIAN_SIZE];       // история для медианы
static int  median_buffer_copy[DD_SNS_TEMP_MEDIAN_SIZE];  // копия для сортировки
static int  wavg_buffer[DD_SNS_TEMP_WAVG_SIZE];           // история для среднего
static const int wavg_weights[DD_SNS_TEMP_WAVG_SIZE] = { 50, 25, 15, 10 }; // %, новейший первый
static bool buffers_ready = false;

/**
 * Конверсия напряжение -> температура по характеристике TMP36:
 *   T[°C] = (U - 500 мВ) / 10 мВ  ->  T[0.1 °C] = (U - 500) * 10 / 10
 */
static int dd_sns_temperature_mv_to_deci(int millivolts)
{
    return (int)(((long)millivolts - DD_SNS_TEMP_ZERO_MV) * DD_SNS_TEMP_DECI_PER_DEG
                 / DD_SNS_TEMP_MV_PER_DEG);
}

/**
 * Тестовые помехи (только для демонстрации фильтров):
 * импульс «соль» (1023) / «перец» (0) или мелкий шум ±JITTER.
 */
static int dd_sns_temperature_inject_noise(int raw)
{
    if ((random() % PERCENT_MAX) < DD_SNS_TEMP_NOISE_PERCENT) {
        return (random() & 1) ? DD_SNS_TEMP_RAW_MAX : DD_SNS_TEMP_RAW_MIN;
    }
    int jitter = (int)(random() % (2 * DD_SNS_TEMP_NOISE_JITTER + 1)) - DD_SNS_TEMP_NOISE_JITTER;
    return lib_cond_saturate_int(raw + jitter, DD_SNS_TEMP_RAW_MIN, DD_SNS_TEMP_RAW_MAX);
}

void dd_sns_temperature_setup(void)
{
    pinMode(DD_SNS_TEMP_PIN, INPUT);   // аналоговый вход, без подтяжки
    buffers_ready = false;             // буферы заполним первым измерением
}

void dd_sns_temperature_loop(void)
{
    dd_sns_temperature_signals_t s;    // считаем локально, публикуем в конце

    // 1. Сбор данных (Collect raw value): hardware -> software
    s.raw = analogRead(DD_SNS_TEMP_PIN);
    s.test_noise = test_noise_enabled;
    if (s.test_noise) {
        s.raw = dd_sns_temperature_inject_noise(s.raw);
    }

    // 2. Конверсия digital -> voltage (линейная, уравнение прямой) + смещение
    s.voltage_mv = (int)map(s.raw,
                            DD_SNS_TEMP_RAW_MIN, DD_SNS_TEMP_RAW_MAX,
                            DD_SNS_TEMP_MV_MIN, DD_SNS_TEMP_MV_MAX)
                   + DD_SNS_TEMP_MV_OFFSET;

    // Температура без фильтров — для лабы 3.1 (только съём и масштабирование)
    s.temperature_unfiltered_deci = dd_sns_temperature_mv_to_deci(s.voltage_mv);

    // 3. Насыщение: всё вне рабочего диапазона датчика срезается к границе
    s.saturated_mv = lib_cond_saturate_int(s.voltage_mv,
                                           DD_SNS_TEMP_SAT_MV_MIN, DD_SNS_TEMP_SAT_MV_MAX);
    s.is_saturated = (s.saturated_mv != s.voltage_mv);

    // Первый запуск: заполняем историю текущим значением (без «разгона» с нуля)
    if (!buffers_ready) {
        lib_cond_buf_fill_int(median_buffer, DD_SNS_TEMP_MEDIAN_SIZE, s.saturated_mv);
        lib_cond_buf_fill_int(wavg_buffer, DD_SNS_TEMP_WAVG_SIZE, s.saturated_mv);
        buffers_ready = true;
    }

    // 4. Фильтр «соль и перец» (медианный)
    lib_cond_fifo_push_int(median_buffer, DD_SNS_TEMP_MEDIAN_SIZE, s.saturated_mv);    // 4.1 буферизация
    lib_cond_buf_copy_int(median_buffer, median_buffer_copy, DD_SNS_TEMP_MEDIAN_SIZE); // 4.2 копия
    lib_cond_buf_sort_int(median_buffer_copy, DD_SNS_TEMP_MEDIAN_SIZE);              // 4.3 сортировка
    s.median_mv = lib_cond_median_int(median_buffer_copy, DD_SNS_TEMP_MEDIAN_SIZE);   // 4.4 медиана

    // 5. Взвешенное скользящее среднее (фильтр нижних частот)
    lib_cond_fifo_push_int(wavg_buffer, DD_SNS_TEMP_WAVG_SIZE, s.median_mv);          // 5.1 буферизация
    s.filtered_mv = lib_cond_weighted_average_int(wavg_buffer, wavg_weights,
                                                  DD_SNS_TEMP_WAVG_SIZE);            // 5.2 среднее

    // 6. Конверсия voltage -> physical (°C) + насыщение температуры
    s.temperature_deci = lib_cond_saturate_int(dd_sns_temperature_mv_to_deci(s.filtered_mv),
                                               DD_SNS_TEMP_DECI_MIN, DD_SNS_TEMP_DECI_MAX);

    // Публикация: копия структуры в критической секции — другие задачи
    // никогда не увидят «половину» обновления (на AVR int — это 2 байта).
    taskENTER_CRITICAL();
    s.sample_count = signals.sample_count + 1;
    signals = s;
    taskEXIT_CRITICAL();
}

// ---------------------------------------------------------------- Getters
void dd_sns_temperature_get_signals(dd_sns_temperature_signals_t *snapshot)
{
    taskENTER_CRITICAL();
    *snapshot = signals;
    taskEXIT_CRITICAL();
}

/** Атомарное чтение одного поля типа int. */
static int dd_sns_temperature_read_int(const int *field)
{
    taskENTER_CRITICAL();
    int value = *field;
    taskEXIT_CRITICAL();
    return value;
}

int dd_sns_temperature_get_raw(void)        { return dd_sns_temperature_read_int(&signals.raw); }
int dd_sns_temperature_get_voltage(void)    { return dd_sns_temperature_read_int(&signals.voltage_mv); }
int dd_sns_temperature_get_saturated(void)  { return dd_sns_temperature_read_int(&signals.saturated_mv); }
int dd_sns_temperature_get_median(void)     { return dd_sns_temperature_read_int(&signals.median_mv); }
int dd_sns_temperature_get_filtered(void)   { return dd_sns_temperature_read_int(&signals.filtered_mv); }
int dd_sns_temperature_get_temperature(void)
{
    return dd_sns_temperature_read_int(&signals.temperature_deci);
}
int dd_sns_temperature_get_temperature_unfiltered(void)
{
    return dd_sns_temperature_read_int(&signals.temperature_unfiltered_deci);
}

void dd_sns_temperature_set_test_noise(bool enabled)
{
    test_noise_enabled = enabled;
}

bool dd_sns_temperature_get_test_noise(void)
{
    return test_noise_enabled;
}
