#!/bin/bash
mkdir -p bin
gcc -o bin/main src/main.c src/tap.c src/in.c src/out.c src/eth.c src/packet.c src/util.c -g
