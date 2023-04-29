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
#include "tap.h"
#include "in.h"
#include "out.h"

#define ETH_INTERFACE_NAME "eno1"

int main(int argc, char *argv[]) {
	struct ifreq ifr;
	char buf[2048];

	int tap_fd = tap_init();

	// Open a socket to send packets to interact with eth interface
	int eth_sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (eth_sockfd == -1) {
		perror("fail to create socket for eth interface (socket)");
		close(tap_fd);
		return -1;
	}

	int eth_ifindex = if_nametoindex(ETH_INTERFACE_NAME);

	// Read packets from TAP device
	while (1) {
		int nbytes = read(tap_fd, buf, sizeof(buf));
		if (nbytes < 0) {
			perror("fail to read packets from tap interface (read)");
			close(tap_fd);
			close(eth_sockfd);
			return -1;
		}

		// Process received packet here
		printf("I received %d bytes.\n", nbytes);

		packet_print(buf);

		int32_t spoofed_in_packet_len;
		uint8_t* spoofed_in_packet_data = in_spoof_packet(buf, nbytes, &spoofed_in_packet_len);

		if (spoofed_in_packet_data == NULL) {
			// packet was ignored.
			continue;
		}

		struct ether_header* spoofed_in_packet_ether_header = packet_ethernet_get_header(spoofed_in_packet_data);
		const uint8_t* spoofed_in_packet_dst_mac_addr = packet_ethernet_get_dst_mac_addr(spoofed_in_packet_ether_header);

		// Fill out the sockaddr_ll struct
		struct sockaddr_ll sock_addr;
		memset(&sock_addr, 0, sizeof(sock_addr));
		sock_addr.sll_family = AF_PACKET;
		sock_addr.sll_ifindex = eth_ifindex;
		sock_addr.sll_halen = ETH_ALEN;
		memcpy(sock_addr.sll_addr, spoofed_in_packet_dst_mac_addr, ETH_ALEN);

		// Send the packet
		ssize_t bytes_sent = sendto(eth_sockfd, spoofed_in_packet_data, spoofed_in_packet_len, 0,
			(struct sockaddr *)&sock_addr, sizeof(sock_addr));
		if (bytes_sent < 0) {
			perror("unable to direct spoofed packet to eth interface (sendto)");
			close(tap_fd);
			close(eth_sockfd);
			return -1;
		}
		printf("%zd bytes sent\n", bytes_sent);

		char response[1024];
		socklen_t sock_addr_len = sizeof(sock_addr);
		ssize_t bytes_received;

		int32_t spoofed_out_packet_len;
		uint8_t* spoofed_out_packet_data;
		for (;;) {
			bytes_received = recvfrom(eth_sockfd, response, sizeof(response), 0, (struct sockaddr *)&sock_addr, &sock_addr_len);
			if (bytes_received < 0) {
				perror("unable to receive response from eth interface (recvfrom)");
				close(tap_fd);
				close(eth_sockfd);
				return -1;
			}
			
			spoofed_out_packet_data = out_spoof_packet(spoofed_in_packet_data, spoofed_in_packet_len,
				response, bytes_received, &spoofed_out_packet_len);
			
			if (spoofed_out_packet_data != NULL) {
				// response was captured
				break;
			}
		}

		printf("Printing spoofed response package.\n");
		packet_print(spoofed_out_packet_data);

		// Write the packet to the tap interface
		nbytes = write(tap_fd, spoofed_out_packet_data, spoofed_out_packet_len);
		if (nbytes < 0) {
			perror("unable to write spoofed packet to tap interface (write)");
			close(tap_fd);
			close(eth_sockfd);
			return -1;
		}
	}

	close(tap_fd);
	close(eth_sockfd);
	return 0;
}


// force all traffic to pudim.com.br to go through tap0
// sudo ip route add 54.207.20.104 dev tap0