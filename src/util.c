#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include "util.h"

#define PROC_NET_ROUTE "/proc/net/route"

uint32_t util_ip_address_str_to_uint32(const uint8_t* ip) {
	return inet_addr(ip);
}

void util_ip_address_str_to_buf(const uint8_t* ip, char bytes[4]) {
	uint32_t ip_uint32 = util_ip_address_str_to_uint32(ip);
	return util_ip_address_to_buf(ip_uint32, bytes);
}

void util_ip_address_to_buf(uint32_t ip, char bytes[4]) {
	bytes[0] = ip & 0xFF;
	bytes[1] = (ip >> 8) & 0xFF;
	bytes[2] = (ip >> 16) & 0xFF;
	bytes[3] = (ip >> 24) & 0xFF;
}

void util_ip_address_to_str(uint32_t ip, char buf[32]) {
	unsigned char bytes[4];
	util_ip_address_to_buf(ip, bytes);
	sprintf(buf, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);
}

void util_mac_address_to_str(const uint8_t mac_addr[6], uint8_t mac_addr_str[32]) {
	sprintf(mac_addr_str, "%02x : %02x : %02x : %02x : %02x : %02x",
		mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
}

// read and parse /proc/net/route to extract the default gateway
// ugly code, make it better later
int util_get_default_gateway(const char* if_name, uint32_t* gateway) {
    FILE* fp = fopen(PROC_NET_ROUTE, "r");

    if (fp == NULL) {
        perror("fail to open /proc/net/route (fopen)");
        return -1;
    }

    char buf[512];
    int found = 0;

    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (strstr(buf, if_name) != NULL && strstr(buf, "00000000") != NULL) {
			int found_space_or_tab = 0;
			int idx = 0;

			// skip Destination
			while (1) {
				if (!found_space_or_tab) {
					if (buf[idx] == ' ' || buf[idx] == '\t') {
						found_space_or_tab = 1;
					}
				} else {
					if (buf[idx] != ' ' && buf[idx] != '\t') {
						break;
					}
				}

				++idx;
			}

			found_space_or_tab = 0;
			// find Gateway
			while (1) {
				if (!found_space_or_tab) {
					if (buf[idx] == ' ' || buf[idx] == '\t') {
						found_space_or_tab = 1;
					}
				} else {
					if (buf[idx] != ' ' && buf[idx] != '\t') {
						break;
					}
				}

				++idx;
			}

            // Found default gateway entry for specified interface
            char gw_str[9];
            strncpy(gw_str, &buf[idx], 8);
            gw_str[8] = '\0';
            *gateway = (uint32_t)strtoul(gw_str, NULL, 16);
            found = 1;
            break;
        }
    }

    fclose(fp);

    if (!found) {
        fprintf(stderr, "Failed to find default gateway for interface [%s]\n", if_name);
        return -1;
    }

    return 0;
}