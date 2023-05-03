#include <stdio.h>
#include <stdlib.h>
#include "tap_receive.h"
#include "packet.h"

static char TAP_IN_BUFFER[IO_BUFFER_SIZE];	// client -> tap interface
static char ETH_OUT_BUFFER[IO_BUFFER_SIZE]; // tap interface -> *spoofing* -> eth interface

void* tap_receive_thread_proc(void* args) {
	Tap_Receive_Thread_Args* rtargs = (Tap_Receive_Thread_Args*)args;
	Eth_Descriptor* eth = rtargs->eth;
	Tap_Descriptor* tap = rtargs->tap;
	Eth_Spoofing_Descriptor* esd = rtargs->esd;
	atomic_int* stop = rtargs->stop;

	// Read packets from TAP device
	for (;;) {
		int32_t tap_in_bytes_read = tap_receive(tap, TAP_IN_BUFFER, IO_BUFFER_SIZE);
		
		if (*stop) {
			printf("stopping eth receive thread...\n");
			break;
		}
		
		if (tap_in_bytes_read < 0) {
			fprintf(stderr, "fail to read packets from tap interface\n");
			return NULL;
		}

		printf("Printing packet received via tap interface.\n");
		packet_print(TAP_IN_BUFFER);

		int32_t spoofed_in_packet_size = eth_spoof_packet(esd, TAP_IN_BUFFER, tap_in_bytes_read, ETH_OUT_BUFFER);

		if (spoofed_in_packet_size < 0) {
			fprintf(stderr, "fail to spoof in packet\n");
			return NULL;
		}

		if (eth_send(eth, ETH_OUT_BUFFER, spoofed_in_packet_size) < 0) {
			fprintf(stderr, "unable to direct spoofed packet to eth interface\n");
			return NULL;
		}
	}

	*stop = 1;
	return NULL;
}