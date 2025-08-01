#ifndef NET_H
#define NET_H

int connect_wifi(const char *ssid, const char *password);
int init_mdns(const char *hostname, int port);

#endif // NET_H
