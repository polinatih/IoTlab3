/**
 * @file srv_lcd_stdio.h
 * @brief Сервис: LCD 1602 как поток STDIO (экран с платой I2C или обычный параллельный 1602A).
 *
 * На LCD пишут через fprintf(srv_lcd_stdio_stream(), ...), а printf()
 * продолжает идти в Serial. (Переиспользуется из лабораторной 1.2.)
 *
 * Управляющие символы в потоке:
 *   '\f' — очистить экран и вернуть курсор в (0,0)
 *   '\n' — перейти в начало следующей строки
 *
 * Тип экрана (SRV_LCD_TYPE):
 *   SRV_LCD_TYPE_I2C      — плата PCF8574 сзади, 4 провода: GND, VCC, SDA, SCL
 *                           (Uno: SDA=A4, SCL=A5;  Mega: SDA=20, SCL=21)
 *   SRV_LCD_TYPE_PARALLEL — «голый» 1602A, 16 выводов, 4-битный режим (пины ниже)
 *   SRV_LCD_TYPE_AUTO     — при старте ищет I2C-экран по адресам 0x27 и 0x3F;
 *                           не нашёл — работает как с параллельным.
 * По умолчанию: Mega — AUTO (ОЗУ хватает на оба драйвера), Uno — I2C (экономия ОЗУ).
 * Какой экран найден — печатается в Serial при старте («LCD: ...»).
 */
#ifndef SRV_LCD_STDIO_H
#define SRV_LCD_STDIO_H

#include <stdio.h>
#include <stdint.h>

#define SRV_LCD_TYPE_AUTO       0
#define SRV_LCD_TYPE_I2C        1
#define SRV_LCD_TYPE_PARALLEL   2

#ifndef SRV_LCD_TYPE
  #if defined(__AVR_ATmega2560__)
    #define SRV_LCD_TYPE  SRV_LCD_TYPE_AUTO
  #else
    #define SRV_LCD_TYPE  SRV_LCD_TYPE_I2C
  #endif
#endif

// ---- Экран с платой I2C
#define SRV_LCD_I2C_ADDRESS       0x27   // самый частый адрес PCF8574
#define SRV_LCD_I2C_ADDRESS_ALT   0x3F   // второй вариант (PCF8574A); проверяется в режиме AUTO

// ---- Параллельный 1602A (4-битный режим, как в примере Arduino «LiquidCrystal»)
//  LCD:  1 VSS->GND  2 VDD->5V  3 V0->средняя ножка потенциометра 10 кОм  5 RW->GND
//        15 A->5V через 220 Ом  16 K->GND  7..10 (D0..D3) не подключать
#define SRV_LCD_PIN_RS   12      // LCD вывод 4
#define SRV_LCD_PIN_E    11      // LCD вывод 6
#define SRV_LCD_PIN_D4   5       // LCD вывод 11
#define SRV_LCD_PIN_D5   4       // LCD вывод 12
#define SRV_LCD_PIN_D6   3       // LCD вывод 13
#define SRV_LCD_PIN_D7   2       // LCD вывод 14

#define SRV_LCD_COLUMNS  16
#define SRV_LCD_ROWS     2

void  srv_lcd_stdio_setup(void);
FILE *srv_lcd_stdio_stream(void);
void  srv_lcd_stdio_set_cursor(uint8_t column, uint8_t row);

#endif // SRV_LCD_STDIO_H
