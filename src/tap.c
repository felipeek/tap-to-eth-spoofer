#define _BSD_SOURCE
#include <fcntl.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <assert.h>
#include <net/route.h>
#include <errno.h>

#include "tap.h"
#include "util.h"

#define TAP_DEVICE_FILE "/dev/net/tun"

int tap_init(Tap_Descriptor* tap, uint8_t* interface_name, uint32_t ip_address, uint32_t netmask,
		uint32_t default_gateway_ip_address, uint8_t mac_address[6]) {
	struct ifreq ifr;
	struct sockaddr_in addr;

	// Open the TAP device
	if ((tap->fd = open(TAP_DEVICE_FILE, O_RDWR)) < 0) {
		perror("fail to open tap device (open)");
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
	strncpy(ifr.ifr_name, interface_name, IFNAMSIZ);

	// Create the TAP interface via the TAP device
	if (ioctl(tap->fd, TUNSETIFF, (void *)&ifr) < 0) {
		perror("fail to create tap interface (ioctl)");
		close(tap->fd);
		return -1;
	}

	// Open a socket for tap interface
	// This will be used to configure the interface
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		perror("fail to create socket for tap interface (socket)");
		close(tap->fd);
		return -1;
	}

	// Get tap interface flags, so we can add others on top of them
	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) {
		perror("fail to get tap interface flags (ioctl)");
		close(tap->fd);
		close(sockfd);
		return -1;
	}

	// Add UP and RUNNING flags to tap interface
	ifr.ifr_flags |= IFF_UP | IFF_RUNNING;

	// Set the new flags to tap interface
	if (ioctl(sockfd, SIOCSIFFLAGS, &ifr)) {
		perror("fail to set tap interface flags (ioctl)");
		close(tap->fd);
		close(sockfd);
		return -1;
	}

	// Prepare to set tap interface IP
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ip_address;
	memcpy(&ifr.ifr_addr, &addr, sizeof(addr));

	// Set tap interface IP
	if (ioctl(sockfd, SIOCSIFADDR, &ifr) < 0) {
		perror("fail to set tap interface IP (ioctl)");
		close(tap->fd);
		close(sockfd);
		return -1;
	}

	char buf[32];
	util_ip_address_to_str(netmask, buf);
	printf("NETMASK IS %s\n", buf);

	// Prepare to set tap interface netmask
	struct sockaddr_in sock_netmask;
	memset(&sock_netmask, 0, sizeof(struct sockaddr_in));
	sock_netmask.sin_family = AF_INET;
	sock_netmask.sin_addr.s_addr = netmask;

	// Set tap interface netmask
	memcpy(&ifr.ifr_netmask, &sock_netmask, sizeof(struct sockaddr_in));
	if (ioctl(sockfd, SIOCSIFNETMASK, &ifr) < 0) {
		perror("fail to set tap interface netmask (ioctl)");
		close(tap->fd);
		close(sockfd);
		return -1;
	}

	ifr.ifr_ifru.ifru_hwaddr.sa_family = 1; // eq to ARPHRD_ETHER , not sure where is this is documented...
	memcpy(ifr.ifr_ifru.ifru_hwaddr.sa_data, mac_address, 6);

	// Set tap interface MAC address
	if (ioctl(sockfd, SIOCSIFHWADDR, &ifr) < 0) {
		perror("fail to set tap interface MAC addr (ioctl)");
		close(tap->fd);
		close(sockfd);
		return -1;
	}

	// Prepare to set tap interface default gateway
	struct rtentry route;
	memset(&route, 0, sizeof(struct rtentry));

	// Set the destination address to 0.0.0.0 (default route)
	route.rt_dst.sa_family = AF_INET;
	route.rt_flags = RTF_GATEWAY;
	route.rt_dev = interface_name;
	
	// Set the gateway IP address
	struct sockaddr_in* gateway_addr = (struct sockaddr_in *)&route.rt_gateway;
	gateway_addr->sin_family = AF_INET;
	gateway_addr->sin_addr.s_addr = default_gateway_ip_address;

	// Remove the route entry from the routing table
    if (ioctl(sockfd, SIOCDELRT, &route) < 0) {
		if (errno == ESRCH) {
			printf("tap interface default gateway does not exist yet...\n");
		} else {
			perror("fail to delete tap interface default gateway (ioctl)");
			close(tap->fd);
			close(sockfd);
			return -1;
		}
    }

	// Set the metric to be very high, it must be higher than the default gateway of ETH,
	// otherwise the kernel will use this default gateway when handling generic requests that are not bound to a specific interface
	// @TODO: we should not rely on the metric, instead we should set the default gateway in a way that it is only used for
	// requests that are made in the context of tap0.. Research how to do that.
	route.rt_metric = 10000;

	// Add the route to the routing table
	if (ioctl(sockfd, SIOCADDRT, &route) < 0) {
		perror("fail to set tap interface default gateway (ioctl)");
		close(tap->fd);
		close(sockfd);
		return -1;
	}

	tap->ip_address = ip_address;
	memcpy(tap->mac_address, mac_address, 6);
	close(sockfd);
	return 0;
}

int tap_send(Tap_Descriptor* tap, const uint8_t* packet_data, int32_t packet_size) {
	int32_t bytes_written = write(tap->fd, packet_data, packet_size);

	if (bytes_written < 0) {
		perror("unable to write spoofed packet to tap interface (write)");
		return -1;
	}

	assert(bytes_written == packet_size); // TODO: Is it possible in this particular case that 'write' will write less?

	return 0;
}

int32_t tap_receive(Tap_Descriptor* tap, uint8_t* buffer, uint32_t buffer_size) {
	int32_t bytes_read = read(tap->fd, buffer, buffer_size);
	if (bytes_read < 0) {
		perror("fail to read packets from tap interface (read)");
		return -1;
	}

	return bytes_read;
}

void tap_release(Tap_Descriptor* tap) {
	close(tap->fd);
}

uint32_t tap_get_ip_address(Tap_Descriptor* tap) {
	return tap->ip_address;
}

void tap_get_mac_address(Tap_Descriptor* tap, uint8_t mac_address[6]) {
	memcpy(mac_address, tap->mac_address, 6);
}