#include <stdio.h>
#include <arpa/inet.h>
#include "util.h"

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