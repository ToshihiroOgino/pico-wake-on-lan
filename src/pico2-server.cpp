#include <time.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

// #include <cstring>
// #include <stdio.h>

#include "error_handler.h"
#include "xip_config.h"

#define BUILTIN_LED CYW43_WL_GPIO_LED_PIN

static bool cyw43_led = false;
void toggle_led() {
	cyw43_led = !cyw43_led;
	printf("LED State: %s\n", cyw43_led ? "ON" : "OFF");
	cyw43_arch_gpio_put(BUILTIN_LED, cyw43_led);
}

static bool connected = false;

err_t connected_fn(void *arg, struct tcp_pcb *tpcb, err_t err) {
	if (err != ERR_OK) {
		auto msg = "Connection failed: " + std::to_string(err);
		return err;
	}
	printf("Connected to server successfully!\n");
	const char *msg = "Hello from Pico!";
	size_t msg_len = strlen(msg);
	struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, msg_len, PBUF_RAM);
	if (!p) {
		handle_error("Failed to allocate pbuf");
		return ERR_MEM;
	}
	connected = true;
	memcpy(p->payload, msg, msg_len);
	tcp_write(tpcb, p->payload, p->len, TCP_WRITE_FLAG_COPY);
	tcp_output(tpcb);
	return ERR_OK;
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

	const auto ip = "192.168.1.52";
	auto pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
	if (!pcb) {
		handle_error("Failed to create TCP PCB");
		return 1;
	}

	cyw43_arch_lwip_begin();
	ip_addr_t dest;
	ip4addr_aton(ip, &dest);
	auto err = tcp_connect(pcb, &dest, 8000, connected_fn);

	while (true) {
		toggle_led();
		sleep_ms(1000);
	}

	cyw43_arch_lwip_end();
}
