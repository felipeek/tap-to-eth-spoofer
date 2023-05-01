#pragma once
#include <stdint.h>

typedef struct {
	uint32_t spoof_ip_address;
	
	// This is the spoofed mac address that is injected in the packets when they are sent to the eth interface
	// Linux allows us to write arbitrary mac addresses and, when reading the interface, it is also possible to see all traffic,
	// even if the dst mac address of the packets are different from the mac address of the interface
	// -> If this is set to the same mac address of the eth interface, then the mac address of the packets will not be spoofed
	uint8_t spoof_mac_address[6];
} In_Spoofing_Descriptor;

void in_spoof_init(In_Spoofing_Descriptor* isd, uint32_t spoof_ip_address, uint8_t spoof_mac_address[6]);
int32_t in_spoof_packet(In_Spoofing_Descriptor* isd, const uint8_t* data, int32_t length, uint8_t* buffer);