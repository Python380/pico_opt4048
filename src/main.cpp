#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <tusb.h>

#include "opt4048.h"

// constants
const uint64_t S_TO_uS          = 1000000;

// pin definition
const uint16_t LED_PIN          = 25;
const uint16_t SDA_PIN          = 4;
const uint16_t SCL_PIN          = 5;
const uint16_t OPT4048_INT_PIN  = 6;

// basic configuration
const uint32_t I2C_FREQ         = 400000; // 400 Khz

// main function - initialization & control loop
int main() {
    // initialize
    stdio_init_all();

    while (!tud_cdc_connected()) {sleep_ms(10);}
    sleep_ms(100);
    printf("Initializing...\n");

    // LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Initialize I2C communications for I2C bus 0
    i2c_init(i2c0, I2C_FREQ);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    // initialize OPT4048
    OPT4048 color_sensor(i2c0, OPT4048_ADDR);

    // state variables
    bool led_state = false;

    // control & tick loop
    uint64_t timer = time_us_64();
    uint64_t seconds_last_tick = 0;
    while (1) {
        if (timer - seconds_last_tick >= S_TO_uS) {  // 1 S = 1000 mS = 1000000 uS
            // Heartbeat <> LED State
            led_state = !led_state;
            gpio_put(LED_PIN, led_state);

            // read OPT4048
            color_sensor.readInRGB();

            // tick
            seconds_last_tick = timer;
        }

        timer = time_us_64();
    }
}