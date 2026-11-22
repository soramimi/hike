# AGENTS.md

This file is for AI coding agents working in this workspace. Humans should
read [README.md](README.md) and [QuickStart.md](QuickStart.md) instead.

## Workspace layout

```
hike-lang/     git submodule: fork of https://github.com/kanryu/hike-lang
               (the compiler, `hikec`), `pico` branch
pico-sdk/      git submodule: Raspberry Pi Pico SDK (2.3.1)
pico-blink/    Example: LED-blink firmware for RP2040, logic written in Hike
pico-i2c-lcd/  Example: HD44780 16x2 LCD over a PCF8574 I2C backpack, in Hike
QuickStart.md  Build hikec + run Hello World
```

## Building hikec

```bash
cd hike-lang
go build -o hikec ./cmd/hikec
```

Requires Go 1.21+ and Clang/LLVM 15+ (both already installed in this
environment: `clang`, `arm-none-eabi-gcc`, `cmake`, `ninja` are all on PATH).

## Gotcha: reserved runtime symbol names

Hike's compiler always embeds a built-in runtime IR file into every compiled
program (chosen by target triple substring in
`pkg/backend/llvm/runtime.go:GetRuntimeIR`). A fixed set of symbol names are
reserved by that runtime and must **not** be reused as your own function
names (extern declarations or definitions) in `.hike` source, or the linker
will silently bind your calls to the runtime's own implementation instead of
yours:

```
hike_thread_spawn, hike_event_create, hike_event_signal, hike_event_wait,
hike_event_destroy, hike_sleep_ms, hike_now_ns, malloc, calloc, free,
memcpy, memcmp, strlen, strcmp (and their *32 variants)
```

See `pkg/backend/llvm/runtime.go`'s `RuntimeLLVMSymbols` map for the full
list. This bit us once in `pico-blink/main.hike`: a function named
`hike_sleep_ms` silently got linked to the runtime's own pthread/`nanosleep`
based implementation instead of the custom pico-sdk shim, causing an
`undefined reference to nanosleep` link error on a bare-metal target. Fix:
prefix your own extern C-ABI shims with something else (we use `pico_`).

## Bare-metal / Cortex-M0 (RP2040) target

`pkg/target/target.go` has a `cortex-m0` target added for this workspace
(triple `thumbv6m-none-eabi`, not upstream in kanryu/hike-lang). Use it like:

```bash
hikec emit-ir -target cortex-m0 foo.hike -o foo.ll
clang --target=thumbv6m-none-eabi -mcpu=cortex-m0plus -mthumb \
      -Oz -ffunction-sections -fdata-sections -c foo.ll -o foo.o
```

Do **not** use `hikec build` for this target — its native "build" path links
against a host OS's libc, which doesn't exist on bare metal. Instead, emit IR,
compile to a relocatable object with `clang -c` as above, and link that
object into a real toolchain's build (here: pico-sdk's CMake/Ninja build, see
`pico-blink/lib/CMakeLists.txt`). Link with `-Wl,--gc-sections` so unused
runtime helpers (pthread/malloc/nanosleep declarations that are always
embedded but never actually implemented for this target) get dropped instead
of causing undefined-reference errors.

Runtime IR selection: `GetRuntimeIR` picks `runtime_common32.ll` when the
target's `PointerBits` is 32 (true for `cortex-m0`), so strings use the
32-bit layout `{ i8*, i32, i32 }` and the `*32` helpers. The OS part still
comes from the Linux template (pthread/nanosleep/write declarations); these
are dropped by `--gc-sections` as long as nothing calls them.

Strings on `cortex-m0` (checked against emitted IR and the linked ELF):

- `len(s)`, `s[i]`, `s[a:b]`, `==`, and string literals are self-contained
  (inline code plus `memcmp32`/`memcpy32` from the runtime). They link
  nothing external. `pico-i2c-lcd/` relies on these.
- `a + b` calls `hike_strcat_len32` → `malloc`. pico-sdk's newlib supplies
  `malloc`, so it *links*, but there is no GC: every concatenation leaks
  heap. Never concatenate inside `loop()`.
- Indexing and slicing are not bounds-checked; out-of-range reads silently
  read adjacent memory.
- `len` counts bytes, not characters. The HD44780 character ROM is not
  UTF-8, so only ASCII renders correctly on the LCD.

Closures, maps, and concurrency features are untested on this target.

## pico-blink build

```bash
cd pico-blink
make          # configures (cmake -S lib -B build -G Ninja) then builds
make clean
make flash    # copies build/blink.uf2 to a mounted RPI-RP2 drive
```

`CMAKE_POLICY_VERSION_MINIMUM=3.5` is exported by the Makefile itself —
needed because pico-sdk vendors an old mbedtls whose
`cmake_minimum_required()` predates policies removed by newer CMake, and
because `picotool` gets fetched/built from source on first run.

File layout: `*.hike` and `Makefile` stay at the example's top level;
everything CMake/pico-sdk-specific (`CMakeLists.txt`, `hike_crt_main.c`,
`hike_shim.c`, `pico_sdk_import.cmake`) lives under `lib/`.

`pico-i2c-lcd/` uses the same layout and Make targets; its output is
`build/pico_lcd_1602_i2c.uf2`, and its `lib/CMakeLists.txt` passes both
`main.hike` and `i2c-lcd.hike` to one `hikec emit-ir` call.

The `clang` step prints `warning: overriding the module target triple with
thumbv6m-unknown-none-eabi [-Woverride-module]`. It's harmless: hikec writes
`thumbv6m-none-eabi`, and clang normalizes it to the 4-part form.

## General notes

- `hike-lang/` and `pico-sdk/` are git submodules. Changes to the compiler
  (for example `pkg/target/target.go`) must be committed inside
  `hike-lang/` first, then the submodule pointer is committed here.
- `hike-lang/hikec` is a build artifact and is not committed.
- Prefer testing actual builds (`go build`, `hikec run`, `make`) over
  reasoning from memory about whether something works — this workspace has
  already hit a couple of non-obvious runtime/linker gotchas (see above).
