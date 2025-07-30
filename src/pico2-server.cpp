#include "lwip/apps/httpd.h"
#include "lwip/apps/mdns.h"
#include "lwip/init.h"
#include "lwip/ip4_addr.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include <stdio.h>

#include "error_handler.h"
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

	sleep_ms(3000);
	printf("Starting...\n");
	load_config();

	if (cyw43_arch_init_with_country(CYW43_COUNTRY_JAPAN)) {
		handle_error("Failed to initialize CYW43 architecture");
		return 1;
	}

	cyw43_arch_enable_sta_mode();
	const auto ssid = get_config_value("ssid");
	const auto password = get_config_value("password");
	if (cyw43_arch_wifi_connect_timeout_ms(ssid.c_str(), password.c_str(),
																				 CYW43_AUTH_WPA2_AES_PSK,
																				 15000) != PICO_ERROR_NONE) {
		handle_error("Failed to connect to WiFi");
		return 1;
	}

	while (true) {
		toggle_led();
		sleep_ms(1000);
	}
}
