#pragma once

struct ether_header* packet_ethernet_get_header(const char* packet);
void packet_ethernet_print(const char* packet);