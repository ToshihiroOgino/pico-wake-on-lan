#ifndef TOTP_H
#define TOTP_H

#include "time.h"
#include <string>
#include <vector>

extern std::vector<uint8_t> key_decoded;

void totp_init(const std::string &secret);
std::string generate_totp(time_t current_time);
bool is_valid_otp(time_t current_time, const char *received_otp,
									size_t received_length);

#endif // TOTP_H
