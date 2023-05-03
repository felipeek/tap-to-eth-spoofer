#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "in.h"
#include "packet.h"
#include "util.h"
#include "tap.h"

void in_spoof_init(In_Spoofing_Descriptor* isd) {
}

int32_t in_spoof_packet(In_Spoofing_Descriptor* isd, const uint8_t* data, int32_t length, uint8_t* buffer) {
	// Clone original packet
	memcpy(buffer, data, length);
	return length;
}