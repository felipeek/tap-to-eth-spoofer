// Create long-lived tap interface
// sudo ip tuntap add tap0 mode tap
// sudo ip link set tap0 up

#define _BSD_SOURCE
#include <stdio.h>
#include <pthread.h>
#include "eth_receive.h"
#include "tap_receive.h"

int main(int argc, char *argv[]) {
	Eth_Descriptor eth;
	Tap_Descriptor tap;
	Eth_Spoofing_Descriptor esd;
	Tap_Spoofing_Descriptor tsd;
	atomic_int thread_stop = 0;

	if (tap_init(&tap)) {
		fprintf(stderr, "fail to init tap\n");
		return -1;
	}

	if (eth_init(&eth)) {
		fprintf(stderr, "fail to init eth\n");
		tap_release(&tap);
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