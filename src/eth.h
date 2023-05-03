#pragma once
#include "common.h"

typedef struct {
	int sockfd;
	int ifindex;
	uint8_t mac_address[6];
	uint32_t ip_address;
	uint32_t netmask;
	uint32_t default_gateway;
} Eth_Descriptor;

int eth_init(Eth_Descriptor* eth);
int eth_send(Eth_Descriptor* eth, const uint8_t* packet_data, int32_t packet_size);
int32_t eth_receive(Eth_Descriptor* eth, uint8_t* buffer, uint32_t buffer_size);
void eth_release(Eth_Descriptor* eth);
uint32_t eth_get_ip_address(Eth_Descriptor* eth);
void eth_get_mac_address(Eth_Descriptor* eth, uint8_t mac_address[6]);
uint32_t eth_get_netmask_address(Eth_Descriptor* eth);
uint32_t eth_get_default_gateway(Eth_Descriptor* eth);