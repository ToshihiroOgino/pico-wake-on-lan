#include "ntp.h"

#include "lwip/dns.h"
#include "pico/stdio.h"
#include "time.h"

#include "error_handler.h"

#define NTP_SERVER "pool.ntp.org"
#define NTP_MSG_LEN 48
#define NTP_PORT 123
#define NTP_DELTA 2208988800 // seconds between 1 Jan 1900 and 1 Jan 1970
#define NTP_TEST_TIME_MS (30 * 1000)
#define NTP_RESEND_TIME_MS (10 * 1000)

ntp_client_t *ntp_client = NULL;

void ntp_result(ntp_client_t *client, int status, time_t *result_utc) {
	if (status == 0 && result_utc) {
		client->ntp_succeed_at = get_absolute_time();
		client->ntp_result_utc = *result_utc;

		printf("NTP time: %04d-%02d-%02d %02d:%02d:%02d UTC\n",
					 localtime(&client->ntp_result_utc)->tm_year + 1900,
					 localtime(&client->ntp_result_utc)->tm_mon + 1,
					 localtime(&client->ntp_result_utc)->tm_mday,
					 localtime(&client->ntp_result_utc)->tm_hour,
					 localtime(&client->ntp_result_utc)->tm_min,
					 localtime(&client->ntp_result_utc)->tm_sec);
	}
	async_context_remove_at_time_worker(cyw43_arch_async_context(),
																			&client->request_worker);
	auto res = async_context_add_at_time_worker_in_ms(
			cyw43_arch_async_context(), &client->request_worker, NTP_TEST_TIME_MS);
	if (!res) {
		handle_error("Failed to add NTP request worker\n");
	}
	printf("NTP request scheduled in %d ms after\n", NTP_TEST_TIME_MS);
}

void ntp_recv(void *arg, udp_pcb *pcb, pbuf *p, const ip_addr_t *addr,
							u16_t port) {
	auto client = (ntp_client_t *)arg;
	if (!client || !p || !pcb || !addr) {
		printf("ntp_recv: invalid parameters\n");
		return;
	}
	u8_t mode = pbuf_get_at(p, 0) & 0x07;
	u8_t stratum = pbuf_get_at(p, 1);

	auto is_valid =
			(ip_addr_cmp(addr, &client->ntp_server_address) && port == NTP_PORT &&
			 p->tot_len == NTP_MSG_LEN && mode == 0x4 && stratum != 0);
	if (is_valid) {
		u8_t seconds_buf[4] = {};
		pbuf_copy_partial(p, seconds_buf, sizeof(seconds_buf), 40);
		u32_t seconds_since_1900 = seconds_buf[0] << 24 | seconds_buf[1] << 16 |
															 seconds_buf[2] << 8 | seconds_buf[3];
		u32_t seconds_since_1970 = seconds_since_1900 - NTP_DELTA;
		auto epoch = (time_t)seconds_since_1970;
		ntp_result(client, 0, &epoch);
	} else {
		printf("ntp_recv: invalid packet from %s:%d\n", ipaddr_ntoa(addr), port);
		ntp_result(client, -1, nullptr);
	}
	pbuf_free(p);
}

void ntp_request(ntp_client_t *client) {
	cyw43_arch_lwip_begin();
	auto p = pbuf_alloc(PBUF_TRANSPORT, NTP_MSG_LEN, PBUF_RAM);
	auto *req = (u8_t *)p->payload;
	memset(req, 0, NTP_MSG_LEN);
	req[0] = 0x1b;
	udp_sendto(client->pcb, p, &client->ntp_server_address, NTP_PORT);
	pbuf_free(p);
	cyw43_arch_lwip_end();
}

#define GLOBAL_IP "167.179.119.205"

void ntp_dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg) {
	auto client = (ntp_client_t *)arg;
	if (ipaddr) {
		client->ntp_server_address = *ipaddr;
		printf("NTP server resolved: %s\n", hostname);
		ntp_request(client);
	} else {
		printf("ntp_dns_found: failed to resolve hostname %s\n", NTP_SERVER);
		ntp_result(client, -1, nullptr);
	}
}

void request_worker_fn(__unused async_context_t *context,
											 async_at_time_worker_t *worker) {
	auto client = (ntp_client_t *)worker->user_data;
	if (!async_context_add_at_time_worker_in_ms(cyw43_arch_async_context(),
																							&client->resend_worker,
																							NTP_RESEND_TIME_MS)) {
		handle_error("Failed to add NTP resend worker\n");
	}
	auto res = dns_gethostbyname(NTP_SERVER, &client->ntp_server_address,
															 ntp_dns_found, client);
	if (res != ERR_OK) {
		ntp_request(client);
	} else {
		printf("DNS request failed\n");
		ntp_result(client, -1, nullptr);
	}
}

void resend_worker_fn(__unused async_context_t *context,
											async_at_time_worker_t *worker) {
	auto client = (ntp_client_t *)worker->user_data;
	printf("NTP request failed\n");
	ntp_result(client, -1, nullptr);
}

int run_ntp_client() {

	ntp_client = (ntp_client_t *)calloc(1, sizeof(ntp_client_t));
	if (!ntp_client) {
		return 1;
	}
	ntp_client->pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
	if (!ntp_client->pcb) {
		free(ntp_client);
		ntp_client = NULL;
		return 1;
	}

	udp_recv(ntp_client->pcb, ntp_recv, ntp_client);
	ntp_client->request_worker.do_work = request_worker_fn;
	ntp_client->request_worker.user_data = ntp_client;
	ntp_client->resend_worker.do_work = resend_worker_fn;
	ntp_client->resend_worker.user_data = ntp_client;

	auto res = async_context_add_at_time_worker_in_ms(
			cyw43_arch_async_context(), &ntp_client->request_worker, 0);
	if (!res) {
		handle_error("Failed to add NTP request worker\n");
	}
	return 0;
}

time_t get_time_utc() {
	if (!ntp_client) {
		return 0;
	}
	auto now = get_absolute_time();
	auto diff = absolute_time_diff_us(now, ntp_client->ntp_succeed_at);
	time_t diff_sec = diff / 1000000;
	time_t current_time = ntp_client->ntp_result_utc + diff_sec;
	return current_time;
}
