// Create long-lived tap interface
// sudo ip tuntap add tap0 mode tap
// sudo ip link set tap0 up

#define _BSD_SOURCE
#include <stdio.h>
#include <pthread.h>
#include "eth_receive.h"
#include "tap_receive.h"
#include "util.h"

#define TAP_INTERFACE_NAME "tap0"
// for now, let's make the tap interface be in the same network as eth
// for that, it has to have a different IP within the same subnet, and also same default gateway
#define TAP_INTERFACE_IP "192.168.0.15"
#define TAP_INTERFACE_NETMASK "255.255.255.0"
#define TAP_DEFAULT_GATEWAY_IP "192.168.0.1"
static char TAP_MAC_ADDR[6] = { 0x82, 0xa2, 0x17, 0x43, 0x15, 0xff };

int main(int argc, char *argv[]) {
	Eth_Descriptor eth;
	Tap_Descriptor tap;
	Eth_Spoofing_Descriptor esd;
	Tap_Spoofing_Descriptor tsd;
	atomic_int thread_stop = 0;

	if (argc != 5) {
		fprintf(stderr, "usage: %s <tap_interface_name> <eth_interface_name> <tap_interface_ip> <tap_interface_mac>\n", argv[0]);
		fprintf(stderr, "example: %s tap0 eth0 192.168.0.22 82:a2:17:43:15:ff\n", argv[0]);
		fprintf(stderr, "\t - <tap_interface_name>: an arbitrary name for the tap interface, will be automatically created, e.g. tap0\n");
		fprintf(stderr, "\t - <eth_interface_name>: the name of the eth interface, which must already exist, e.g. eth0\n");
		fprintf(stderr, "\t - <tap_interface_ip>: the ip of the tap interface, it is arbitrary, however, it must be within the same subnet of the eth interface, e.g. 192.168.0.22\n");
		fprintf(stderr, "\t - <tap_interface_mac>: the mac of the tap interface, it is arbitrary, e.g. 82:a2:17:43:15:ff\n");
		return -1;
	}

	uint8_t* tap_interface_name = argv[1];
	uint8_t* eth_interface_name = argv[2];
	uint32_t tap_interface_ip = util_ip_address_str_to_uint32(argv[3]);
	uint8_t tap_interface_mac[6];
	util_mac_address_str_to_buf(argv[4], tap_interface_mac);

	if (eth_init(&eth, eth_interface_name)) {
		fprintf(stderr, "fail to init eth\n");
		return -1;
	}

	uint32_t eth_ip = eth_get_ip_address(&eth);
	uint32_t eth_netmask = eth_get_netmask_address(&eth);
	uint32_t eth_default_gateway = eth_get_default_gateway(&eth);

	// check if provided tap interface IP is within eth subnet
	// if it is not, we fail to start, this is because we are by default simply dispatching tap packets to eth
	// so in this case these packets would have a source IP address that is not part of that subnet, which could
	// introduce routing problems.
	// we could circumvent this by always mapping TAP interface's IP address to another IP within eth's subnet when dispatching
	// and therefore allowing TAP interface's subnet to be arbitrary... but this is not being done atm
	// (also, would need to probably fix other stuff, e.g. we would need to also map the default gateway, and review the
	// sent packets list logic in eth_receive, as we would probably not receive duplicated packets in eth anymore)
	if (!util_is_ip_within_subnet(tap_interface_ip, eth_netmask, eth_ip)) {
		fprintf(stderr, "fail: tap interface IP not within eth subnet\n");
		eth_release(&eth);
		return -1;
	}

	if (tap_init(&tap, tap_interface_name, tap_interface_ip, eth_netmask, eth_default_gateway, tap_interface_mac)) {
		fprintf(stderr, "fail to init tap\n");
		eth_release(&eth);
		return -1;
	}

	eth_spoof_init(&esd);
	tap_spoof_init(&tsd);

	pthread_t eth_receive_thread;
	Eth_Receive_Thread_Args eth_rt_args;
	eth_rt_args.eth = &eth;
	eth_rt_args.tap = &tap;
	eth_rt_args.tsd = &tsd;
	eth_rt_args.stop = &thread_stop;
	if (pthread_create(&eth_receive_thread, NULL, eth_receive_thread_proc, &eth_rt_args) < 0) {
		perror("unable to create eth receive thread (pthread_create)");
		tap_release(&tap);
		eth_release(&eth);
		return -1;
	}

	pthread_t tap_receive_thread;
	Tap_Receive_Thread_Args tap_rt_args;
	tap_rt_args.eth = &eth;
	tap_rt_args.tap = &tap;
	tap_rt_args.esd = &esd;
	tap_rt_args.stop = &thread_stop;
	if (pthread_create(&tap_receive_thread, NULL, tap_receive_thread_proc, &tap_rt_args) < 0) {
		perror("unable to create tap receive thread (pthread_create)");
		tap_release(&tap);
		eth_release(&eth);
		return -1;
	}

	if (pthread_join(eth_receive_thread, NULL) < 0) {
		perror("unable to join eth receive thread (pthread_join)");
		tap_release(&tap);
		eth_release(&eth);
		return -1;
	}
	
	if (pthread_join(tap_receive_thread, NULL) < 0) {
		perror("unable to join tap receive thread (pthread_join)");
		tap_release(&tap);
		eth_release(&eth);
		return -1;
	}

	tap_release(&tap);
	eth_release(&eth);
	return 0;
}