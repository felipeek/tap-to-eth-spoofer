#include "packet.h"
#include <stdio.h>

struct ether_header* packet_ethernet_get_header(const unsigned char* packet) {
	return (struct ether_header*) packet;
}

uint16_t packet_ethernet_get_type(const struct ether_header* ether_header) {
	return (ether_header->ether_type >> 8) | (ether_header->ether_type << 8); // fix endianness
}

const uint8_t* packet_ethernet_get_src_mac_addr(const struct ether_header* ether_header) {
	return ether_header->ether_shost;
}

const uint8_t* packet_ethernet_get_dst_mac_addr(const struct ether_header* ether_header) {
	return ether_header->ether_dhost;
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

	printf("\t- Src Mac:\t[%02x : %02x : %02x : %02x : %02x : %02x]\n",
		mac_src_addr[0], mac_src_addr[1], mac_src_addr[2], mac_src_addr[3], mac_src_addr[4], mac_src_addr[5]);
	printf("\t- Dst MAC:\t[%02x : %02x : %02x : %02x : %02x : %02x]\n",
		mac_dst_addr[0], mac_dst_addr[1], mac_dst_addr[2], mac_dst_addr[3], mac_dst_addr[4], mac_dst_addr[5]);
}

struct iphdr* packet_ip_get_header(const unsigned char* packet) {
	return (struct iphdr*) (packet + sizeof(struct ether_header));
}

uint32_t packet_ip_get_src_ip(const struct iphdr* ip_header) {
	return ip_header->saddr;
}

uint32_t packet_ip_get_dst_ip(const struct iphdr* ip_header) {
	return ip_header->daddr;
}

static void ip_to_str(uint32_t ip, char buf[32]) {
	unsigned char bytes[4];
	bytes[0] = ip & 0xFF;
	bytes[1] = (ip >> 8) & 0xFF;
	bytes[2] = (ip >> 16) & 0xFF;
	bytes[3] = (ip >> 24) & 0xFF;
	sprintf(buf, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);
}

void packet_ip_print(const struct iphdr* ip_header) {
	uint32_t src_ip = packet_ip_get_src_ip(ip_header);
	uint32_t dst_ip = packet_ip_get_dst_ip(ip_header);

	char src_ip_buf[32];
	char dst_ip_buf[32];
	ip_to_str(src_ip, src_ip_buf);
	ip_to_str(dst_ip, dst_ip_buf);
	
	printf("IP Header\n");
	printf("\t- Src IP:\t[%s]\n", src_ip_buf);
	printf("\t- Dst IP:\t[%s]\n", dst_ip_buf);
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
		default: {
			printf("I don't know how to print this ether type.\n");
		} break;
	}
}