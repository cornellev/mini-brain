#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/pwm.h"


void setup_pwm(int pin, int dir) {
    // Brushless PWM output
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint out_slice = pwm_gpio_to_slice_num(pin);

    // ~1.5 kHz @ clk_sys = 125 MHz
    pwm_set_wrap(out_slice, 57970);
    pwm_set_clkdiv_int_frac(out_slice, 1, 7); // 1 + 7/16 = 1.4375
    pwm_set_enabled(out_slice, true);

    pwm_set_gpio_level(pin, 0);

    gpio_init(dir);
    gpio_set_dir(dir, GPIO_OUT);
    gpio_put(dir, 0);
}

void set_brushless_pwm(int pin, int dir, uint32_t value, bool forward) {
    //TODO: maybe remove?
    uint32_t level_out = ((uint32_t) value * (57970u + 1u) + (65535u / 2u)) / 65535u;
    if (level_out > 57970u) level_out = 57970u;

    if (forward) {
        gpio_put(dir, 0);
    } else {
        gpio_put(dir, 1);
    }

    pwm_set_gpio_level(pin, (uint16_t)level_out);
}