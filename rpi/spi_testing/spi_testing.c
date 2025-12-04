#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include <math.h>

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

#define LED_PIN 25

int main()
{
    stdio_init_all();

    gpio_set_function(LED_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(LED_PIN);

    pwm_set_wrap(slice_num, 65535);
    pwm_set_clkdiv(slice_num, 10.0);
    pwm_set_enabled(slice_num, true);

    pwm_set_gpio_level(LED_PIN, 65534); // to full to indicate waiting for data

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    // spi input is in the format of:
    // [0] = 0xAA (start byte)
    // [1..9] = speed data (double, 8 bytes)
    // [10...17] = steering data (double, 8 bytes)
    // 18 => XOR checksum of bytes [0..17]

    while (true) {
        // get data from spi
        uint8_t rx_data[19] = {0};
        gpio_put(PIN_CS, 0); // Assert CS
        spi_read_blocking(SPI_PORT, 0x00, rx_data, 19);
        gpio_put(PIN_CS, 1); // Deassert CS
        
        double speed = 0.0;
        double steering = 0.0;
        // parse data
        if (rx_data[0] == 0xAA) {
            // calculate checksum
            uint8_t checksum = 0;
            for (int i = 0; i < 18; i++) {
                checksum ^= rx_data[i];
            }
            if (checksum == rx_data[18]) {
                // valid data, extract speed and steering
                memcpy(&speed, &rx_data[1], sizeof(double));
                memcpy(&steering, &rx_data[9], sizeof(double));
                
                // print received values
                printf("Speed: %f, Steering: %f\n", speed, steering);
                // set LED brightness based on speed, speed is between -2.0 and 2.0
                double normalized_speed = (speed + 2.0) / 4.0;
                if (normalized_speed < 0.0) normalized_speed = 0.0;
                if (normalized_speed > 1.0) normalized_speed = 1.0;
                uint16_t duty_cycle = (uint16_t)(normalized_speed * 65535);
                pwm_set_gpio_level(LED_PIN, duty_cycle);
            } else {
                printf("Checksum error!\n");
            }
        } else {
            printf("Invalid start byte!\n");
            pwm_set_gpio_level(LED_PIN, 65535 / 2); // Set LED to half brightness to indicate error
        }
    }
}
