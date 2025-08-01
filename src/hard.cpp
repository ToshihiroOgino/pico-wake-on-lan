#include "hard.h"

#include "pico/cyw43_arch.h"
#include "pico/stdio.h"
#include <malloc.h>
#include <stdio.h>

#define BUILTIN_LED CYW43_WL_GPIO_LED_PIN

bool cyw43_led_state = false;
int change_led_state(bool state) {
	cyw43_led_state = state;
	if (!cyw43_is_initialized(&cyw43_state)) {
		printf("cyw43 is not initialized, cannot control LED\n");
		return -1;
	}
	cyw43_arch_gpio_put(BUILTIN_LED, cyw43_led_state);
	return 0;
}

void toggle_led() {
	auto new_state = !cyw43_led_state;
	change_led_state(new_state);
}

void show_memory_usage() {
	struct mallinfo info = mallinfo();
	printf("Memory usage: %d bytes allocated, %d bytes in use\n", info.uordblks,
				 info.arena);
	printf("Free memory: %d bytes\n", info.fordblks);
	printf("Total memory: %d bytes\n", info.hblkhd);
}

async_at_time_worker_t led_blink_worker;
typedef struct {
	int duration_ms;
	int interval_ms;
} led_blink_setting_t;

void blink_led(__unused async_context_t *context,
							 async_at_time_worker_t *worker) {
	auto setting = (led_blink_setting_t *)worker->user_data;
	if (!setting) {
		return;
	}

	change_led_state(true);
	sleep_ms(setting->duration_ms);
	change_led_state(false);

	async_context_remove_at_time_worker(cyw43_arch_async_context(),
																			&led_blink_worker);
	auto res = async_context_add_at_time_worker_in_ms(
			cyw43_arch_async_context(), &led_blink_worker, setting->interval_ms);

	if (!res) {
		printf("Failed to reschedule LED blink worker\n");
	}
}

int init_led_blink_worker(int duration_ms, int interval_ms) {
	if (change_led_state(false) != 0) {
		return -1;
	}
	auto setting = (led_blink_setting_t *)calloc(1, sizeof(led_blink_setting_t));
	if (!setting) {
		return -1;
	}

	setting->duration_ms = duration_ms;
	setting->interval_ms = interval_ms;

	led_blink_worker.do_work = blink_led;
	led_blink_worker.user_data = setting;

	async_context_add_at_time_worker_in_ms(cyw43_arch_async_context(),
																				 &led_blink_worker, 0);
	return 0;
}
