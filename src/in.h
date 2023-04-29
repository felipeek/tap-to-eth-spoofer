#pragma once
#include <stdint.h>

int32_t in_spoof_packet(const uint8_t* data, int32_t length, uint8_t* buffer);