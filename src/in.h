#pragma once
#include <stdint.h>

uint8_t* in_spoof_packet(const uint8_t* data, int32_t length, int32_t* new_length);