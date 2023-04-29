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

#include "packet.h"
#include "tap.h"
#include "eth.h"
#include "in.h"
#include "out.h"

#define IO_BUFFER_SIZE 2048

static char TAP_IN_BUFFER[IO_BUFFER_SIZE];

int main(int argc, char *argv[]) {
	Eth_Descriptor eth;
	Tap_Descriptor tap;
	struct ifreq ifr;

	if (tap_init(&tap)) {
		fprintf(stderr, "fail to init tap\n");
		return -1;
	}

	if (eth_init(&eth)) {
		fprintf(stderr, "fail to init eth\n");
		tap_release(&tap);
		return -1;
	}

	// Read packets from TAP device
	while (1) {
		int32_t tap_in_bytes_read;
		if (tap_receive(&tap, TAP_IN_BUFFER, IO_BUFFER_SIZE, &tap_in_bytes_read)) {
			fprintf(stderr, "fail to read packets from tap interface\n");
			tap_release(&tap);
			eth_release(&eth);
			return -1;
		}

		packet_print(TAP_IN_BUFFER);

		int32_t spoofed_in_packet_len;
		uint8_t* spoofed_in_packet_data = in_spoof_packet(TAP_IN_BUFFER, tap_in_bytes_read, &spoofed_in_packet_len);

		if (spoofed_in_packet_data == NULL) {
			// packet was ignored.
			continue;
		}

		if (eth_send(&eth, spoofed_in_packet_data, spoofed_in_packet_len) < 0) {
			fprintf(stderr, "unable to direct spoofed packet to eth interface");
			tap_release(&tap);
			eth_release(&eth);
			return -1;
		}

		int32_t spoofed_out_packet_len;
		uint8_t* spoofed_out_packet_data;
		for (;;) {
			// Question: can we lose packets with this approach?
			// i.e. start reading only after we already dispatched?
			// Or are packets kept in a socket queue?
			// If yes what is the queue capacity?
			uint8_t receive_buffer[2048];
			int32_t received_packet_size;
			if (eth_receive(&eth, receive_buffer, 2048, &received_packet_size)) {
				fprintf(stderr, "unable to receive packet from eth interface\n");
				tap_release(&tap);
				eth_release(&eth);
				return -1;
			}

			spoofed_out_packet_data = out_spoof_packet(spoofed_in_packet_data, spoofed_in_packet_len,
				receive_buffer, received_packet_size, &spoofed_out_packet_len);
			
			if (spoofed_out_packet_data != NULL) {
				// response was captured
				break;
			}
		}

		printf("Printing spoofed response package.\n");
		packet_print(spoofed_out_packet_data);

		// Write the packet to the tap interface
		if (tap_send(&tap, spoofed_out_packet_data, spoofed_in_packet_len)) {
			fprintf(stderr, "unable to write spoofed packet to tap interface\n");
			tap_release(&tap);
			eth_release(&eth);
			return -1;
		}
	}

	tap_release(&tap);
	eth_release(&eth);
	return 0;
}


// force all traffic to pudim.com.br to go through tap0
// sudo ip route add 54.207.20.104 dev tap0