#include <stdio.h>
#include <string.h>
#include "packet.h"
#include "util.h"

#define INVERT_ENDIANESS_16(X) ((X >> 8) | (X << 8))

struct ether_header* packet_ethernet_get_header(const unsigned char* packet) {
	return (struct ether_header*) packet;
}

uint16_t packet_ethernet_get_type(const struct ether_header* ether_header) {
	return INVERT_ENDIANESS_16(ether_header->ether_type);
}

const uint8_t* packet_ethernet_get_src_mac_addr(const struct ether_header* ether_header) {
	return ether_header->ether_shost;
}

void packet_ethernet_set_src_mac_addr(struct ether_header* ether_header, uint8_t new_src_mac_addr[6]) {
	uint8_t* src_mac_addr = ether_header->ether_shost;
	memcpy(src_mac_addr, new_src_mac_addr, 6);
}

const uint8_t* packet_ethernet_get_dst_mac_addr(const struct ether_header* ether_header) {
	return ether_header->ether_dhost;
}

void packet_ethernet_set_dst_mac_addr(struct ether_header* ether_header, uint8_t new_dst_mac_addr[6]) {
	uint8_t* dst_mac_addr = ether_header->ether_dhost;
	memcpy(dst_mac_addr, new_dst_mac_addr, 6);
}

void packet_ethernet_print(const struct ether_header* ether_header) {
	printf("Ethernet Header\n");

	uint16_t type = packet_ethernet_get_type(ether_header);
	const uint8_t* mac_src_addr = packet_ethernet_get_src_mac_addr(ether_header);
	const uint8_t* mac_dst_addr = packet_ethernet_get_dst_mac_addr(ether_header);

	switch (type) {
		case ETHERTYPE_PUP: printf("\t- Type:\t[PUP]\n"); break;
		case ETHERTYPE_SPRITE: printf("\t- Type:\t[SPRITE]\n"); break;
		case ETHERTYPE_IP: printf("\t- Type:\t[IP]\n"); break;
		case ETHERTYPE_ARP: printf("\t- Type:\t[ARP]\n"); break;
		case ETHERTYPE_REVARP: printf("\t- Type:\t[REVARP]\n"); break;
		case ETHERTYPE_AT: printf("\t- Type:\t[AT]\n"); break;
		case ETHERTYPE_AARP: printf("\t- Type:\t[AARP]\n"); break;
		case ETHERTYPE_VLAN: printf("\t- Type:\t[VLAN]\n"); break;
		case ETHERTYPE_IPX: printf("\t- Type:\t[IPX]\n"); break;
		case ETHERTYPE_IPV6: printf("\t- Type:\t[IPV6]\n"); break;
		case ETHERTYPE_LOOPBACK: printf("\t- Type:\t[LOOPBACK]\n"); break;
	}

	uint8_t mac_src_addr_str[32];
	uint8_t mac_dst_addr_str[32];

	util_mac_address_to_str(mac_src_addr, mac_src_addr_str);
	util_mac_address_to_str(mac_dst_addr, mac_dst_addr_str);
	printf("\t- Src Mac:\t[%s]\n", mac_src_addr_str);
	printf("\t- Dst Mac:\t[%s]\n", mac_dst_addr_str);
}

struct iphdr* packet_ip_get_header(const unsigned char* packet) {
	return (struct iphdr*) (packet + sizeof(struct ether_header));
}

uint32_t packet_ip_get_src_ip(const struct iphdr* ip_header) {
	return ip_header->saddr;
}

void packet_ip_set_src_ip(struct iphdr* ip_header, uint32_t new_src_ip) {
	ip_header->saddr = new_src_ip;
}

uint32_t packet_ip_get_dst_ip(const struct iphdr* ip_header) {
	return ip_header->daddr;
}

void packet_ip_print(const struct iphdr* ip_header) {
	uint32_t src_ip = packet_ip_get_src_ip(ip_header);
	uint32_t dst_ip = packet_ip_get_dst_ip(ip_header);

	char src_ip_buf[32];
	char dst_ip_buf[32];
	util_ip_address_to_str(src_ip, src_ip_buf);
	util_ip_address_to_str(dst_ip, dst_ip_buf);
	
	printf("IP Header\n");
	printf("\t- Src IP:\t[%s]\n", src_ip_buf);
	printf("\t- Dst IP:\t[%s]\n", dst_ip_buf);
}

struct arphdr* packet_arp_get_header(const unsigned char* packet) {
	return (struct arphdr*) (packet + sizeof(struct ether_header));
}

uint16_t packet_arp_get_hardware_type(const struct arphdr* arp_header) {
	return INVERT_ENDIANESS_16(arp_header->ar_hrd);
}

uint16_t packet_arp_get_protocol_type(const struct arphdr* arp_header) {
	return INVERT_ENDIANESS_16(arp_header->ar_pro);
}

uint16_t packet_arp_get_opcode(const struct arphdr* arp_header) {
	return INVERT_ENDIANESS_16(arp_header->ar_op);
}

const unsigned char* packet_arp_get_sender_hw_address(const struct arphdr* arp_header) {
	return (const unsigned char*)(arp_header + 1); // 6 bytes
}

void packet_arp_set_sender_hw_address(struct arphdr* arp_header, char new_sender_hw_addr[6]) {
	unsigned char* sender_hw_addr = (unsigned char*)(arp_header + 1); // 6 bytes
	memcpy(sender_hw_addr, new_sender_hw_addr, 6);
}

uint32_t packet_arp_get_sender_protocol_address(const struct arphdr* arp_header) {
	return *(uint32_t*)((const unsigned char*)(arp_header + 1) + 6); // 4 bytes
}

void packet_arp_set_sender_protocol_address(const struct arphdr* arp_header, uint32_t new_sender_protocol_addr) {
	uint32_t* sender_protocol_addr = (uint32_t*)((const unsigned char*)(arp_header + 1) + 6); // 4 bytes
	*sender_protocol_addr = new_sender_protocol_addr;
}

const unsigned char* packet_arp_get_target_hw_address(const struct arphdr* arp_header) {
	return (const unsigned char*)(arp_header + 1) + 10; // 6 bytes
}

void packet_arp_set_target_hw_address(struct arphdr* arp_header, char new_target_hw_addr[6]) {
	unsigned char* target_hw_addr = (unsigned char*)(arp_header + 1) + 10; // 6 bytes
	memcpy(target_hw_addr, new_target_hw_addr, 6);
}

uint32_t packet_arp_get_target_protocol_address(const struct arphdr* arp_header) {
	return *(uint32_t*)((const unsigned char*)(arp_header + 1) + 16); // 4 bytes
}

void packet_arp_set_target_protocol_address(const struct arphdr* arp_header, uint32_t new_target_protocol_addr) {
	uint32_t* target_protocol_addr = (uint32_t*)((const unsigned char*)(arp_header + 1) + 16); // 4 bytes
	*target_protocol_addr = new_target_protocol_addr;
}

void packet_arp_print(const struct arphdr* arp_header) {
	uint16_t hardware_type = packet_arp_get_hardware_type(arp_header);
	uint16_t protocol_type = packet_arp_get_protocol_type(arp_header);
	uint16_t opcode = packet_arp_get_opcode(arp_header);

	printf("\t- Hardware Type: ");
	switch (hardware_type) {
		case ARPHRD_ETHER: {
			printf("ETHER\n");
		} break;
		default: {
			printf("Unknown\t(hex %x | decimal %d)\n", hardware_type, hardware_type);
		} break;
	}

	printf("\t- Protocol Type: ");
	switch (protocol_type) {
		case 0x0800: { // todo: which header has this?
			printf("IPv4\n");
		} break;
		default: {
			printf("Unknown\t(hex %x | decimal %d)\n", protocol_type, protocol_type);
		} break;
	}

	printf("\t- OpCode: ");
	switch (opcode) {
		case ARPOP_REQUEST: {
			printf("ARP Request\n");
		} break;
		default: {
			printf("Unknown\t(hex %x | decimal %d)\n", opcode, opcode);
		} break;
	}

	const unsigned char* sender_hardware_addr = packet_arp_get_sender_hw_address(arp_header);
	uint32_t sender_protocol_addr = packet_arp_get_sender_protocol_address(arp_header);
	const unsigned char* target_hardware_addr = packet_arp_get_target_hw_address(arp_header);
	uint32_t target_protocol_addr = packet_arp_get_target_protocol_address(arp_header);

	char sender_protocol_buf[32];
	char target_protocol_buf[32];
	util_ip_address_to_str(sender_protocol_addr, sender_protocol_buf);
	util_ip_address_to_str(target_protocol_addr, target_protocol_buf);

	printf("\t- Sender Hardware Addr:\t[%02x : %02x : %02x : %02x : %02x : %02x]\n",
		sender_hardware_addr[0], sender_hardware_addr[1], sender_hardware_addr[2], sender_hardware_addr[3], sender_hardware_addr[4], sender_hardware_addr[5]);
	printf("\t- Sender Protocol Addr:\t[%s]\n", sender_protocol_buf);
	printf("\t- Target Hardware Addr:\t[%02x : %02x : %02x : %02x : %02x : %02x]\n",
		target_hardware_addr[0], target_hardware_addr[1], target_hardware_addr[2], target_hardware_addr[3], target_hardware_addr[4], target_hardware_addr[5]);
	printf("\t- Target Protocol Addr:\t[%s]\n", target_protocol_buf);
}

void packet_print(const unsigned char* packet) {
	struct ether_header* ether_header = packet_ethernet_get_header(packet);
	uint16_t ether_type = packet_ethernet_get_type(ether_header);

	packet_ethernet_print(ether_header);

	switch (ether_type) {
		case ETHERTYPE_IP: {
			const struct iphdr* ip_header = packet_ip_get_header(packet);
			packet_ip_print(ip_header);
		} break;
		case ETHERTYPE_ARP: {
			const struct arphdr* arp_header = packet_arp_get_header(packet);
			packet_arp_print(arp_header);
		} break;
		default: {
			printf("I don't know how to print this ether type.\n");
		} break;
	}
}
