void init_step_timer(void);
void set_step_timer_period(int p);
void enable_step_timer_isr(void);
void disable_step_timer_isr(void);
void init_step_pins(void);
void init_steppers(void);
void set_step_pins(uint8_t);
void set_dir_pins(uint8_t);

void limits_init(void);
uint8_t limits_get_state(void);
void limits_disable(void);