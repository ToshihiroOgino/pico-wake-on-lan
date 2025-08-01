#include "error_handler.h"

#include "hard.h"
#include "pico/stdlib.h"
#include <stdio.h>

void handle_error(const char *message) {
	while (true) {
		printf("\rError: %s", message);
		for (int i = 0; i < 10; i++) {
			printf(".");
			sleep_ms(500);
			toggle_led();
		}
	}
}
