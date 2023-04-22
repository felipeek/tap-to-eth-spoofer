#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <unistd.h>
#include <fcntl.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include "packet.h"

#define TAP_DEVICE_NAME "tap0"

static void ip_to_str(uint32_t ip, char buf[32]) {
	unsigned char bytes[4];
	bytes[0] = ip & 0xFF;
	bytes[1] = (ip >> 8) & 0xFF;
	bytes[2] = (ip >> 16) & 0xFF;
	bytes[3] = (ip >> 24) & 0xFF;
	sprintf(buf, "%d.%d.%d.%d", bytes[3], bytes[2], bytes[1], bytes[0]);
}

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

		packet_ethernet_print(buf);

		//struct iphdr* ip = (struct iphdr*)(h + 1);
		//char saddr[32];
		//char daddr[32];
		//ip_to_str(ip->saddr, saddr);
		//ip_to_str(ip->daddr, daddr);
		//printf("- saddr: %s\n", saddr);
		//printf("- daddr: %s\n", daddr);
	}

	close(tap_fd);
	return 0;
}
