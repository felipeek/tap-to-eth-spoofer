#pragma once

typedef struct {
	int fd;
} Tap_Descriptor;

int tap_init(Tap_Descriptor* tap);
int tap_send(Tap_Descriptor* tap, const uint8_t* packet_data, int32_t packet_size);
int32_t tap_receive(Tap_Descriptor* tap, uint8_t* buffer, uint32_t buffer_size);
void tap_release(Tap_Descriptor* tap);
uint32_t tap_get_ip_address(Tap_Descriptor* eth);
void tap_get_mac_address(Tap_Descriptor* eth, uint8_t mac_address[6]);