/**
 * @file srv_lcd_stdio.cpp
 * @brief Реализация потока STDIO поверх LiquidCrystal_I2C или LiquidCrystal (параллельный).
 */
#include "srv_lcd_stdio.h"
#include <Arduino.h>

#define SRV_LCD_HAS_I2C       (SRV_LCD_TYPE != SRV_LCD_TYPE_PARALLEL)
#define SRV_LCD_HAS_PARALLEL  (SRV_LCD_TYPE != SRV_LCD_TYPE_I2C)

#if SRV_LCD_HAS_I2C
  #include <Wire.h>
  #include <LiquidCrystal_I2C.h>
  // Тайм-аут шины I2C: если дисплей не подключён, программа не зависнет в Wire
  #define SRV_LCD_I2C_TIMEOUT_US   25000UL
  static LiquidCrystal_I2C lcd_i2c(SRV_LCD_I2C_ADDRESS, SRV_LCD_COLUMNS, SRV_LCD_ROWS);
#endif
#if SRV_LCD_TYPE == SRV_LCD_TYPE_AUTO
  static LiquidCrystal_I2C lcd_i2c_alt(SRV_LCD_I2C_ADDRESS_ALT, SRV_LCD_COLUMNS, SRV_LCD_ROWS);
#endif
#if SRV_LCD_HAS_PARALLEL
  #include <LiquidCrystal.h>
  static LiquidCrystal lcd_parallel(SRV_LCD_PIN_RS, SRV_LCD_PIN_E, SRV_LCD_PIN_D4,
                                    SRV_LCD_PIN_D5, SRV_LCD_PIN_D6, SRV_LCD_PIN_D7);
#endif

/** Какой драйвер реально используется. */
typedef enum { LCD_USE_I2C, LCD_USE_I2C_ALT, LCD_USE_PARALLEL } srv_lcd_use_t;

static srv_lcd_use_t lcd_use =
#if SRV_LCD_TYPE == SRV_LCD_TYPE_PARALLEL
    LCD_USE_PARALLEL;
#else
    LCD_USE_I2C;
#endif

static FILE    lcd_stream;
static uint8_t current_row = 0;

// ---------------------------------------------------------------- Обёртки над драйверами
static void srv_lcd_clear(void)
{
    switch (lcd_use) {
#if SRV_LCD_HAS_I2C
    case LCD_USE_I2C:      lcd_i2c.clear(); break;
#endif
#if SRV_LCD_TYPE == SRV_LCD_TYPE_AUTO
    case LCD_USE_I2C_ALT:  lcd_i2c_alt.clear(); break;
#endif
#if SRV_LCD_HAS_PARALLEL
    case LCD_USE_PARALLEL: lcd_parallel.clear(); break;
#endif
    default: break;
    }
}

static void srv_lcd_cursor(uint8_t column, uint8_t row)
{
    switch (lcd_use) {
#if SRV_LCD_HAS_I2C
    case LCD_USE_I2C:      lcd_i2c.setCursor(column, row); break;
#endif
#if SRV_LCD_TYPE == SRV_LCD_TYPE_AUTO
    case LCD_USE_I2C_ALT:  lcd_i2c_alt.setCursor(column, row); break;
#endif
#if SRV_LCD_HAS_PARALLEL
    case LCD_USE_PARALLEL: lcd_parallel.setCursor(column, row); break;
#endif
    default: break;
    }
}

static void srv_lcd_write(uint8_t symbol)
{
    switch (lcd_use) {
#if SRV_LCD_HAS_I2C
    case LCD_USE_I2C:      lcd_i2c.write(symbol); break;
#endif
#if SRV_LCD_TYPE == SRV_LCD_TYPE_AUTO
    case LCD_USE_I2C_ALT:  lcd_i2c_alt.write(symbol); break;
#endif
#if SRV_LCD_HAS_PARALLEL
    case LCD_USE_PARALLEL: lcd_parallel.write(symbol); break;
#endif
    default: break;
    }
}

#if SRV_LCD_TYPE == SRV_LCD_TYPE_AUTO
/** Есть ли на шине I2C устройство с таким адресом (ответило ACK). */
static bool srv_lcd_i2c_present(uint8_t address)
{
    Wire.beginTransmission(address);
    return Wire.endTransmission() == 0;
}
#endif

/** Вывод одного символа на LCD (вызывается fprintf). */
static int srv_lcd_put_char(char symbol, FILE *stream)
{
    (void)stream;
    if (symbol == '\f') {                       // очистка экрана
        srv_lcd_clear();
        current_row = 0;
    } else if (symbol == '\n') {                // следующая строка
        current_row = (uint8_t)((current_row + 1) % SRV_LCD_ROWS);
        srv_lcd_cursor(0, current_row);
    } else {
        srv_lcd_write((uint8_t)symbol);
    }
    return 0;
}

void srv_lcd_stdio_setup(void)
{
#if SRV_LCD_HAS_I2C
    Wire.begin();
    Wire.setWireTimeout(SRV_LCD_I2C_TIMEOUT_US, true);
#endif

#if SRV_LCD_TYPE == SRV_LCD_TYPE_AUTO
    // Автоопределение: сначала ищем плату I2C, иначе считаем экран параллельным
    if (srv_lcd_i2c_present(SRV_LCD_I2C_ADDRESS)) {
        lcd_use = LCD_USE_I2C;
    } else if (srv_lcd_i2c_present(SRV_LCD_I2C_ADDRESS_ALT)) {
        lcd_use = LCD_USE_I2C_ALT;
    } else {
        lcd_use = LCD_USE_PARALLEL;
    }
#endif

    switch (lcd_use) {
#if SRV_LCD_HAS_I2C
    case LCD_USE_I2C:
        lcd_i2c.init();
        lcd_i2c.backlight();
        printf_P(PSTR("LCD: I2C, address 0x%02X\n"), SRV_LCD_I2C_ADDRESS);
        break;
#endif
#if SRV_LCD_TYPE == SRV_LCD_TYPE_AUTO
    case LCD_USE_I2C_ALT:
        lcd_i2c_alt.init();
        lcd_i2c_alt.backlight();
        printf_P(PSTR("LCD: I2C, address 0x%02X\n"), SRV_LCD_I2C_ADDRESS_ALT);
        break;
#endif
#if SRV_LCD_HAS_PARALLEL
    case LCD_USE_PARALLEL:
        lcd_parallel.begin(SRV_LCD_COLUMNS, SRV_LCD_ROWS);
        printf_P(PSTR("LCD: parallel 1602A (RS=%d E=%d D4..D7=%d,%d,%d,%d)\n"),
                 SRV_LCD_PIN_RS, SRV_LCD_PIN_E, SRV_LCD_PIN_D4, SRV_LCD_PIN_D5,
                 SRV_LCD_PIN_D6, SRV_LCD_PIN_D7);
        break;
#endif
    default:
        break;
    }
    srv_lcd_clear();
    fdev_setup_stream(&lcd_stream, srv_lcd_put_char, NULL, _FDEV_SETUP_WRITE);
}

FILE *srv_lcd_stdio_stream(void)
{
    return &lcd_stream;
}

void srv_lcd_stdio_set_cursor(uint8_t column, uint8_t row)
{
    srv_lcd_cursor(column, row);
    current_row = row;
}
