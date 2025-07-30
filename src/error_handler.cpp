#include "error_handler.h"

#include "pico/stdlib.h"
#include <stdio.h>

void handle_error(const char *message) {
	while (true) {
		printf("Error: %s\n", message);
		sleep_ms(1000);
	}
}
