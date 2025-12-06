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

    // 500 kHz
    spi_init(SPI_PORT, 500 * 1000);
    spi_set_slave(SPI_PORT, true);

    // Configure all SPI pins as SPI functions (including CS!)
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // Match Jetson: 8 bits, mode 0. If weird, try CPOL_1/CPHA_1 (mode 3).
    spi_set_format(SPI_PORT,
        8,
        SPI_CPOL_0,
        SPI_CPHA_0,
        SPI_MSB_FIRST
    );

    
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    // spi input is in the format of:
    // [0] = 0xAA (start byte)
    // [1..9] = speed data (double, 8 bytes)
    // [10...17] = steering data (double, 8 bytes)
    // 18 => XOR checksum of bytes [0..17]

    while (true) {
        // print out each byte
        gpio_put(PIN_CS, 0); // Assert CS
        uint8_t test_data[18] = {0};
        spi_read_blocking(SPI_PORT, 0x00, test_data, 18);
        gpio_put(PIN_CS, 1); // Deassert CS
        printf("Received bytes: ");
        for (int i = 0; i < 18; i++) {
            printf("0x%02X ", test_data[i]);
        }
        printf("\n");
    }

    // first we'll wait till we see a 0xAA byte
    printf("Waiting for data...\n");
    bool data_received = false;
    while (!data_received) {
        uint8_t rx_byte = 0;
        gpio_put(PIN_CS, 0); // Assert CS
        spi_read_blocking(SPI_PORT, 0x00, &rx_byte, 1);
        gpio_put(PIN_CS, 1); // Deassert CS
        if (rx_byte == 0xAA) {
            data_received = true;
        }
    }

    printf("Data received, entering main loop...\n");
    
    gpio_put(PIN_CS, 0);
    spi_read_blocking(SPI_PORT, 0x00, NULL, 18); // read and discard the rest of the first packet
    gpio_put(PIN_CS, 1);

    while (true) {
        // do a wait for start byte again
        data_received = false;
        while (!data_received) {
            uint8_t rx_byte = 0;
            gpio_put(PIN_CS, 0); // Assert CS
            spi_read_blocking(SPI_PORT, 0x00, &rx_byte, 1);
            gpio_put(PIN_CS, 1); // Deassert CS
            if (rx_byte == 0xAA) {
                data_received = true;
            }
        }

        // get data from spi
        uint8_t rx_data[17] = {0};
        gpio_put(PIN_CS, 0); // Assert CS
        spi_read_blocking(SPI_PORT, 0x00, rx_data, 18);
        gpio_put(PIN_CS, 1); // Deassert CS
        
        double speed = 0.0;
        double steering = 0.0;

        // calculate checksum
        uint8_t checksum = 0;
        for (int i = 0; i < 16; i++) {
            checksum ^= rx_data[i];
        }

        if (checksum == rx_data[16]) { // make sure checksum matches
            // valid data, extract speed and steering
            memcpy(&speed, &rx_data[0], sizeof(double));
            memcpy(&steering, &rx_data[8], sizeof(double));
            
            // print received values
            printf("Speed: %f, Steering: %f\n", speed, steering);
            // set LED brightness based on speed, speed is between -2.0 and 2.0
            double normalized_speed = (speed + 2.0) / 4.0;
            if (normalized_speed < 0.0) normalized_speed = 0.0;
            if (normalized_speed > 1.0) normalized_speed = 1.0;
            uint16_t duty_cycle = (uint16_t)(normalized_speed * 65535);
            pwm_set_gpio_level(LED_PIN, duty_cycle);
        } else {
            //printf("Checksum error!\n");
        }
    }
}
