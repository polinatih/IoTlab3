/**
 * @file dd_led.h
 * @brief Драйвер светодиода (device driver), переиспользуется из лаб 1.x/2.x.
 */
#ifndef DD_LED_H
#define DD_LED_H

#include <stdint.h>

void dd_led_setup(uint8_t pin);           ///< вывод -> OUTPUT, LED выключен
void dd_led_set(uint8_t pin, bool is_on); ///< включить / выключить
void dd_led_on(uint8_t pin);
void dd_led_off(uint8_t pin);

#endif // DD_LED_H
