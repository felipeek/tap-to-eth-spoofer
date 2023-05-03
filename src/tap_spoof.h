#pragma once
#include "common.h"

typedef struct {
} Tap_Spoofing_Descriptor;

void tap_spoof_init(Tap_Spoofing_Descriptor* tsd);
int32_t tap_spoof_packet(Tap_Spoofing_Descriptor* tsd, 
		const uint8_t* received_packet_data, int32_t received_packet_length, uint8_t* buffer);