add_library(pico_opt4048
    ./opt4048.cpp
    ./conversions.cpp
    )

target_link_libraries(pico_opt4048
    pico_stdlib
    hardware_i2c
    )

target_include_directories(pico_opt4048 PUBLIC ./)
