#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eth_receive.h"
#include "packet.h"

#define SENT_LIST_MAX_NUM_PACKETS 1024

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
	int32_t num_packets;
} Packet_Container_List;

static int pop_packet_from_sent_list_if_there(Packet_Container_List* pcl, uint8_t* packet, uint32_t packet_size) {
	if (pcl->first != NULL) {
		if (pcl->first->packet_length == packet_size) {
			if (!memcmp(pcl->first->packet_data, packet, pcl->first->packet_length)) {
				Packet_Container* pc_next = pcl->first->next;
				free(pcl->first);
				pcl->first = pc_next;
				if (pcl->first == NULL) {
					pcl->last = NULL;
				}
				--pcl->num_packets;

				return 1;
			}
		}
	}

	return 0;
}

static int add_packet_to_sent_list(Packet_Container_List* pcl, uint8_t* packet, uint32_t packet_size) {
	if (pcl->num_packets > SENT_LIST_MAX_NUM_PACKETS) {
		fprintf(stderr, "unable to add packet to sent list, list is full.\n");
		return -1;
	}

	Packet_Container* pc = malloc(sizeof(Packet_Container));
	if (pc == NULL) {
		perror("fail to alloc packet container (malloc)");
		return -1;
	}

	memcpy(pc->packet_data, packet, packet_size);
	pc->packet_length = packet_size;
	pc->next = NULL;
	if (pcl->last == NULL) {
		pcl->first = pc;
		pcl->last = pc;
	} else {
		pcl->last->next = pc;
	}
	++pcl->num_packets;

	return 0;
}

static void release_packet_list(Packet_Container_List* pcl) {
	Packet_Container* pc = pcl->first;
	
	while (pc) {
		Packet_Container* pc_next = pc->next;
		free(pc);
		pc = pc_next;
	}

	pcl->first = NULL;
	pcl->last = NULL;
}

void* eth_receive_thread_proc(void* args) {
	Eth_Receive_Thread_Args* rtargs = (Eth_Receive_Thread_Args*)args;
	Eth_Descriptor* eth = rtargs->eth;
	Tap_Descriptor* tap = rtargs->tap;
	Tap_Spoofing_Descriptor* tsd = rtargs->tsd;
	atomic_int* stop = rtargs->stop;

	// We create a list to store the packets that we send to the tap interface
	// The only reason we keep this list is because all packets that we send to the tap interface will also
	// be received when we receive packets from the eth interface, if the socket is opened in promiscuous mode
	// ...probably because since both tap interface and eth interface are sharing the same subnet
	// Therefore, we need to somehow ignore these packets when we receive them, and for that we compare with our list.
	Packet_Container_List pcl;
	memset(&pcl, 0, sizeof(Packet_Container_List));

	uint8_t tap_mac_address[6];
	tap_get_mac_address(tap, tap_mac_address);

	for (;;) {
		int32_t received_packet_size = eth_receive(eth, ETH_IN_BUFFER, IO_BUFFER_SIZE);

		if (*stop) {
			printf("stopping eth receive thread...\n");
			break;
		}

		if (received_packet_size < 0) {
			fprintf(stderr, "unable to receive packet from eth interface\n");
			break;
		}

		if (pop_packet_from_sent_list_if_there(&pcl, ETH_IN_BUFFER, received_packet_size)) {
			// We received a packet that we previously sent, we must ignore, otherwise we will loop indefinitely.
			continue;
		}

		struct ether_header* received_packet_ether_header = packet_ethernet_get_header(ETH_IN_BUFFER);
		const uint8_t* mac_addr = packet_ethernet_get_dst_mac_addr(received_packet_ether_header);
		
		if (memcmp(tap_mac_address, mac_addr, 6)) {
			// Destination MAC Address is not equal to TAP interface's destination MAC address.
			// Packet can be ignored.
			continue;
		}

		int32_t spoofed_out_packet_len = tap_spoof_packet(tsd, 
			ETH_IN_BUFFER, received_packet_size, TAP_OUT_BUFFER);

		if (spoofed_out_packet_len < 0) {
			fprintf(stderr, "fail to spoof out packet\n");
			break;
		}

		if (spoofed_out_packet_len > 0) {
			printf("Printing spoofed response package.\n");
			packet_print(TAP_OUT_BUFFER);

			// Write the packet to the tap interface
			if (tap_send(tap, TAP_OUT_BUFFER, spoofed_out_packet_len)) {
				fprintf(stderr, "unable to write spoofed packet to tap interface\n");
				break;
			}

			if (add_packet_to_sent_list(&pcl, TAP_OUT_BUFFER, spoofed_out_packet_len) < 0) {
				fprintf(stderr, "unable to add packet to sent list");
				break;
			}
		}
	}

	release_packet_list(&pcl);
	*stop = 1;
	return NULL;
}