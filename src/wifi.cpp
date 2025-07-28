#include "wifi.h"

#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "xip_config.h"

#define NETMASK "255.255.255.0"
#define GATEWAY "192.168.1.1"

std::string config_get_ssid() { return get_config_value("ssid"); }
std::string config_get_password() { return get_config_value("password"); }
std::string config_get_ipv4() { return get_config_value("ipv4"); }

int connect_wifi() {
	const auto ssid = config_get_ssid();
	const auto password = config_get_password();
	const auto ipv4 = config_get_ipv4();

	if (ssid.empty() || password.empty() || ipv4.empty()) {
		printf("Invalid Wi-Fi configuration\n");
		return 1;
	}

	ip4_addr_t ipaddr, netmask, gateway;
	ip4addr_aton(ipv4.c_str(), &ipaddr);
	ip4addr_aton(NETMASK, &netmask);
	ip4addr_aton(GATEWAY, &gateway);

	struct netif *netif = netif_default;
	netif_set_addr(netif, &ipaddr, &netmask, &gateway);

	printf("Connecting to Wi-Fi...\n");
	if (cyw43_arch_wifi_connect_timeout_ms(ssid.c_str(), password.c_str(),
																				 CYW43_AUTH_WPA2_AES_PSK, 30000)) {
		printf("Failed to connect to Wi-Fi.\n");
		return 1;
	}
	printf("Connected.\nIP Address: %s\n", ipv4.c_str());
	return 0;
}
