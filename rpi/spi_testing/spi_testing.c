#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include <math.h>
#include "motors/brushless.h"
#include "motors/stepper.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0

#define LED_PIN 25

#define LB_PWM 16 // FIX
#define LB_DIR 17 // FIX
#define RB_PWM 18 // FIX
#define RB_DIR 19 // FIX

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

    setup_pwm(LB_PWM, LB_DIR);
    setup_pwm(RB_PWM, RB_DIR);
    setup_stepper();

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
    
    // stupid ahh spi read byte by byte because pico sdk spi_read_blocking is broken
    for (int i = 0; i < 17; i++) {
        uint8_t byte = 0;
        spi_read_blocking(SPI_PORT, 0x00, NULL, 1);
    }

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

        // stupid ahh spi read byte by byte because pico sdk spi_read_blocking is broken
        for (int i = 0; i < 17; i++) {
            uint8_t byte = 0;
            spi_read_blocking(SPI_PORT, 0x00, &byte, 1);
            printf("Read byte %d: %02X\n", i, byte);
            rx_data[i] = byte;
        }
        
        
        double speed = 0.0;
        double steering = 0.0;

        // calculate checksum
        uint8_t checksum = 0;
        for (int i = 0; i < 16; i++) {
            checksum ^= rx_data[i];
        }

        printf("Received checksum: %02X, Calculated checksum: %02X\n", rx_data[16], checksum);

        if (checksum == rx_data[16]) { // make sure checksum matches
            // valid data, extract speed and steering
            memcpy(&speed, &rx_data[0], sizeof(double));
            memcpy(&steering, &rx_data[8], sizeof(double));

            // correctly scale
            if (speed > 1) {
                speed = 1;
            } else if (speed < 1) {
                speed = -1;
            }
            
            // assume that speed is between -1 and 1
            bool forward = speed > 0;
            uint32_t sp = (int) ((speed < 0 ? -speed : speed) * 65535u);
            set_brushless_pwm(LB_PWM, LB_DIR, sp, forward);
            set_brushless_pwm(RB_PWM, RB_DIR, sp, !forward);

            // do steering
        } else {
            //printf("Checksum error!\n");
        }
    }
}
