#!/bin/bash
mkdir -p bin
gcc -o bin/main src/main.c src/eth_receive.c src/tap_receive.c src/tap.c src/in.c src/out.c src/eth.c src/packet.c src/util.c -g -lpthread
