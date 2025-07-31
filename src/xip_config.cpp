#include "xip_config.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>

#include "error_handler.h"

using namespace std;

static map<string, string> config_values;

string trim_space(const string &str) {
	size_t first = str.find_first_not_of(' ');
	if (first == string::npos)
		return "";
	size_t last = str.find_last_not_of(' ');
	return str.substr(first, (last - first + 1));
}

void load_config() {
	char *config_ptr = (char *)CONFIG_ADDRESS;
	istringstream iss(config_ptr);
	string line;
	while (getline(iss, line)) {
		const size_t separator_pos = line.find('=');
		if (separator_pos != string::npos) {
			string key = line.substr(0, separator_pos);
			transform(key.begin(), key.end(), key.begin(), ::tolower);
			string value = line.substr(separator_pos + 1);
			config_values[trim_space(key)] = trim_space(value);
		} else {
			handle_error(("Invalid config line: " + line).c_str());
		}
	}
	printf("Configuration loaded successfully.\n");	return;
}

string get_config_value(const string &key) {
	auto it = config_values.find(key);
	if (it != config_values.end()) {
		return it->second;
	}
	return "";
}
