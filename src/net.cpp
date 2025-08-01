#ifndef MDNS_H
#define MDNS_H

#include "lwip/apps/mdns.h"
#include "lwip/dns.h"
#include "lwip/ip.h"
#include "pico/cyw43_arch.h"
#include <stdio.h>

int connect_wifi(const char *ssid, const char *password) {
	cyw43_arch_enable_sta_mode();
	if (cyw43_arch_wifi_connect_timeout_ms(
					ssid, password, CYW43_AUTH_WPA2_AES_PSK, 30000) != PICO_ERROR_NONE) {
		return 1;
	}
	auto self_ip = ip4addr_ntoa(netif_ip4_addr(netif_list));
	printf("Connected to WiFi, My IP: %s\n", self_ip);
	return 0;
}

#ifndef SRV_ENABLED
#define SRV_ENABLED 0
#endif

#if LWIP_MDNS_RESPONDER && SRV_ENABLED
static void srv_txt(struct mdns_service *service, void *txt_userdata) {
	err_t res;
	LWIP_UNUSED_ARG(txt_userdata);
	res = mdns_resp_add_service_txtitem(service, "path=/", 6);
	LWIP_ERROR("mdns add service txt failed\n", (res == ERR_OK), return);
}
#endif

int init_mdns(const char *hostname, int port) {
#if LWIP_MDNS_RESPONDER
	cyw43_arch_lwip_begin();
	mdns_resp_init();
	mdns_resp_add_netif(&cyw43_state.netif[CYW43_ITF_STA], hostname);
	mdns_resp_add_service(&cyw43_state.netif[CYW43_ITF_STA], "pico_server",
												"_tcp", DNSSD_PROTO_TCP, 8000,
#if SRV_ENABLED
												srv_txt
#else
												nullptr
#endif
												,
												0);
	cyw43_arch_lwip_end();
	printf("mdns host name %s.local\n", hostname);
	return 0;
#else
	printf("mDNS responder not enabled\n");
	return 1;
#endif
}

#endif // MDNS_H
