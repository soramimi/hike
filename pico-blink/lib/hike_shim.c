// C-ABI shims called from blink.hike. Kept uint32-only on the Hike side to
// avoid depending on how Hike lowers `bool` for extern calls.
#include "pico/stdlib.h"

void pico_gpio_init(uint32_t pin) {
    gpio_init(pin);
}

void pico_gpio_set_dir_out(uint32_t pin) {
    gpio_set_dir(pin, true);
}

void pico_gpio_put(uint32_t pin, uint32_t value) {
    gpio_put(pin, value != 0);
}

void pico_delay_ms(uint32_t ms) {
    sleep_ms(ms);
}
