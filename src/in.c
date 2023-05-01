#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "in.h"
#include "packet.h"
#include "util.h"
#include "tap.h"

void in_spoof_init(In_Spoofing_Descriptor* isd, uint32_t spoof_ip_address, uint8_t spoof_mac_address[6]) {
	isd->spoof_ip_address = spoof_ip_address;
	memcpy(isd->spoof_mac_address, spoof_mac_address, 6);
}

static int32_t in_spoof_packet_arp(In_Spoofing_Descriptor* isd, const uint8_t* data, int32_t length, uint8_t* buffer) {
	printf("Spoofing ARP packet's MAC and IP...\n");

	// Clone original packet
	memcpy(buffer, data, length);
	
	struct ether_header* ether_header = packet_ethernet_get_header(buffer);
	const uint8_t* src_mac = packet_ethernet_get_src_mac_addr(ether_header);
	const uint8_t* dst_mac = packet_ethernet_get_dst_mac_addr(ether_header);
	struct arphdr* arp_header = packet_arp_get_header(buffer);

	packet_arp_set_sender_hw_address(arp_header, isd->spoof_mac_address);
	packet_ethernet_set_src_mac_addr(ether_header, isd->spoof_mac_address);

	uint32_t new_spoofed_src_ip = isd->spoof_ip_address;

	packet_arp_set_sender_protocol_address(arp_header, new_spoofed_src_ip);

	uint8_t ip_buf[32];
	util_ip_address_to_str(new_spoofed_src_ip, ip_buf);
	printf("new spoofed IP: %s\n", ip_buf);

	return length;
}

int32_t in_spoof_packet(In_Spoofing_Descriptor* isd, const uint8_t* data, int32_t length, uint8_t* buffer) {
	struct ether_header* ether_header = packet_ethernet_get_header(data);
	uint16_t ether_type = packet_ethernet_get_type(ether_header);

	switch (ether_type) {
		case ETHERTYPE_ARP: {
			return in_spoof_packet_arp(isd, data, length, buffer);
		} break;
	}
	
	printf("Cannot handle ethertype - dropping packet.\n");
	return -1;
}