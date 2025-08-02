#ifndef SERVER_H
#define SERVER_H

#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "pico/stdlib.h"
#include <cstring>

#include "error_handler.h"
#include "ntp.h"
#include "totp.h"
#include "wake_on_lan.h"

#define MAX_POLL_COUNT 10
#define TIMEOUT_MS 5000

typedef struct {
	absolute_time_t connected_at;
	int poll_count;
} server_state_t;

server_state_t state;

#define BUFFER_SIZE (PBUF_POOL_BUFSIZE + 1)
char received_data[BUFFER_SIZE];

err_t validate_state(struct tcp_pcb *pcb, server_state_t *state) {
	const auto epoch = get_absolute_time();
	const auto ttl = absolute_time_diff_us(state->connected_at, epoch);
	if (ttl > TIMEOUT_MS * 1000) {
		printf("Connection timed out.\n");
		return tcp_close(pcb);
	}
	if (state->poll_count >= MAX_POLL_COUNT) {
		printf("Max poll count reached.\n");
		return tcp_close(pcb);
	}
	return ERR_OK;
}

err_t server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
	if (err != ERR_OK || !p) {
		printf("server_recv: error %d\n", err);
		return ERR_VAL;
	}
	size_t len = (size_t)p->len;
	memcpy(received_data, p->payload, p->len);
	received_data[BUFFER_SIZE - 1] = '\0';
	printf("Received data: %.*s\n", p->len, received_data);
	tcp_recved(pcb, p->len);
	pbuf_free(p);

	auto is_valid = is_valid_otp(get_posix_time_utc(), received_data, len);
	if (is_valid) {
		printf("Valid OTP received: %s\n", received_data);
		send_magic_packet();
	}

	// Echo back the received data
	if (tcp_sndbuf(pcb) < len) {
		printf("Not enough send buffer space, closing connection\n");
		return tcp_close(pcb);
	}

	snprintf(received_data + len, BUFFER_SIZE - len, is_valid ? "_ok" : "_ng");
	len = strlen(received_data);
	err = tcp_write(pcb, received_data, len, TCP_WRITE_FLAG_COPY);
	if (err != ERR_OK) {
		printf("tcp_write failed: %d\n", err);
	}
	printf("Connection closed\n");
	return tcp_close(pcb);
}

err_t server_sent(void *arg, struct tcp_pcb *pcb, u16_t len) {
	LWIP_UNUSED_ARG(arg);
	printf("server_sent: %d bytes acknowledged\n", len);
	return ERR_OK;
}

err_t server_poll(void *arg, struct tcp_pcb *pcb) {
	LWIP_UNUSED_ARG(pcb);
	printf("\rserver_poll called");
	auto state = (server_state_t *)arg;
	state->poll_count++;
	return validate_state(pcb, state);
}

err_t server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
	if (err != ERR_OK || client_pcb == NULL) {
		printf("server_accept: error %d\n", err);
		return ERR_VAL;
	}
	printf("accepted client connection\n");

	state.poll_count = 0;
	state.connected_at = get_absolute_time();

	tcp_nagle_disable(client_pcb);
	tcp_arg(client_pcb, &state);
	tcp_recv(client_pcb, server_recv);
	tcp_sent(client_pcb, server_sent);
	tcp_poll(client_pcb, server_poll, 4);
	return ERR_OK;
}

void start_server(const int port) {
	auto pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
	if (!pcb) {
		handle_error("Failed to create TCP PCB");
	}
	if (tcp_bind(pcb, nullptr, port) != ERR_OK) {
		handle_error("Failed to bind TCP PCB");
	}

	auto server_pcb = tcp_listen_with_backlog(pcb, 3);
	if (!server_pcb) {
		if (pcb) {
			tcp_close(pcb);
		}
		handle_error("Failed to listen on TCP PCB");
	}

	tcp_arg(server_pcb, nullptr);
	tcp_accept(server_pcb, server_accept);
}

#endif // SERVER_H
