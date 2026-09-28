#include "pico/stdlib.h"

// Defined in blink.hike, compiled separately by hikec + clang into blink.o.
extern void setup(void);
extern void loop(void);

int main() {
    setup();
    while (true) {
        loop();
    }
    return 0;
}
