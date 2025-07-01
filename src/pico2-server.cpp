#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include <stdio.h>

#include "wifi.h"
#include "xip_config.h"

#define BUILTIN_LED CYW43_WL_GPIO_LED_PIN
#define GPIO_BTN 15

int init() {
	stdio_init_all();
	if (load_config()) {
		printf("Failed to load config\n");
		return 1;
	}
	gpio_init(GPIO_BTN);
	gpio_set_dir(GPIO_BTN, GPIO_IN);
	gpio_pull_up(GPIO_BTN);
	if (cyw43_arch_init()) {
		printf("Failed to initialize CYW43\n");
		return 1;
	}
	return 0;
}

static bool cyw43_led = false;
void toggle_led() {
	cyw43_led = !cyw43_led;
	cyw43_arch_gpio_put(BUILTIN_LED, cyw43_led);
}

int main() {
	if (init()) {
		return -1;
	}

	// if (connect_wifi()) {
	// 	while(true){
	// 		printf("WiFi Connection Failed\n");
	// 		sleep_ms(1000);
	// 	}
	// }

	const int loop_delay = 1;
	int counter = 0;
	const int max_counter = 2000 / loop_delay;

	const auto ip = get_config_value("ipv4");
	const auto ssid = get_config_value("ssid");
	const auto password = get_config_value("password");

	while (true) {
		if (counter++ >= max_counter) {
			printf("led\n");
			counter = 0;
			cyw43_arch_gpio_put(BUILTIN_LED, true);
			sleep_ms(100);
			cyw43_arch_gpio_put(BUILTIN_LED, false);

			printf("IP: %s\n", ip.c_str());
			printf("SSID: %s\n", ssid.c_str());
			printf("Password: %s\n", password.c_str());
		}

		if (gpio_get(GPIO_BTN) != GPIO_IRQ_LEVEL_LOW) {
			const int loop_times = 5;
			for (int i = 0; i < loop_times; i++) {
				printf("Button pressed %d/%d\n", i + 1, loop_times);
				toggle_led();
				sleep_ms(250);
			}
		}

		sleep_ms(loop_delay);
	}
}
