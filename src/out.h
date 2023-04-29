#pragma once

#include <stdint.h>

uint8_t* out_spoof_packet(const uint8_t* sent_packet_data, int32_t sent_packet_length,
		const uint8_t* received_packet_data, int32_t received_packet_length, int32_t* new_length);