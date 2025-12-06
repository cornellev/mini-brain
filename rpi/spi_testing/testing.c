#include <stdio.h>
#include <string.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"

#define SPI_PORT spi0
// RP2040 SPI0 default pins
#define PIN_MOSI 3    // RX
#define PIN_CS   1    // CSn
#define PIN_SCK  2    // SCK
#define PIN_MISO 0    // TX

#define LED_PIN 25

int main() {
    stdio_init_all();

    sleep_ms(2000); // wait for USB serial to connect

    // LED PWM init (just for fun)
    gpio_set_function(LED_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(LED_PIN);
    pwm_set_wrap(slice_num, 65535);
    pwm_set_clkdiv(slice_num, 10.0f);
    pwm_set_enabled(slice_num, true);
    pwm_set_gpio_level(LED_PIN, 65535); // full brightness at start

    // SPI init – 500 kHz, slave mode
    spi_init(SPI_PORT, 1 * 1000 * 1000);
    spi_set_slave(SPI_PORT, true);

    // All 4 SPI pins as SPI (including CS!)
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);

    // Mode 0 to start; if it misbehaves later, try CPOL_1/CPHA_1 (mode 3)
    // spi_set_format(SPI_PORT,
    //     8,
    //     SPI_CPOL_1,
    //     SPI_CPHA_1,
    //     SPI_MSB_FIRST
    // );

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    printf("Pico SPI slave ready\n");

    uint8_t b;

    while (true) {
        // // Wait until at least one byte is present in RX FIFO
        // if (spi_is_readable(SPI_PORT)) {
        //     spi_read_blocking(SPI_PORT, 0x0a, &b, 1);
        //     printf("Got byte: 0x%02X\n", b);
        // }
        //read 18 bytes
        uint8_t buf[2];
        spi_read_blocking(SPI_PORT, 0x0a, buf, sizeof buf);
        //print buf as hex
        printf("Got bytes: ");
        for (int i = 0; i < (sizeof buf) / (sizeof buf[0]); ++i) {
            printf("0x%02X ", buf[i]);
        }
        printf("\n");
    }
}
