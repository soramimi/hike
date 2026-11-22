#include "pico/stdlib.h"

// Defined in main.hike and linked from the Hike object.
extern void setup(void);
extern void loop(void);

int main() {
    setup();
    while (true) {
        loop();
    }
    return 0;
}
