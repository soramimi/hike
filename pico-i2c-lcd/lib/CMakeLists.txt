cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pico_lcd_1602_i2c C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# Toolchain used to compile the Hike sources. hikec emits LLVM IR;
# clang (not the project's arm-none-eabi-gcc) turns that IR into a Thumb
# object file, which is then linked normally alongside hike_crt_main.c/hike_shim.c.
find_program(CLANG_EXECUTABLE clang REQUIRED)
set(HIKEC "${CMAKE_CURRENT_SOURCE_DIR}/../../hike-lang/hikec" CACHE FILEPATH "Path to the hikec compiler binary")

set(HIKE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/../main.hike
    ${CMAKE_CURRENT_SOURCE_DIR}/../i2c-lcd.hike
)
set(HIKE_LL ${CMAKE_CURRENT_BINARY_DIR}/lcd.ll)
set(HIKE_OBJ ${CMAKE_CURRENT_BINARY_DIR}/lcd.o)

add_custom_command(
    OUTPUT ${HIKE_LL}
    COMMAND ${HIKEC} emit-ir -target cortex-m0 ${HIKE_SRC} -o ${HIKE_LL}
    DEPENDS ${HIKE_SRC} ${HIKEC}
    COMMENT "hikec: Hike LCD sources -> lcd.ll (thumbv6m-none-eabi)"
    VERBATIM
)

add_custom_command(
    OUTPUT ${HIKE_OBJ}
    COMMAND ${CLANG_EXECUTABLE} --target=thumbv6m-none-eabi -mcpu=cortex-m0plus -mthumb
            -Oz -ffunction-sections -fdata-sections
            -c ${HIKE_LL} -o ${HIKE_OBJ}
    DEPENDS ${HIKE_LL}
    COMMENT "clang: lcd.ll -> lcd.o"
    VERBATIM
)

add_custom_target(hike_lcd_obj DEPENDS ${HIKE_OBJ})

add_executable(pico_lcd_1602_i2c
    hike_crt_main.c
    hike_shim.c
)
add_dependencies(pico_lcd_1602_i2c hike_lcd_obj)

target_link_libraries(pico_lcd_1602_i2c
    pico_stdlib
    hardware_i2c
    ${HIKE_OBJ}
)

# Discard unused Hike runtime helpers that reference host OS functions.
target_link_options(pico_lcd_1602_i2c PRIVATE -Wl,--gc-sections)

pico_add_extra_outputs(pico_lcd_1602_i2c)
