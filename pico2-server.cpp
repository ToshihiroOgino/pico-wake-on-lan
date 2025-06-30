#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include <stdio.h>

#define BUILTIN_LED CYW43_WL_GPIO_LED_PIN
#define GPIO_BTN 15

int connect_wifi() {
	// Enable wifi station
	cyw43_arch_enable_sta_mode();

	printf("Connecting to Wi-Fi...\n");
	if (cyw43_arch_wifi_connect_timeout_ms("Your Wi-Fi SSID",
																				 "Your Wi-Fi Password",
																				 CYW43_AUTH_WPA2_AES_PSK, 30000)) {
		printf("failed to connect.\n");
		return 1;
	} else {
		printf("Connected.\n");
		// Read the ip address in a human readable way
		uint8_t *ip_address = (uint8_t *)&(cyw43_state.netif[0].ip_addr.addr);
		printf("IP address %d.%d.%d.%d\n", ip_address[0], ip_address[1],
					 ip_address[2], ip_address[3]);
	}
	return 0;
}

int init() {
	stdio_init_all();
	gpio_init(GPIO_BTN);
	gpio_set_dir(GPIO_BTN, GPIO_IN);
	gpio_pull_up(GPIO_BTN);
	return cyw43_arch_init();
}

static bool cyw43_led = false;
void toggle_led() {
	cyw43_led = !cyw43_led;
	cyw43_arch_gpio_put(BUILTIN_LED, cyw43_led);
}

int main() {
	if (init()) {
		printf("init failed\n");
		return -1;
	}

	const int loop_delay = 1;
	int counter = 0;
	const int max_counter = 2000 / loop_delay;

	while (true) {
		if (counter++ >= max_counter) {
			printf(".\n");
			counter = 0;
			cyw43_arch_gpio_put(BUILTIN_LED, true);
			sleep_ms(100);
			cyw43_arch_gpio_put(BUILTIN_LED, false);
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
