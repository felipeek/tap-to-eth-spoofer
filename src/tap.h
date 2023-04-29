#pragma once

#define TAP_DEVICE_FILE "/dev/net/tun"
#define TAP_INTERFACE_NAME "tap0"
#define TAP_INTERFACE_IP "192.168.42.2"
static char TAP_MAC_ADDR[6] = { 0x82, 0xa2, 0x17, 0x43, 0x15, 0xef };

typedef struct {
	int fd;
} Tap_Descriptor;

int tap_init(Tap_Descriptor* tap);
int tap_send(Tap_Descriptor* tap, const uint8_t* packet_data, int32_t packet_size);
int32_t tap_receive(Tap_Descriptor* tap, uint8_t* buffer, uint32_t buffer_size);
void tap_release(Tap_Descriptor* tap);