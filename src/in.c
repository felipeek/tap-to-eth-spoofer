#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "in.h"
#include "packet.h"
#include "tap.h"

#define ETH_INTERFACE_IP "192.168.0.2"

// This is the spoofed mac address that is injected in the packets when they are sent to the eth interface
// Linux allows us to write arbitrary mac addresses and, when reading the interface, it is also possible to see all traffic,
// even if the dst mac address of the packets are different from the mac address of the interface
// -> If this is set to the same mac address of the eth interface, then the mac address of the packets will not be spoofed
static uint8_t SPOOFED_SRC_MAC_ADDRESS[6] = { 0x54, 0xb2, 0x03, 0x04, 0x77, 0xe0 };

static uint8_t* in_spoof_packet_arp(const uint8_t* data, int32_t length, int32_t* new_length) {
	printf("Spoofing ARP packet's MAC and IP...\n");

	// Clone original packet
	uint8_t* new_packet = malloc(length);
	if (new_packet == NULL) {
		perror("unable to allocate memory for new packet (malloc)");
		return NULL;
	}
	memcpy(new_packet, data, length);
	
	struct ether_header* ether_header = packet_ethernet_get_header(new_packet);
	const uint8_t* src_mac = packet_ethernet_get_src_mac_addr(ether_header);
	const uint8_t* dst_mac = packet_ethernet_get_dst_mac_addr(ether_header);
	struct arphdr* arp_header = packet_arp_get_header(new_packet);

	// todo: get this mac automatically , this is from eth (eno1)
	packet_arp_set_sender_hw_address(arp_header, SPOOFED_SRC_MAC_ADDRESS);
	packet_ethernet_set_src_mac_addr(ether_header, SPOOFED_SRC_MAC_ADDRESS);

	//unsigned char new_spoofed_src_ip_arr[4] = { 192, 168, 0, 2 };
	unsigned char new_spoofed_src_ip_arr[4];
	packet_ip_address_str_to_buf(ETH_INTERFACE_IP, new_spoofed_src_ip_arr);
	uint32_t new_spoofed_src_ip = new_spoofed_src_ip_arr[0] | (new_spoofed_src_ip_arr[1] << 8) |
		(new_spoofed_src_ip_arr[2] << 16) | (new_spoofed_src_ip_arr[3] << 24);

	packet_arp_set_sender_protocol_address(arp_header, new_spoofed_src_ip);

	uint8_t ip_buf[32];
	packet_ip_address_to_str(new_spoofed_src_ip, ip_buf);
	printf("new spoofed IP: %s\n", ip_buf);

	*new_length = length;
	return new_packet;
}

uint8_t* in_spoof_packet(const uint8_t* data, int32_t length, int32_t* new_length) {
	struct ether_header* ether_header = packet_ethernet_get_header(data);
	uint16_t ether_type = packet_ethernet_get_type(ether_header);

	switch (ether_type) {
		case ETHERTYPE_ARP: {
			return in_spoof_packet_arp(data, length, new_length);
		} break;
	}
	
	printf("Cannot handle ethertype - dropping packet.\n");
	return NULL;
}