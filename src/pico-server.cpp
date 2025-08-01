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
#include "totp.h"
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

	const auto totp_secret = get_config_value("totp_secret");
	totp_init(totp_secret);

	start_server(port);
	printf("Server started on port %d\n", port);

	while (true) {
		// printf(".");
		const time_t current_time = get_posix_time_utc();
		printf("Current Time: %d\n", (u32_t)current_time);
		struct tm *utc = gmtime(&current_time);
		printf("%04d-%02d-%02d %02d:%02d:%02d UTC\n", 1900 + utc->tm_year,
					 1 + utc->tm_mon, utc->tm_mday, utc->tm_hour, utc->tm_min,
					 utc->tm_sec);

		// auto otp = generate_totp(current_time);
		auto otp = generate_totp(0);
		printf("TOTP Secret: %s\n", totp_secret.c_str());
		printf("Current OTP: %s\n", otp.c_str());

		toggle_led();
		sleep_ms(3000);
	}

	cyw43_arch_lwip_end();
}
