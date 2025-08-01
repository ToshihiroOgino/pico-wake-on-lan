#include "totp.h"

#include "time.h"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "mbedtls/sha256.h"
#include "mbedtls/version.h"
#include "pico/stdlib.h"

using namespace std;

#define TOTP_TIME_STEP 30
#define DIGITS 6

vector<uint8_t> base32_decode(const string &encoded_str) {
	string base32_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
	vector<uint8_t> decoded_bytes;
	uint32_t buffer = 0;
	int bits_left = 0;

	for (char c : encoded_str) {
		if (c >= 'a' && c <= 'z')
			c = toupper(c);
		size_t value = base32_chars.find(c);
		if (value == string::npos)
			continue;

		buffer = (buffer << 5) | value;
		bits_left += 5;

		if (bits_left >= 8) {
			decoded_bytes.push_back((buffer >> (bits_left - 8)) & 0xFF);
			bits_left -= 8;
		}
	}
	return decoded_bytes;
}

vector<uint8_t> key_decoded;
void totp_init(const std::string &secret) {
	key_decoded = base32_decode(secret);
}

string generate_totp(time_t current_time) {
	uint64_t t = static_cast<uint64_t>(current_time) / TOTP_TIME_STEP;

	uint8_t t_bytes[8];
	for (int i = 7; i >= 0; --i) {
		t_bytes[i] = t & 0xFF;
		t >>= 8;
	}

	unsigned char hmac_result[32];
	mbedtls_sha256_context ctx;
	mbedtls_sha256_init(&ctx);
	mbedtls_sha256_starts(&ctx, 0);
	mbedtls_sha256_update(&ctx, key_decoded.data(), key_decoded.size());
	mbedtls_sha256_update(&ctx, t_bytes, 8);
	mbedtls_sha256_finish(&ctx, hmac_result);
	mbedtls_sha256_free(&ctx);

	int offset = hmac_result[31] & 0x0F;
	uint32_t truncated_hash = (hmac_result[offset] & 0x7F) << 24 |
														(hmac_result[offset + 1] & 0xFF) << 16 |
														(hmac_result[offset + 2] & 0xFF) << 8 |
														(hmac_result[offset + 3] & 0xFF);

	int power_of_10 = 1;
	for (int i = 0; i < DIGITS; ++i) {
		power_of_10 *= 10;
	}
	int otp_value = truncated_hash % power_of_10;

	string otp_str = to_string(otp_value);
	while (otp_str.length() < DIGITS) {
		otp_str = "0" + otp_str;
	}

	return otp_str;
}

bool is_valid_otp(const std::string &otp, time_t current_time) {
	if (otp.length() != DIGITS) {
		return false;
	}

	string generated_otp = generate_totp(current_time);
	return otp == generated_otp;
}
