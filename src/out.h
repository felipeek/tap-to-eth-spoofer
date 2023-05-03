#pragma once

#include <stdint.h>

typedef struct {
} Out_Spoofing_Descriptor;

void out_spoof_init(Out_Spoofing_Descriptor* osd);
int32_t out_spoof_packet(Out_Spoofing_Descriptor* osd, 
		const uint8_t* received_packet_data, int32_t received_packet_length, uint8_t* buffer);