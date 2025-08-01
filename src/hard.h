#ifndef HARD_H
#define HARD_H

extern bool cyw43_led_state;
void toggle_led();
void show_memory_usage();
int init_led_blink_worker(int duration_ms, int interval_ms);

#endif // HARD_H
