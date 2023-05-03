#include <string.h>
#include "eth_spoof.h"

void eth_spoof_init(Eth_Spoofing_Descriptor* esd) {
}

int32_t eth_spoof_packet(Eth_Spoofing_Descriptor* esd, const uint8_t* data, int32_t length, uint8_t* buffer) {
	// Clone original packet
	memcpy(buffer, data, length);
	return length;
}