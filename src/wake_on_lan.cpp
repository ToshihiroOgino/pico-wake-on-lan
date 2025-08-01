#include "wake_on_lan.h"

#include "stdio.h"
#include <cstring>
#include <iomanip>
#include <string>
#include <vector>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "lwip/pbuf.h"
#include "lwip/udp.h"

using namespace std;

#define WOL_PORT 9

char *mac_address;

void set_mac_address(const char *mac) {
	if (mac_address) {
		free(mac_address);
	}
	mac_address = strdup(mac);
}

vector<uint8_t> mac_string_to_bytes(const string &mac_str) {
	vector<uint8_t> mac_bytes;
	stringstream ss(mac_str);
	string segment;
	int value;

	while (getline(ss, segment, ':')) {
		istringstream(segment) >> hex >> value;
		mac_bytes.push_back(static_cast<uint8_t>(value));
	}
	return mac_bytes;
}

void send_magic_packet() {
	if (!mac_address) {
		printf("MAC address not set\n");
	}
	printf("Sending magic packet to %s\n", mac_address);
	struct udp_pcb *pcb = udp_new();
	if (!pcb) {
		printf("Failed to create UDP PCB\n");
		return;
	}
	const vector<uint8_t> mac_bytes = mac_string_to_bytes(mac_address);

	vector<uint8_t> magic_packet(6, 0xFF);
	for (int i = 0; i < 16; ++i) {
		magic_packet.insert(magic_packet.end(), mac_bytes.begin(), mac_bytes.end());
	}
	magic_packet.push_back(0x00);
	magic_packet.push_back(0x00);

	const auto packet_len = magic_packet.size();
	struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, (u16_t)packet_len, PBUF_RAM);
	if (!p) {
		printf("Failed to allocate pbuf\n");
		udp_remove(pcb);
		return;
	}

	memcpy(p->payload, magic_packet.data(), packet_len);
	if (udp_sendto(pcb, p, IP_ADDR_BROADCAST, WOL_PORT) != ERR_OK) {
		printf("Failed to send magic packet\n");
	} else {
		printf("Magic packet sent successfully\n");
	}
	pbuf_free(p);
	udp_remove(pcb);
}
