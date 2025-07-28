#include "xip_config.h"
#include "wifi.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>

using namespace std;

static map<string, string> config_values;

string trim_space(const string &str) {
	size_t first = str.find_first_not_of(' ');
	if (first == string::npos)
		return "";
	size_t last = str.find_last_not_of(' ');
	return str.substr(first, (last - first + 1));
}

int load_config() {
	char *config_ptr = (char *)CONFIG_ADDRESS;
	istringstream iss(config_ptr);
	string line;
	while (getline(iss, line)) {
		const size_t separator_pos = line.find('=');
		if (separator_pos != string::npos) {
			const string key = line.substr(0, separator_pos);
			transform(key.begin(), key.end(), key.begin(), ::tolower);
			const string value = line.substr(separator_pos + 1);
			config_values[trim_space(key)] = trim_space(value);
		} else {
			printf("Invalid config line: %s\n", line.c_str());
			return 1;
		}
	}
	return 0;
}

string get_config_value(const string &key) {
	auto it = config_values.find(key);
	if (it != config_values.end()) {
		return it->second;
	}
	return "";
}
