#pragma once

typedef struct {
	int sockfd;
	int ifindex;
} Eth_Descriptor;

int eth_init(Eth_Descriptor* eth);
int eth_send(Eth_Descriptor* eth, const uint8_t* packet_data, int32_t packet_size);
int32_t eth_receive(Eth_Descriptor* eth, uint8_t* buffer, uint32_t buffer_size);
void eth_release(Eth_Descriptor* eth);