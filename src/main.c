// Create long-lived tap interface
// sudo ip tuntap add tap0 mode tap
// sudo ip link set tap0 up

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

static char TAP_IN_BUFFER[IO_BUFFER_SIZE];	// client -> tap interface
static char ETH_OUT_BUFFER[IO_BUFFER_SIZE]; // tap interface -> *spoofing* -> eth interface
static char ETH_IN_BUFFER[IO_BUFFER_SIZE];  // eth interface -> remote -> eth interface
static char TAP_OUT_BUFFER[IO_BUFFER_SIZE]; // eth interface -> *reverse-spoofing* -> client

int main(int argc, char *argv[]) {
	Eth_Descriptor eth;
	Tap_Descriptor tap;
	In_Spoofing_Descriptor isd;
	Out_Spoofing_Descriptor osd;
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

	uint32_t in_spoof_ip_address = eth_get_ip_address(&eth);
	uint8_t in_spoof_mac_address[6];
	eth_get_mac_address(&eth, in_spoof_mac_address);
	in_spoof_init(&isd, in_spoof_ip_address, in_spoof_mac_address);

	int32_t out_spoof_ip_address = tap_get_ip_address(&tap);
	int8_t out_spoof_mac_address[6];
	tap_get_mac_address(&tap, out_spoof_mac_address);
	out_spoof_init(&osd, out_spoof_ip_address, out_spoof_mac_address);

	// Read packets from TAP device
	while (1) {
		int32_t tap_in_bytes_read = tap_receive(&tap, TAP_IN_BUFFER, IO_BUFFER_SIZE);
		if (tap_in_bytes_read < 0) {
			fprintf(stderr, "fail to read packets from tap interface\n");
			tap_release(&tap);
			eth_release(&eth);
			return -1;
		}

		packet_print(TAP_IN_BUFFER);

		int32_t spoofed_in_packet_size = in_spoof_packet(&isd, TAP_IN_BUFFER, tap_in_bytes_read, ETH_OUT_BUFFER);

		if (spoofed_in_packet_size < 0) {
			// packet was ignored.
			continue;
		}

		if (eth_send(&eth, ETH_OUT_BUFFER, spoofed_in_packet_size) < 0) {
			fprintf(stderr, "unable to direct spoofed packet to eth interface");
			tap_release(&tap);
			eth_release(&eth);
			return -1;
		}

		int32_t spoofed_out_packet_len;
		for (;;) {
			// Question: can we lose packets with this approach?
			// i.e. start reading only after we already dispatched?
			// Or are packets kept in a socket queue?
			// If yes what is the queue capacity?
			int32_t received_packet_size = eth_receive(&eth, ETH_IN_BUFFER, IO_BUFFER_SIZE);
			if (received_packet_size < 0) {
				fprintf(stderr, "unable to receive packet from eth interface\n");
				tap_release(&tap);
				eth_release(&eth);
				return -1;
			}

			spoofed_out_packet_len = out_spoof_packet(&osd, ETH_OUT_BUFFER, spoofed_in_packet_size,
				ETH_IN_BUFFER, received_packet_size, TAP_OUT_BUFFER);
			
			if (spoofed_out_packet_len >= 0) {
				// response was captured
				break;
			}
		}

		printf("Printing spoofed response package.\n");
		packet_print(TAP_OUT_BUFFER);

		// Write the packet to the tap interface
		if (tap_send(&tap, TAP_OUT_BUFFER, spoofed_out_packet_len)) {
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