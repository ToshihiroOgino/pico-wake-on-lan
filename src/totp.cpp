#include "totp.h"

#include "time.h"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "mbedtls/md.h"
#include "mbedtls/sha256.h"
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
		if (c == '=') {
			continue;
		}
		if (value == string::npos)
			continue;

		buffer = (buffer << 5) | value;
		bits_left += 5;

		if (bits_left >= 8) {
			decoded_bytes.push_back((buffer >> (bits_left - 8)) & 0xFF);
			bits_left -= 8;
		}
	}
	if (bits_left > 0) {
		decoded_bytes.push_back((buffer << (8 - bits_left)) & 0xFF);
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

	unsigned char hmac_result[32] = {};
	mbedtls_md_context_t ctx;
	mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
	mbedtls_md_init(&ctx);
	mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
	mbedtls_md_hmac_starts(&ctx, key_decoded.data(), key_decoded.size());
	mbedtls_md_hmac_update(&ctx, t_bytes, 8);
	mbedtls_md_hmac_finish(&ctx, hmac_result);
	mbedtls_md_free(&ctx);

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
