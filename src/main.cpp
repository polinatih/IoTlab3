/**
 * @file main.cpp
 * @brief Точка входа: только перенаправление к выбранному приложению.
 *
 * Символ APP_NAME (app_config.h) работает как переключатель компиляции:
 * препроцессор оставляет только код выбранной лабораторной работы.
 */
#include <Arduino.h>
#include "app_config.h"

#if APP_NAME == APP_LAB_3_1
#include "app/lab_3_1/app_lab_3_1.h"
#elif APP_NAME == APP_LAB_3_2
#include "app/lab_3_2/app_lab_3_2.h"
#else
#error "APP_NAME: выберите приложение в app_config.h"
#endif

void setup()
{
#if APP_NAME == APP_LAB_3_1
    app_lab_3_1_setup();
#elif APP_NAME == APP_LAB_3_2
    app_lab_3_2_setup();
#endif
    // После setup() библиотека FreeRTOS сама запускает планировщик.
}

void loop()
{
    // loop() выполняется внутри Idle-задачи FreeRTOS — здесь ничего не делаем.
#if APP_NAME == APP_LAB_3_1
    app_lab_3_1_loop();
#elif APP_NAME == APP_LAB_3_2
    app_lab_3_2_loop();
#endif
}
