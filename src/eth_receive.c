#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eth_receive.h"
#include "packet.h"

static char ETH_IN_BUFFER[IO_BUFFER_SIZE];  // eth interface -> remote -> eth interface
static char TAP_OUT_BUFFER[IO_BUFFER_SIZE]; // eth interface -> *reverse-spoofing* -> client

typedef struct Packet_Container {
	uint8_t packet_data[2048];
	uint32_t packet_length;
	struct Packet_Container* next;
} Packet_Container;

typedef struct {
	Packet_Container* first;
	Packet_Container* last;
} Packet_Container_List;

void* eth_receive_thread_proc(void* args) {
	Eth_Receive_Thread_Args* rtargs = (Eth_Receive_Thread_Args*)args;
	Eth_Descriptor* eth = rtargs->eth;
	Tap_Descriptor* tap = rtargs->tap;
	Out_Spoofing_Descriptor* osd = rtargs->osd;
	Packet_Container_List pcl;
	memset(&pcl, 0, sizeof(Packet_Container_List));

	uint8_t tap_mac_address[6];
	tap_get_mac_address(tap, tap_mac_address);

	for (;;) {
		int32_t received_packet_size = eth_receive(eth, ETH_IN_BUFFER, IO_BUFFER_SIZE);
		if (received_packet_size < 0) {
			fprintf(stderr, "unable to receive packet from eth interface\n");
			return NULL;
		}

		if (pcl.first != NULL) {
			if (pcl.first->packet_length == received_packet_size) {
				if (!memcmp(pcl.first->packet_data, ETH_IN_BUFFER, pcl.first->packet_length)) {
					free(pcl.first);
					pcl.first = pcl.first->next;
					if (pcl.first == NULL) {
						pcl.last = NULL;
					}

					continue;
				}
			}
		}

		struct ether_header* received_packet_ether_header = packet_ethernet_get_header(ETH_IN_BUFFER);
		const uint8_t* mac_addr = packet_ethernet_get_dst_mac_addr(received_packet_ether_header);
		
		if (memcmp(tap_mac_address, mac_addr, 6)) {
			// Destination MAC Address is not equal to TAP interface's destination MAC address.
			// Packet can be ignored.
			continue;
		}

		int32_t spoofed_out_packet_len = out_spoof_packet(osd, 
			ETH_IN_BUFFER, received_packet_size, TAP_OUT_BUFFER);

		if (spoofed_out_packet_len < 0) {
			fprintf(stderr, "fail to spoof out packet\n");
			return NULL;
		}

		if (spoofed_out_packet_len > 0) {
			printf("Printing spoofed response package.\n");
			packet_print(TAP_OUT_BUFFER);

			//uint8_t new_mac[6];
			//memset(new_mac, 0, 6);
			//new_mac[2] = 5;
			//packet_ethernet_set_src_mac_addr(TAP_OUT_BUFFER, new_mac);
			// Write the packet to the tap interface
			if (tap_send(tap, TAP_OUT_BUFFER, spoofed_out_packet_len)) {
				fprintf(stderr, "unable to write spoofed packet to tap interface\n");
				return NULL;
			}

			Packet_Container* pc = malloc(sizeof(Packet_Container));
			if (pc == NULL) {
				perror("fail to alloc packet container (malloc)");
				return NULL;
			}

			memcpy(pc->packet_data, TAP_OUT_BUFFER, spoofed_out_packet_len);
			pc->packet_length = spoofed_out_packet_len;
			pc->next = NULL;
			if (pcl.last == NULL) {
				pcl.first = pc;
				pcl.last = pc;
			} else {
				pcl.last->next = pc;
			}
		}
	}


	return NULL;
}