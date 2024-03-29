#pragma once
#include "common.h"

uint32_t util_ip_address_str_to_uint32(const uint8_t* ip);
void util_ip_address_str_to_buf(const uint8_t* ip, char bytes[4]);
void util_ip_address_to_buf(uint32_t ip, char bytes[4]);
void util_ip_address_to_str(uint32_t ip, char buf[32]);
void util_mac_address_to_str(const uint8_t mac_addr[6], uint8_t mac_addr_str[32]);
void util_mac_address_str_to_buf(const uint8_t mac_addr_str[32], uint8_t mac_addr[6]);
int util_get_default_gateway(const char* if_name, uint32_t* gateway);
int util_is_ip_within_subnet(uint32_t ip, uint32_t netmask, uint32_t netip);