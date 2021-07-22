#!/bin/sh

IFS=' '

# Start the emulator with the pintos iso
qemu-system-x86_64 -drive if=ide,format=raw,file=pintos.img $*
