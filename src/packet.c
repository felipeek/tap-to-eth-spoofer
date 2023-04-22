#include <stdio.h>
#include <net/ethernet.h>

struct ether_header* packet_ethernet_get_header(const char* packet) {
	struct ether_header* ether_header = (struct ether_header*) packet;
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

void packet_ethernet_print(struct ether_header* ether_header) {
	printf("Ethernet Header\n");

	uint16_t type = packet_ethernet_get_type(ether_header);
	const uint8_t* mac_src_addr = packet_ethernet_get_src_mac_addr(ether_header);
	const uint8_t* mac_dst_addr = packet_ethernet_get_src_mac_addr(ether_header);

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