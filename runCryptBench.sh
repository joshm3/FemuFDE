#!/bin/bash

sudo apt install libtomcrypt-dev cryptsetup-bin libssl-dev
gcc symCryptBench.c -o scb -lcrypto -ltomcrypt
./scb
rm ./scb
