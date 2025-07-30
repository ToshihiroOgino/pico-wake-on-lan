#ifndef XIP_CONFIG_H
#define XIP_CONFIG_H

#include <string>

#ifndef CONFIG_ADDRESS
#define CONFIG_ADDRESS 0x10100000
#endif

void load_config();
/**
 * @param key lowercase
 * @return value or "" if not found
 */
std::string get_config_value(const std::string &key);

#endif // XIP_CONFIG_H
