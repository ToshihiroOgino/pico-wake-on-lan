#ifndef NTP_H
#define NTP_H

#include "lwip/ip.h"
#include "lwip/udp.h"
#include "pico/cyw43_arch.h"

typedef struct {
	ip_addr_t ntp_server_address;
	udp_pcb *pcb;
	async_at_time_worker_t request_worker;
	time_t ntp_result_utc;
	absolute_time_t ntp_succeed_at;
} ntp_client_t;

extern ntp_client_t *ntp_client;

int run_ntp_client();
time_t get_posix_time_utc();

#endif // NTP_H
