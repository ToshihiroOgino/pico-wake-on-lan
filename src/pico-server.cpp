// #include <time.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

// #include "lwip/apps/mdns.h"
// #include "lwip/netif.h"
// #include "lwip/pbuf.h"
// #include "lwip/tcp.h"
// #include "lwip/tcpip.h"

#include "error_handler.h"
#include "hard.h"
#include "net.h"
#include "ntp.h"
#include "server.h"
#include "xip_config.h"

int main() {
	stdio_init_all();
	sleep_ms(3000);

	printf("Starting...\n");

	load_config();

	if (cyw43_arch_init_with_country(CYW43_COUNTRY_JAPAN)) {
		handle_error("Failed to initialize CYW43 architecture");
	}

	const auto ipv4 = get_config_value("ipv4");
	const auto ssid = get_config_value("ssid");
	const auto password = get_config_value("password");
	const int port = atoi(get_config_value("port").c_str());
	const auto hostname = get_config_value("hostname");
	printf("Connecting to WiFi SSID: %s\n", ssid.c_str());
	if (connect_wifi(ssid.c_str(), password.c_str(), ipv4.c_str())) {
		handle_error("Failed to connect to WiFi");
	}

	if (init_mdns(hostname.c_str(), port)) {
		handle_error("Failed to initialize mDNS");
	}

	printf("Starting NTP client...\n");
	if (run_ntp_client()) {
		handle_error("Failed to start NTP client");
	}

	start_server(port);
	printf("Server started on port %d\n", port);

	while (true) {
		printf(".");
		toggle_led();
		sleep_ms(1000);
	}

	cyw43_arch_lwip_end();
}
