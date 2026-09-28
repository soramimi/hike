# AGENTS.md

This file is for AI coding agents working in this workspace. Humans should
read [README.md](README.md) and [QuickStart.md](QuickStart.md) instead.

## Workspace layout

```
hike-lang/    Clone of https://github.com/kanryu/hike-lang (the compiler, `hikec`)
pico-sdk/     Raspberry Pi Pico SDK (v2.0.0), used only by pico-blink/
pico-blink/   Example: an LED-blink firmware for RP2040, partly written in Hike
QuickStart.md Build hikec + run Hello World
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
list. This bit us once in `pico-blink/blink.hike`: a function named
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

Known limitation: `GetRuntimeIR` doesn't recognize ARM/thumb triples, so it
falls back to the 64-bit-pointer Linux runtime IR. This is harmless as long
as the Hike code sticks to primitives (`uint32`, `int`, `bool`, plain
functions/loops) and avoids slices, strings, closures, or concurrency
features — those would pull in runtime helpers built around 64-bit pointer
layouts, which are wrong for a 32-bit target's ABI.

## pico-blink build

```bash
cd pico-blink
make          # configures (cmake -S lib -B build -G Ninja) then builds
make clean
make flash    # copies build/blink.uf2 to a mounted RPI-RP2 drive
```

`CMAKE_POLICY_VERSION_MINIMUM=3.5` is exported by the Makefile itself —
needed because pico-sdk 2.0.0 vendors an old mbedtls whose
`cmake_minimum_required()` predates policies removed by newer CMake, and
because `picotool` gets fetched/built from source on first run.

File layout: `blink.hike` and `Makefile` stay at `pico-blink/` top level;
everything CMake/pico-sdk-specific (`CMakeLists.txt`, `main.c`,
`hike_shim.c`, `pico_sdk_import.cmake`) lives under `pico-blink/lib/`.

## General notes

- This is not a git repository (`hike-lang/` and `pico-sdk/` are plain
  directory copies, not submodules), so there's no git history here to
  check with `git log`/`git blame`.
- Prefer testing actual builds (`go build`, `hikec run`, `make`) over
  reasoning from memory about whether something works — this workspace has
  already hit a couple of non-obvious runtime/linker gotchas (see above).
