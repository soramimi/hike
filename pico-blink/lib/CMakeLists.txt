cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pico_blink C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# Toolchain used to compile blink.hike. hikec turns .hike into LLVM IR;
# clang (not the project's arm-none-eabi-gcc) turns that IR into a Thumb
# object file, which is then linked normally alongside main.c/hike_shim.c.
find_program(CLANG_EXECUTABLE clang REQUIRED)
set(HIKEC "${CMAKE_CURRENT_SOURCE_DIR}/../../hike-lang/hikec" CACHE FILEPATH "Path to the hikec compiler binary")

set(HIKE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../blink.hike)
set(HIKE_LL ${CMAKE_CURRENT_BINARY_DIR}/blink.ll)
set(HIKE_OBJ ${CMAKE_CURRENT_BINARY_DIR}/blink.o)

add_custom_command(
    OUTPUT ${HIKE_LL}
    COMMAND ${HIKEC} emit-ir -target cortex-m0 ${HIKE_SRC} -o ${HIKE_LL}
    DEPENDS ${HIKE_SRC} ${HIKEC}
    COMMENT "hikec: blink.hike -> blink.ll (thumbv6m-none-eabi)"
    VERBATIM
)

add_custom_command(
    OUTPUT ${HIKE_OBJ}
    COMMAND ${CLANG_EXECUTABLE} --target=thumbv6m-none-eabi -mcpu=cortex-m0plus -mthumb
            -Oz -ffunction-sections -fdata-sections
            -c ${HIKE_LL} -o ${HIKE_OBJ}
    DEPENDS ${HIKE_LL}
    COMMENT "clang: blink.ll -> blink.o"
    VERBATIM
)

add_custom_target(hike_blink_obj DEPENDS ${HIKE_OBJ})

add_executable(blink
    main.c
    hike_shim.c
)
add_dependencies(blink hike_blink_obj)

target_link_libraries(blink
    pico_stdlib
    ${HIKE_OBJ}
)

# Unreferenced runtime helpers embedded in blink.o (Hike's pthread/malloc-based
# native runtime, not yet trimmed down for bare-metal targets) rely on
# --gc-sections to drop away before link since this target has no libc/pthread.
target_link_options(blink PRIVATE -Wl,--gc-sections)

pico_add_extra_outputs(blink)
