#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"

#define PUL 13
#define DIR 12

void setup_stepper() {
    gpio_init(PUL);
    gpio_set_dir(PUL, GPIO_OUT);
    gpio_put(PUL, 0);

    gpio_init(DIR);
    gpio_set_dir(DIR, GPIO_OUT);
    gpio_put(DIR, 0);
}

static uint32_t abs(int x) {
    return (uint32_t) (x < 0 ? -x : x);
}

static void step_once_period_us(uint32_t period_us) {
    uint32_t half = period_us / 2;
    if (half < 5) half = 5; // keep pulses comfortably wide

    gpio_put(PUL, 1);
    busy_wait_us_32(half);
    gpio_put(PUL, 0);
    busy_wait_us_32(half);
}

void move_steps_ramped(uint32_t steps, bool dir, uint32_t start_period_us,
                      uint32_t target_period_us, uint32_t ramp_steps) {
    gpio_put(DIR, dir ? 1 : 0);
    busy_wait_us_32(10); // DIR setup time

    if (steps == 0) return;

    if (ramp_steps * 2 > steps) ramp_steps = steps / 2;
    if (ramp_steps == 0) ramp_steps = 1;

    uint32_t period = start_period_us;

    uint32_t delta = 0;
    if (start_period_us > target_period_us) {
        delta = (start_period_us - target_period_us) / ramp_steps;
        if (delta < 1) delta = 1;
    }

    for (uint32_t i = 0; i < steps; i++) {
        step_once_period_us(period);
        
        // accelerate
        if (i < ramp_steps && period > target_period_us) {
            period = (period > delta) ? (period - delta) : target_period_us;
            if (period < target_period_us) period = target_period_us;
        }

        // decelerate
        if (i >= (steps - ramp_steps) && period < start_period_us) {
            period += delta;
            if (period > start_period_us) period = start_period_us;
        }
    }
}