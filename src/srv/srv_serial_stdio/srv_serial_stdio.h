/**
 * @file srv_serial_stdio.h
 * @brief Сервис: последовательный порт как стандартный ввод/вывод (STDIO).
 *
 * После srv_serial_stdio_setup() работают printf(), puts(), scanf(), getchar()
 * — текст идёт в Serial Monitor. (Переиспользуется из лабораторной 1.1.)
 */
#ifndef SRV_SERIAL_STDIO_H
#define SRV_SERIAL_STDIO_H

#include <stdio.h>
#include <stdint.h>

#define SRV_SERIAL_STDIO_BAUD   115200UL

void srv_serial_stdio_setup(void);

/** Есть ли непрочитанные символы (чтобы getchar() не блокировал задачу). */
int srv_serial_stdio_available(void);

#endif // SRV_SERIAL_STDIO_H
