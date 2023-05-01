#pragma once

#include <stdint.h>

typedef struct {
	uint32_t spoof_ip_address;
	uint8_t spoof_mac_address[6];
} Out_Spoofing_Descriptor;

void out_spoof_init(Out_Spoofing_Descriptor* osd, uint32_t spoof_ip_address, uint8_t spoof_mac_address[6]);
int32_t out_spoof_packet(Out_Spoofing_Descriptor* osd, const uint8_t* sent_packet_data, int32_t sent_packet_length,
		const uint8_t* received_packet_data, int32_t received_packet_length, uint8_t* buffer);