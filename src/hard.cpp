#include "hard.h"

#include "pico/cyw43_arch.h"
#include <stdio.h>

#define BUILTIN_LED CYW43_WL_GPIO_LED_PIN

bool cyw43_led_state = false;

void toggle_led() {
	cyw43_led_state = !cyw43_led_state;
	if (!cyw43_is_initialized(&cyw43_state)) {
		printf("cyw43 is not initialized, cannot control LED\n");
		return;
	}
	cyw43_arch_gpio_put(BUILTIN_LED, cyw43_led_state);
}
