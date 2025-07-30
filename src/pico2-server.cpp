#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include <stdio.h>

#include "xip_config.h"

#define BUILTIN_LED CYW43_WL_GPIO_LED_PIN

static bool cyw43_led = false;
void toggle_led() {
	cyw43_led = !cyw43_led;
	printf("LED State: %s\n", cyw43_led ? "ON" : "OFF");
	cyw43_arch_gpio_put(BUILTIN_LED, cyw43_led);
}

int main() {
	stdio_init_all();
	cyw43_arch_init();

	sleep_ms(3000);

	printf("Starting...\n");

	if (load_config()) {
		printf("Failed to load config\n");
		return 1;
	}

	while (true) {
		toggle_led();
		sleep_ms(1000);
	}
}
