#pragma once
#include <stdint.h>

typedef struct {
} In_Spoofing_Descriptor;

void in_spoof_init(In_Spoofing_Descriptor* isd);
int32_t in_spoof_packet(In_Spoofing_Descriptor* isd, const uint8_t* data, int32_t length, uint8_t* buffer);