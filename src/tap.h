#pragma once

#define TAP_DEVICE_FILE "/dev/net/tun"
#define TAP_INTERFACE_NAME "tap0"
#define TAP_INTERFACE_IP "192.168.42.2"
static char TAP_MAC_ADDR[6] = { 0x82, 0xa2, 0x17, 0x43, 0x15, 0xef };

int tap_init();