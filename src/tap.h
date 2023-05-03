#pragma once
#include "common.h"

typedef struct {
	int fd;
	uint32_t ip_address;
	uint8_t mac_address[6];
} Tap_Descriptor;

int tap_init(Tap_Descriptor* tap, uint8_t* interface_name, uint32_t ip_address, uint32_t netmask,
		uint32_t default_gateway_ip_address, uint8_t mac_address[6]);
int tap_send(Tap_Descriptor* tap, const uint8_t* packet_data, int32_t packet_size);
int32_t tap_receive(Tap_Descriptor* tap, uint8_t* buffer, uint32_t buffer_size);
void tap_release(Tap_Descriptor* tap);
uint32_t tap_get_ip_address(Tap_Descriptor* eth);
void tap_get_mac_address(Tap_Descriptor* eth, uint8_t mac_address[6]);