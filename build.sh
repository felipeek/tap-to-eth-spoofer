#!/bin/bash
mkdir -p bin
gcc -o bin/main src/main.c src/eth_receive.c src/tap_receive.c src/tap.c src/eth_spoof.c src/tap_spoof.c src/eth.c src/packet.c src/util.c -g -lpthread
