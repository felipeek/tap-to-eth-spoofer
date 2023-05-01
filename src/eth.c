#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/if_tun.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>

#include "eth.h"
#include "packet.h"
#include "util.h"

#define ETH_INTERFACE_NAME "eno1"

int eth_init(Eth_Descriptor* eth) {
	// Open a socket to send packets to interact with eth interface
	eth->sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (eth->sockfd < 0) {
		perror("fail to create socket for eth interface (socket)");
		return -1;
	}

	eth->ifindex = if_nametoindex(ETH_INTERFACE_NAME);
	
	struct ifreq ifr;
	strncpy(ifr.ifr_ifrn.ifrn_name, ETH_INTERFACE_NAME, IFNAMSIZ);
	
	// Get eth interface MAC address
	if (ioctl(eth->sockfd, SIOCGIFHWADDR, &ifr) < 0) {
		perror("fail to get eth interface MAC addr (ioctl)");
		close(eth->sockfd);
		return -1;
	}
	memcpy(eth->mac_address, ifr.ifr_ifru.ifru_hwaddr.sa_data, 6);

	// Get eth interface IP address
	strncpy(ifr.ifr_ifrn.ifrn_name, ETH_INTERFACE_NAME, IFNAMSIZ);
	ifr.ifr_ifru.ifru_addr.sa_family = AF_INET;

	if (ioctl(eth->sockfd, SIOCGIFADDR, &ifr) < 0) {
		perror("fail to get eth interface IP addr (ioctl)");
		close(eth->sockfd);
		return -1;
	}
	eth->ip_address = ((struct sockaddr_in*)&ifr.ifr_ifru.ifru_addr)->sin_addr.s_addr;


	char mac_buf[32];
	char ip_buf[32];
	util_mac_address_to_str(eth->mac_address, mac_buf);
	util_ip_address_to_str(eth->ip_address, ip_buf);
	printf("Successfully loaded interface [%s] with:\n", ETH_INTERFACE_NAME);
	printf("\t- MAC Address: [%s]\n", mac_buf);
	printf("\t- IP Address: [%s]\n", ip_buf);
	
	return 0;
}

int eth_send(Eth_Descriptor* eth, const uint8_t* packet_data, int32_t packet_size) {
	struct ether_header* ether_header = packet_ethernet_get_header(packet_data);
	const uint8_t* eth_dst_mac_addr = packet_ethernet_get_dst_mac_addr(ether_header);

	// Fill out the sockaddr_ll struct
	struct sockaddr_ll sock_addr;
	memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sll_family = AF_PACKET;
	sock_addr.sll_ifindex = eth->ifindex;
	sock_addr.sll_halen = ETH_ALEN;
	memcpy(sock_addr.sll_addr, eth_dst_mac_addr, ETH_ALEN);

	// Send the packet
	ssize_t bytes_written = sendto(eth->sockfd, packet_data, packet_size, 0, (struct sockaddr *)&sock_addr, sizeof(sock_addr));
	if (bytes_written < 0) {
		printf("SIZE IS %d\n", packet_size);
		perror("unable to direct spoofed packet to eth interface (sendto)");
		return -1;
	}

	assert(bytes_written == packet_size); // TODO: Is it possible in this particular case that 'write' will write less?

	return 0;
}

int32_t eth_receive(Eth_Descriptor* eth, uint8_t* buffer, uint32_t buffer_size) {
	// Fill out the sockaddr_ll struct
	struct sockaddr_ll sock_addr;
	memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sll_family = AF_PACKET;
	sock_addr.sll_ifindex = eth->ifindex;
	sock_addr.sll_halen = ETH_ALEN;
	
	// todo: TEST
	//char new_spoofed_src_mac[6] = { 0x54, 0xb2, 0x03, 0x04, 0x77, 0xe3 };
	//memcpy(sock_addr.sll_addr, new_spoofed_src_mac, ETH_ALEN); // need to bind?

	socklen_t sock_addr_len = sizeof(sock_addr);

	ssize_t bytes_received = recvfrom(eth->sockfd, buffer, buffer_size, 0, (struct sockaddr *)&sock_addr, &sock_addr_len);
	if (bytes_received < 0) {
		perror("unable to receive response from eth interface (recvfrom)");
		return -1;
	}

	return bytes_received;
}

void eth_release(Eth_Descriptor* eth) {
	close(eth->sockfd);
}

uint32_t eth_get_ip_address(Eth_Descriptor* eth) {
	return eth->ip_address;
}

void eth_get_mac_address(Eth_Descriptor* eth, uint8_t mac_address[6]) {
	memcpy(mac_address, eth->mac_address, 6);
}