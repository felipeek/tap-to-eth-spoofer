#define _BSD_SOURCE
#include <fcntl.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <linux/if_arp.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "tap.h"

int tap_init() {
	int fd;
	struct ifreq ifr;
	struct sockaddr_in addr;

	// Open the TAP device
	if ((fd = open(TAP_DEVICE_FILE, O_RDWR)) < 0) {
		perror("fail to open tap device (open)");
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
	strncpy(ifr.ifr_name, TAP_INTERFACE_NAME, IFNAMSIZ);

	// Create the TAP interface via the TAP device
	if (ioctl(fd, TUNSETIFF, (void *)&ifr) < 0) {
		perror("fail to create tap interface (ioctl)");
		close(fd);
		return -1;
	}

	// Open a socket for tap interface
	// This will be used to configure the interface
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		perror("fail to create socket for tap interface (socket)");
		close(fd);
		return -1;
	}

	// Get tap interface flags, so we can add others on top of them
	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) {
		perror("fail to get tap interface flags (ioctl)");
		close(fd);
		close(sockfd);
		return -1;
	}

	// Add UP and RUNNING flags to tap interface
	ifr.ifr_flags |= IFF_UP | IFF_RUNNING;

	// Set the new flags to tap interface
	if (ioctl(sockfd, SIOCSIFFLAGS, &ifr)) {
		perror("fail to set tap interface flags (ioctl)");
		close(fd);
		close(sockfd);
		return -1;
	}

	// Prepare to set tap interface IP
	memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(TAP_INTERFACE_IP);
    memcpy(&ifr.ifr_addr, &addr, sizeof(addr));

	// Set tap interface IP
    if (ioctl(sockfd, SIOCSIFADDR, &ifr) < 0) {
        perror("fail to set tap interface IP (ioctl)");
        close(fd);
		close(sockfd);
		return -1;
    }

	ifr.ifr_ifru.ifru_hwaddr.sa_family = 1; // eq to ARPHRD_ETHER , not sure where is this is documented...
	memcpy(ifr.ifr_ifru.ifru_hwaddr.sa_data, TAP_MAC_ADDR, sizeof(TAP_MAC_ADDR));

	// Set tap interface MAC address
    if (ioctl(sockfd, SIOCSIFHWADDR, &ifr) < 0) {
        perror("fail to set tap interface MAC addr (ioctl)");
        close(fd);
		close(sockfd);
		return -1;
    }

	close(sockfd);
	return fd;
}