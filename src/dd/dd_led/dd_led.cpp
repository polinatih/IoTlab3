/**
 * @file dd_led.cpp
 * @brief Реализация драйвера светодиода.
 */
#include "dd_led.h"
#include <Arduino.h>

void dd_led_setup(uint8_t pin)
{
    pinMode(pin, OUTPUT);
    dd_led_off(pin);
}

void dd_led_set(uint8_t pin, bool is_on)
{
    digitalWrite(pin, is_on ? HIGH : LOW);
}

void dd_led_on(uint8_t pin)
{
    dd_led_set(pin, true);
}

void dd_led_off(uint8_t pin)
{
    dd_led_set(pin, false);
}
