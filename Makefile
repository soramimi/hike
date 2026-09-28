
main:
	less QuickStart.md

hikec: hike-lang/hikec

hike-lang/hikec:
	cd hike-lang && go build -o hikec ./cmd/hikec

pico-blink:
	cd pico-blink && make

clean-pico-blink:
	cd pico-blink && make clean

flash-pico-blink:
	cd pico-blink && make flash

.PHONY: hikec pico-blink clean-pico-blink flash-pico-blink

