#include "xip_config.h"
#include "wifi.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>

using namespace std;

static map<string, string> config_values;

int load_config() {
	char *config_ptr = (char *)CONFIG_ADDRESS;
	istringstream iss(config_ptr);
	string line;
	while (getline(iss, line)) {
		auto separator_pos = line.find('=');
		if (separator_pos != string::npos) {
			string key = line.substr(0, separator_pos);
			transform(key.begin(), key.end(), key.begin(), ::tolower);
			string value = line.substr(separator_pos + 1);
			config_values[key] = value;
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
