#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include "out.h"
#include "packet.h"
#include "tap.h"
#include "util.h"

void out_spoof_init(Out_Spoofing_Descriptor* osd, uint32_t spoof_ip_address, uint8_t spoof_mac_address[6]) {
	osd->spoof_ip_address = spoof_ip_address;
	memcpy(osd->spoof_mac_address, spoof_mac_address, 6);
}

static int32_t out_spoof_packet_arp(Out_Spoofing_Descriptor* osd, const uint8_t* sent_packet_data, int32_t sent_packet_length,
		const uint8_t* received_packet_data, int32_t received_packet_length, uint8_t* buffer) {

	struct arphdr* sent_packet_arp_header = packet_arp_get_header(sent_packet_data);
	uint32_t sent_packet_arp_target_ip = packet_arp_get_target_protocol_address(sent_packet_arp_header);

	struct arphdr* received_packet_arp_header = packet_arp_get_header(received_packet_data);
	uint32_t received_packet_arp_sender_ip = packet_arp_get_sender_protocol_address(received_packet_arp_header);
	
	if (received_packet_arp_sender_ip != sent_packet_arp_target_ip) {
		uint8_t ip_bufA[32];
		uint8_t ip_bufB[32];
		util_ip_address_to_str(received_packet_arp_sender_ip, ip_bufA);
		util_ip_address_to_str(sent_packet_arp_target_ip, ip_bufB);
		printf("not response, skipping (want %s but got %s)\n", ip_bufB, ip_bufA);
		//packet_print(response);
		
		return -1;
	}
	
	printf("Found response!\n");

	// Clone original packet
	memcpy(buffer, received_packet_data, received_packet_length);

	struct ether_header* ether_header = packet_ethernet_get_header(buffer);
	struct arphdr* arp_header = packet_arp_get_header(buffer);

	char* new_spoofed_dst_mac = osd->spoof_mac_address;
	packet_arp_set_target_hw_address(arp_header, new_spoofed_dst_mac);
	packet_ethernet_set_dst_mac_addr(ether_header, new_spoofed_dst_mac);

	uint32_t new_spoofed_dst_ip = osd->spoof_ip_address;
	packet_arp_set_target_protocol_address(arp_header, new_spoofed_dst_ip);

	uint8_t ip_buf[32];
	util_ip_address_to_str(new_spoofed_dst_ip, ip_buf);
	printf("new spoofed IP: %s\n", ip_buf);

	return received_packet_length;
}

int32_t out_spoof_packet(Out_Spoofing_Descriptor* osd, const uint8_t* sent_packet_data, int32_t sent_packet_length,
		const uint8_t* received_packet_data, int32_t received_packet_length, uint8_t* buffer) {
	struct ether_header* sent_packet_ether_header = packet_ethernet_get_header(sent_packet_data);
	uint16_t sent_packet_ether_type = packet_ethernet_get_type(sent_packet_ether_header);
	
	struct ether_header* received_packet_ether_header = packet_ethernet_get_header(received_packet_data);
	uint16_t received_packet_ether_type = packet_ethernet_get_type(received_packet_ether_header);

	// If the packets have different type, this is not our response.
	if (sent_packet_ether_type != received_packet_ether_type) {
		return -1;
	}

	switch (received_packet_ether_type) {
		case ETHERTYPE_ARP: {
			return out_spoof_packet_arp(osd, sent_packet_data, sent_packet_length, received_packet_data,
				received_packet_length, buffer);
		} break;
	}

	printf("Cannot handle ethertype - dropping packet.\n");
	return -1;
}