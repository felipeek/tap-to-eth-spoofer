#include <stdio.h>
#include <string.h>
#include "tap_spoof.h"

void tap_spoof_init(Tap_Spoofing_Descriptor* tsd) {
}

int32_t tap_spoof_packet(Tap_Spoofing_Descriptor* tsd, 
		const uint8_t* received_packet_data, int32_t received_packet_length, uint8_t* buffer) {
	memcpy(buffer, received_packet_data, received_packet_length);
	return received_packet_length;
}