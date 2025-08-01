#ifndef WAKE_ON_LAN_H
#define WAKE_ON_LAN_H

extern char *mac_address;

void set_mac_address(const char *mac);
void send_magic_packet();

#endif // WAKE_ON_LAN_H
