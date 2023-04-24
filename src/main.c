#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
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

#include "packet.h"

#define TAP_DEVICE_NAME "tap0"
#define ETH_DEVICE_NAME "eno1"

int main(int argc, char *argv[]) {
	int tap_fd;
	struct ifreq ifr;
	char buf[2048];

	// Open TAP device
	if ((tap_fd = open("/dev/net/tun", O_RDWR)) < 0) {
		perror("open");
		exit(1);
	}

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
	strncpy(ifr.ifr_name, TAP_DEVICE_NAME, IFNAMSIZ);

	// Create TAP device
	if (ioctl(tap_fd, TUNSETIFF, (void *)&ifr) < 0) {
		perror("ioctl");
		close(tap_fd);
		exit(1);
	}

	// Open a socket for sending packets
	int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sockfd == -1) {
		perror("socket");
		close(tap_fd);
		exit(EXIT_FAILURE);
	}

	// Specify the interface name and index
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ETH_DEVICE_NAME, IFNAMSIZ - 1);
	if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1) {
		perror("ioctl");
		close(tap_fd);
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	//int ifindex = ifr.ifr_ifindex;

	int ifindex = if_nametoindex(ETH_DEVICE_NAME);

	// Read packets from TAP device
	while (1) {
		int nbytes = read(tap_fd, buf, sizeof(buf));
		if (nbytes < 0) {
			perror("read");
			close(tap_fd);
			exit(1);
		}

		// Process received packet here
		printf("I received %d bytes.\n", nbytes);

		packet_print(buf);
		struct ether_header* ether_header = packet_ethernet_get_header(buf);
		const uint8_t* src_mac = packet_ethernet_get_src_mac_addr(ether_header);
		const uint8_t* dst_mac = packet_ethernet_get_dst_mac_addr(ether_header);

		// Fill out the sockaddr_ll struct
		struct sockaddr_ll sock_addr;
		memset(&sock_addr, 0, sizeof(sock_addr));
		sock_addr.sll_family = AF_PACKET;
		sock_addr.sll_ifindex = ifindex;
		sock_addr.sll_halen = ETH_ALEN;
		memcpy(sock_addr.sll_addr, dst_mac, ETH_ALEN);

		uint16_t ether_type = packet_ethernet_get_type(ether_header);

		uint32_t arp_unknown_ip;
		if (ether_type == ETHERTYPE_ARP) {
			printf("Spoofing ARP packet's MAC and IP...\n");

			struct arphdr* arp_header = packet_arp_get_header(buf);

			arp_unknown_ip = packet_arp_get_target_protocol_address(arp_header);

			char new_spoofed_src_mac[6] = { 0x54, 0xb2, 0x03, 0x04, 0x77, 0xe3 };
			packet_arp_set_sender_hw_address(arp_header, new_spoofed_src_mac);
			packet_ethernet_set_src_mac_addr(ether_header, new_spoofed_src_mac);

			unsigned char new_spoofed_src_ip_arr[4] = { 192, 168, 0, 2 };
			uint32_t new_spoofed_src_ip = new_spoofed_src_ip_arr[0] | (new_spoofed_src_ip_arr[1] << 8) |
				(new_spoofed_src_ip_arr[2] << 16) | (new_spoofed_src_ip_arr[3] << 24);

			packet_arp_set_sender_protocol_address(arp_header, new_spoofed_src_ip);

			uint8_t ip_buf[32];
			packet_ip_address_to_str(new_spoofed_src_ip, ip_buf);
			printf("new spoofed IP: %s\n", ip_buf);
		} else {
			printf("dropping packet...\n");
			continue;
		}

		// Construct the Ethernet frame
		//struct ethhdr *eth = (struct ethhdr *)buf;
		//memcpy(eth->h_source, src_mac, ETH_ALEN);
		//memcpy(eth->h_dest, dst_mac, ETH_ALEN);
		//eth->h_proto = 0;
		//int frame_len = sizeof(struct ethhdr);
		//strcpy(buf + sizeof(struct ethhdr), "Hello World");
		//frame_len += strlen("Hello World");

		// Send the packet
		//ssize_t bytes_sent = sendto(sockfd, buf, frame_len, 0, (struct sockaddr *)&sock_addr, sizeof(sock_addr));
		ssize_t bytes_sent = sendto(sockfd, buf, nbytes, 0, (struct sockaddr *)&sock_addr, sizeof(sock_addr));
		if (bytes_sent == -1) {
			perror("sendto");
			exit(EXIT_FAILURE);
		}
		printf("%zd bytes sent\n", bytes_sent);

		char response[1024];
		//struct sockaddr_in recv_addr;
		//socklen_t recv_addr_len = sizeof(recv_addr);
		socklen_t sock_addr_len = sizeof(sock_addr);
		ssize_t bytes_received;

		for (;;) {
			//bytes_received = recvfrom(sockfd, response, sizeof(response), 0, (struct sockaddr *)&recv_addr, &recv_addr_len);
			bytes_received = recvfrom(sockfd, response, sizeof(response), 0, (struct sockaddr *)&sock_addr, &sock_addr_len);
			if (bytes_received == -1) {
				perror("recvfrom");
				exit(EXIT_FAILURE);
			}
			
			ether_header = packet_ethernet_get_header(response);
			ether_type = packet_ethernet_get_type(ether_header);

			if (ether_type == ETHERTYPE_ARP) {
				printf("Spoofing ARP packet's *RESPONSE* MAC and IP...\n");

				struct arphdr* arp_header = packet_arp_get_header(response);

				if (packet_arp_get_sender_protocol_address(arp_header) == arp_unknown_ip) {
					printf("Found response!\n");
				} else {
					uint8_t ip_bufA[32];
					uint8_t ip_bufB[32];
					packet_ip_address_to_str(packet_arp_get_sender_protocol_address(arp_header), ip_bufA);
					packet_ip_address_to_str(arp_unknown_ip, ip_bufB);
					printf("not response, skipping (want %s but got %s)\n", ip_bufB, ip_bufA);
					//packet_print(response);
					
					continue;
				}

				char new_spoofed_dst_mac[6] = { 0x82, 0xa2, 0x17, 0x43, 0x15, 0xef };
				packet_arp_set_target_hw_address(arp_header, new_spoofed_dst_mac);
				packet_ethernet_set_dst_mac_addr(ether_header, new_spoofed_dst_mac);

				unsigned char new_spoofed_dst_ip_arr[4] = { 192, 168, 2, 1 };
				uint32_t new_spoofed_dst_ip = new_spoofed_dst_ip_arr[0] | (new_spoofed_dst_ip_arr[1] << 8) |
					(new_spoofed_dst_ip_arr[2] << 16) | (new_spoofed_dst_ip_arr[3] << 24);
				packet_arp_set_target_protocol_address(arp_header, new_spoofed_dst_ip);

				uint8_t ip_buf[32];
				packet_ip_address_to_str(new_spoofed_dst_ip, ip_buf);
				printf("new spoofed IP: %s\n", ip_buf);
				break;
			}
		}

		printf("Printing spoofed response package.\n");
		packet_print(response);

		// Write the packet to the tap interface
		nbytes = write(tap_fd, response, bytes_received);
		if (nbytes < 0) {
			perror("write");
			close(tap_fd);
			exit(1);
		}
	}

	close(tap_fd);
	return 0;
}


// force all traffic to pudim.com.br to go through tap0
// sudo ip route add 54.207.20.104 dev tap0