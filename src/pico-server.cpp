#include <time.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "lwip/apps/mdns.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/tcpip.h"

// #include <cstring>
// #include <stdio.h>

#include "error_handler.h"
#include "xip_config.h"

#define BUILTIN_LED CYW43_WL_GPIO_LED_PIN

static bool cyw43_led = false;
void toggle_led() {
	cyw43_led = !cyw43_led;
	// printf("LED State: %s\n", cyw43_led ? "ON" : "OFF");
	printf(".");
	cyw43_arch_gpio_put(BUILTIN_LED, cyw43_led);
}

static bool connected = false;

err_t connected_fn(void *arg, struct tcp_pcb *pcb, err_t err) {
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
	tcp_write(pcb, p->payload, p->len, TCP_WRITE_FLAG_COPY);
	tcp_output(pcb);
	return ERR_OK;
}

static size_t get_mac_ascii(int idx, size_t chr_off, size_t chr_len,
														char *dest_in) {
	static const char hexchr[17] = "0123456789ABCDEF";
	uint8_t mac[6];
	char *dest = dest_in;
	assert(chr_off + chr_len <= (2 * sizeof(mac)));
	cyw43_hal_get_mac(idx, mac);
	for (; chr_len && (chr_off >> 1) < sizeof(mac); ++chr_off, --chr_len) {
		*dest++ = hexchr[mac[chr_off >> 1] >> (4 * (1 - (chr_off & 1))) & 0xf];
	}
	return dest - dest_in;
}

#if LWIP_MDNS_RESPONDER
static void srv_txt(struct mdns_service *service, void *txt_userdata) {
	err_t res;
	LWIP_UNUSED_ARG(txt_userdata);
	res = mdns_resp_add_service_txtitem(service, "path=/", 6);
	LWIP_ERROR("mdns add service txt failed\n", (res == ERR_OK), return);
}
#endif

err_t server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
	if (err != ERR_OK || !p) {
		printf("server_recv: error %d\n", err);
		return ERR_VAL;
	}
	auto len = p->len;
	char *received_data = (char *)calloc(p->len + 1, sizeof(char));
	if (!received_data) {
		handle_error("Failed to allocate memory for received data");
		return ERR_MEM;
	}
	memcpy(received_data, p->payload, p->len);
	printf("Received data: %.*s\n", p->len, received_data);
	tcp_recved(pcb, p->len);
	pbuf_free(p);

	// Echo back the received data
	if (tcp_sndbuf(pcb) < len) {
		printf("Not enough send buffer space, closing connection\n");
		return tcp_close(pcb);
	}
	err = tcp_write(pcb, received_data, len, TCP_WRITE_FLAG_COPY);
	if (err != ERR_OK) {
		printf("tcp_write failed: %d\n", err);
	}

	tcp_close(pcb);
	printf("Connection closed\n");

	return ERR_OK;
}

err_t server_sent(void *arg, struct tcp_pcb *pcb, u16_t len) {
	LWIP_UNUSED_ARG(arg);
	printf("server_sent: %d bytes acknowledged\n", len);
	return ERR_OK;
}

err_t server_poll(void *arg, struct tcp_pcb *pcb) {
	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(pcb);
	printf("\rserver_poll called");
	return ERR_OK;
}

err_t server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
	if (err != ERR_OK || client_pcb == NULL) {
		printf("server_accept: error %d\n", err);
		return ERR_VAL;
	}
	printf("accepted client connection\n");

	tcp_nagle_disable(client_pcb);
	tcp_arg(client_pcb, NULL);
	tcp_recv(client_pcb, server_recv);
	tcp_sent(client_pcb, server_sent);
	tcp_err(client_pcb, NULL);
	tcp_poll(client_pcb, server_poll, 4);
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

	// const auto hostname = "pico.local";
	// netif_set_hostname(&cyw43_state.netif[CYW43_ITF_STA], hostname);
	char hostname[sizeof(CYW43_HOST_NAME) + 4];
	memcpy(&hostname[0], CYW43_HOST_NAME, sizeof(CYW43_HOST_NAME) - 1);
	get_mac_ascii(CYW43_HAL_MAC_WLAN0, 8, 4,
								&hostname[sizeof(CYW43_HOST_NAME) - 1]);
	hostname[sizeof(hostname) - 1] = '\0';
	netif_set_hostname(&cyw43_state.netif[CYW43_ITF_STA], hostname);
	printf("tmp: mdns host name %s.local\n", hostname);

	cyw43_arch_enable_sta_mode();
	const auto ssid = get_config_value("ssid");
	const auto password = get_config_value("password");
	if (cyw43_arch_wifi_connect_timeout_ms(ssid.c_str(), password.c_str(),
																				 CYW43_AUTH_WPA2_AES_PSK,
																				 15000) != PICO_ERROR_NONE) {
		handle_error("Failed to connect to WiFi");
		return 1;
	}

	// const auto ipv4 = get_config_value("ipv4");
	// ip_addr_t ipaddr, netmask, gw;
	// ip4addr_aton(ipv4.c_str(), &(netif_list->ip_addr));
	// ip4addr_aton("255.255.255.0", &(netif_list->netmask));
	// ip4addr_aton("192.168.1.1", &(netif_list->gw));

	auto self_ip = ip4addr_ntoa(netif_ip4_addr(netif_list));
	printf("Connected to WiFi, My IP: %s\n", self_ip);

	// const auto ip = "192.168.1.52";
	// auto pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
	// if (!pcb) {
	// 	handle_error("Failed to create TCP PCB");
	// 	return 1;
	// }

#if LWIP_MDNS_RESPONDER
	cyw43_arch_lwip_begin();
	mdns_resp_init();
	printf("mdns host name %s.local\n", hostname);
	mdns_resp_add_netif(&cyw43_state.netif[CYW43_ITF_STA], hostname);
	mdns_resp_add_service(&cyw43_state.netif[CYW43_ITF_STA], "pico_server",
												"_tcp", DNSSD_PROTO_TCP, 8000, srv_txt, 0);
	cyw43_arch_lwip_end();
#endif

	auto pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
	if (!pcb) {
		handle_error("Failed to create TCP PCB");
		return 1;
	}
	if (tcp_bind(pcb, NULL, 8000) != ERR_OK) {
		handle_error("Failed to bind TCP PCB");
		return 1;
	}

	auto server_pcb = tcp_listen_with_backlog(pcb, 3);
	if (!server_pcb) {
		if (pcb) {
			tcp_close(pcb);
		}
		handle_error("Failed to listen on TCP PCB");
		return 1;
	}

	tcp_accept(server_pcb, server_accept);

	while (true) {
		toggle_led();
		sleep_ms(1000);
	}

	cyw43_arch_lwip_end();
}
