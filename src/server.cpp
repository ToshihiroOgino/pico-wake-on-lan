#ifndef SERVER_H
#define SERVER_H

#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "pico/stdlib.h"
#include <cstring>

#include "error_handler.h"
#include "ntp.h"
#include "totp.h"

#define BUFFER_SIZE (PBUF_POOL_BUFSIZE + 1)
char received_data[BUFFER_SIZE];

err_t server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
	if (err != ERR_OK || !p) {
		printf("server_recv: error %d\n", err);
		return ERR_VAL;
	}
	auto len = p->len;
	memcpy(received_data, p->payload, p->len);
	received_data[BUFFER_SIZE - 1] = '\0';
	printf("Received data: %.*s\n", p->len, received_data);
	tcp_recved(pcb, p->len);
	pbuf_free(p);

	if (is_valid_otp(std::string(received_data, len), get_time_utc())) {
		printf("Valid OTP received: %s\n", received_data);
	}

	// Echo back the received data
	if (tcp_sndbuf(pcb) < len) {
		printf("Not enough send buffer space, closing connection\n");
		return tcp_close(pcb);
	}
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
