# pico_opt4048
A Raspberry Pi Pico C/C++ SDK library for the OPT4048 Tristiumulus Color Sensor

## Build
Run the following command to build & test: `mkdir build && cd build && cmake .. && make`.

This assumes that you have a complete Raspberry Pi Pico dev toolchain installed, as per Raspberry Pi's instructions.

## Library

You can take the library itself from `lib/pico_opt4048`.

To include said library, add the following line to your CMakeLists.txt (Modify as needed):
```cmake
add_subdirectory(./lib/pico_opt4048)

#...

target_link_libraries(...
    ...
    pico_opt4048
)
```

Documentation for the library API can be found in `lib/pico_opt4048`