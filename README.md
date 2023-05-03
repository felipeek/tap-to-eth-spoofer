# tap-to-eth-spoofer

Small application that creates and listens to a tap device, dispatching all packets to a physical eth interface transparently.

Packets can be spoofed when being dispatched from tap to eth, as well as from eth to tap, by injecting code in `tap_spoof.c` and `eth_spoof.c`, respectively.

This application works by creating a tap device with an IP that is within eth interface's subnet, such that packets can be dispatched as they arrive, and the tap device will "appear" as an independent device to eth's network, with its own IP.

## Build

Only Linux is supported. Simply run:

```bash
$ make
```

The binary will be available in `bin/tap-to-eth-spoofer/tap-to-eth-spoofer`

## Run

```bash
$ bin/tap-to-eth-spoofer/tap-to-eth-spoofer 
usage: bin/tap-to-eth-spoofer/tap-to-eth-spoofer <tap_interface_name> <eth_interface_name> <tap_interface_ip> <tap_interface_mac>
example: bin/tap-to-eth-spoofer/tap-to-eth-spoofer tap0 eth0 192.168.0.22 82:a2:17:43:15:ff
	 - <tap_interface_name>: an arbitrary name for the tap interface, will be automatically created, e.g. tap0
	 - <eth_interface_name>: the name of the eth interface, which must already exist, e.g. eth0
	 - <tap_interface_ip>: the ip of the tap interface, it is arbitrary, however, it must be within the same subnet of the eth interface, e.g. 192.168.0.22
	 - <tap_interface_mac>: the mac of the tap interface, it is arbitrary, e.g. 82:a2:17:43:15:ff
```