/**
 * @file dd_sns_temperature.h
 * @brief Драйвер аналогового датчика температуры TMP36.
 *
 * Аппаратная часть: TMP36, 3 вывода (плоской стороной к себе, слева направо):
 *   +Vs -> 5V,  Vout -> A0,  GND -> GND.
 * Характеристика датчика (даташит): Vout = 500 мВ + 10 мВ/°C,
 * диапазон -40..+125 °C  ->  100..1750 мВ.
 *
 * Поток сигналов (каждый шаг — отдельный сигнал с getter-ом):
 *
 *   1 raw          analogRead(A0), 0..1023
 *   2 voltage      raw -> мВ (линейная конверсия map), 0..5000
 *   3 saturated    насыщение мВ до рабочего диапазона датчика 100..1750 мВ
 *   4 median       фильтр «соль и перец»: буфер 5, копия, сортировка, медиана
 *   5 filtered     взвешенное среднее (ФНЧ): веса 50/25/15/10 %
 *   6 temperature  мВ -> °C, в десятых долях градуса (253 = 25.3 °C)
 *
 * Для лабы 3.1 есть упрощённый сигнал temperature_unfiltered (raw -> мВ -> °C).
 * Все вычисления — в целых числах (у ATmega328P нет аппаратной плавающей точки).
 */
#ifndef DD_SNS_TEMPERATURE_H
#define DD_SNS_TEMPERATURE_H

#include <Arduino.h>
#include <stdint.h>

// ---------------------------------------------------------------- Аппаратная часть
#define DD_SNS_TEMP_PIN               A0

// ---------------------------------------------------------------- Конверсия raw -> мВ
#define DD_SNS_TEMP_RAW_MIN           0
#define DD_SNS_TEMP_RAW_MAX           1023
#define DD_SNS_TEMP_MV_MIN            0
#define DD_SNS_TEMP_MV_MAX            5000
#define DD_SNS_TEMP_MV_OFFSET         0      // калибровочное смещение, мВ

// ---------------------------------------------------------------- Конверсия мВ -> °C (TMP36)
// T[0.1 °C] = (U[мВ] - 500) * 10 / 10  =  U - 500   (10 мВ на 1 °C)
#define DD_SNS_TEMP_DECI_PER_DEG      10
#define DD_SNS_TEMP_ZERO_MV           500    // 0 °C  -> 500 мВ
#define DD_SNS_TEMP_MV_PER_DEG        10     // наклон характеристики

// ---------------------------------------------------------------- Насыщение
#define DD_SNS_TEMP_SAT_MV_MIN        100    // -40 °C
#define DD_SNS_TEMP_SAT_MV_MAX        1750   // +125 °C
#define DD_SNS_TEMP_DECI_MIN          (-400)
#define DD_SNS_TEMP_DECI_MAX          1250

// ---------------------------------------------------------------- Фильтры
#define DD_SNS_TEMP_MEDIAN_SIZE       5      // нечётное — есть точная середина
#define DD_SNS_TEMP_WAVG_SIZE         4      // веса: новейший 50 %, затем 25, 15, 10 %

// ---------------------------------------------------------------- Тестовые помехи
#define DD_SNS_TEMP_NOISE_PERCENT     10     // вероятность импульса «соль/перец», %
#define DD_SNS_TEMP_NOISE_JITTER      2      // мелкий шум ±2 единицы АЦП (≈ ±1 °C)

/** Снимок всех сигналов драйвера (согласованный, на один момент времени). */
typedef struct {
    int  raw;                       ///< 1: сырое значение АЦП
    int  voltage_mv;                ///< 2: напряжение, мВ
    int  saturated_mv;              ///< 3: после насыщения, мВ
    int  median_mv;                 ///< 4: после фильтра «соль и перец», мВ
    int  filtered_mv;               ///< 5: после взвешенного среднего, мВ
    int  temperature_deci;          ///< 6: температура после кондиционирования, 0.1 °C
    int  temperature_unfiltered_deci; ///< температура без фильтров (лаба 3.1), 0.1 °C
    bool is_saturated;              ///< сработало ли насыщение на этом отсчёте
    bool test_noise;                ///< включены ли тестовые помехи
    unsigned long sample_count;     ///< число выполненных измерений
} dd_sns_temperature_signals_t;

void dd_sns_temperature_setup(void);   ///< настройка пина, очистка буферов
void dd_sns_temperature_loop(void);    ///< один цикл: съём + кондиционирование

// ---- Getters (доступ к сигналам только через интерфейс модуля)
int  dd_sns_temperature_get_raw(void);
int  dd_sns_temperature_get_voltage(void);
int  dd_sns_temperature_get_saturated(void);
int  dd_sns_temperature_get_median(void);
int  dd_sns_temperature_get_filtered(void);
int  dd_sns_temperature_get_temperature(void);             ///< после фильтров, 0.1 °C
int  dd_sns_temperature_get_temperature_unfiltered(void);  ///< без фильтров, 0.1 °C
void dd_sns_temperature_get_signals(dd_sns_temperature_signals_t *snapshot);

// ---- Тестовый режим: программные помехи для демонстрации фильтров
void dd_sns_temperature_set_test_noise(bool enabled);
bool dd_sns_temperature_get_test_noise(void);

#endif // DD_SNS_TEMPERATURE_H
