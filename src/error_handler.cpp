#include "error_handler.h"

#include "hard.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string>

void handle_error(const char *message) {
	const int max_msg_length = 128;
	const std::string msg(message, max_msg_length);
	while (true) {
		printf("Error: %s", msg.c_str());
		for (int i = 0; i < 10; i++) {
			toggle_led();
			printf(".");
			sleep_ms(500);
		}
		printf("\n");
	}
}
