cmake_minimum_required(VERSION 3.20)

include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)
pico_sdk_init()

project(pico_opt4048_test)

include_directories(pico_opt4048_test
    ${PICO_SDK_PATH}/src/common/pico_util/include/
)

add_subdirectory(./lib/pico_opt4048)

add_executable(pico_opt4048_test
    # main
    ./src/main.cpp
    )

# pull in common dependencies
target_link_libraries(pico_opt4048_test
    pico_stdlib
    hardware_i2c
    pico_opt4048
)

pico_enable_stdio_usb(pico_opt4048_test 1)
pico_enable_stdio_uart(pico_opt4048_test 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(pico_opt4048_test)
