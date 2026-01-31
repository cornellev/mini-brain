
void setup_stepper();

void move_steps_ramped(uint32_t steps, bool dir, uint32_t start_period_us, uint32_t target_period_us, uint32_t ramp_steps);