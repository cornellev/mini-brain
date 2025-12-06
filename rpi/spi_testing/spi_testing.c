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

#define LED_PIN 25

int main()
{
    stdio_init_all();

    sleep_ms(2000); // wait for USB serial to connect

    gpio_set_function(LED_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(LED_PIN);

    pwm_set_wrap(slice_num, 65535);
    pwm_set_clkdiv(slice_num, 10.0);
    pwm_set_enabled(slice_num, true);

    pwm_set_gpio_level(LED_PIN, 65534); // to full to indicate waiting for data

    // SPI init – 500 kHz, slave mode
    spi_init(SPI_PORT, 1 * 1000 * 1000);
    spi_set_slave(SPI_PORT, true);

    // All 4 SPI pins as SPI (including CS!)
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    // spi input is in the format of:
    // [0] = 0xAA (start byte)
    // [1..9] = speed data (double, 8 bytes)
    // [10...17] = steering data (double, 8 bytes)
    // 18 => XOR checksum of bytes [0..17]

    // first we'll wait till we see a 0xAA byte
    printf("Waiting for data...\n"); 
    bool data_received = false;
    while (!data_received) {
        uint8_t rx_byte = 0;
        spi_read_blocking(SPI_PORT, 0x00, &rx_byte, 1);
        if (rx_byte == 0xAA) {
            data_received = true;
        }
    }

    printf("Data received, entering main loop...\n");
    
    spi_read_blocking(SPI_PORT, 0x00, NULL, 18); // read and discard the rest of the first packet

    while (true) {
        // do a wait for start byte again
        data_received = false;
        while (!data_received) {
            uint8_t rx_byte = 0;
            spi_read_blocking(SPI_PORT, 0x00, &rx_byte, 1);
            if (rx_byte == 0xAA) {
                data_received = true;
            }
        }

        // get data from spi
        uint8_t rx_data[17] = {0};
        spi_read_blocking(SPI_PORT, 0x00, rx_data, 18);
        
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
