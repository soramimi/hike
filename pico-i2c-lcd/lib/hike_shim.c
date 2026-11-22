#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"

#define LCD_I2C_SDA 16
#define LCD_I2C_SCL 17

void pico_i2c_init(void) {
    i2c_init(i2c0, 100000);
    gpio_set_function(LCD_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(LCD_I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(LCD_I2C_SDA);
    gpio_pull_up(LCD_I2C_SCL);
    bi_decl(bi_2pins_with_func(LCD_I2C_SDA, LCD_I2C_SCL, GPIO_FUNC_I2C));
}

void pico_i2c_write_byte(uint32_t address, uint32_t value) {
    uint8_t data = (uint8_t)value;
    i2c_write_blocking(i2c0, (uint8_t)address, &data, 1, false);
}

void pico_delay_us(uint32_t us) {
    sleep_us(us);
}

void pico_delay_ms(uint32_t ms) {
    sleep_ms(ms);
}
