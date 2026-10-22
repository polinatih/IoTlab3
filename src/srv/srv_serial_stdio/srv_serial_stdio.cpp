/**
 * @file srv_serial_stdio.cpp
 * @brief Реализация: связываем потоки avr-libc stdout/stdin с Serial.
 */
#include "srv_serial_stdio.h"
#include <Arduino.h>

static FILE serial_stream;

/** Вывод символа (вызывается printf). '\n' дополняем '\r' для терминала. */
static int srv_serial_put_char(char symbol, FILE *stream)
{
    (void)stream;
    if (symbol == '\n') {
        Serial.write('\r');
    }
    Serial.write(symbol);
    return 0;
}

/** Ввод символа (вызывается scanf/getchar). Ждёт, пока символ придёт. */
static int srv_serial_get_char(FILE *stream)
{
    (void)stream;
    while (Serial.available() == 0) {
        // ожидание символа
    }
    return Serial.read();
}

void srv_serial_stdio_setup(void)
{
    Serial.begin(SRV_SERIAL_STDIO_BAUD);
    fdev_setup_stream(&serial_stream, srv_serial_put_char, srv_serial_get_char, _FDEV_SETUP_RW);
    stdout = &serial_stream;
    stdin  = &serial_stream;
}

int srv_serial_stdio_available(void)
{
    return Serial.available();
}
