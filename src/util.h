#pragma once

#include <stdint.h>

uint32_t util_ip_address_str_to_uint32(const uint8_t* ip);
void util_ip_address_str_to_buf(const uint8_t* ip, char bytes[4]);
void util_ip_address_to_buf(uint32_t ip, char bytes[4]);
void util_ip_address_to_str(uint32_t ip, char buf[32]);
void util_mac_address_to_str(const uint8_t mac_addr[6], uint8_t mac_addr_str[32]);