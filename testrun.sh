#!/bin/sh

make clean
make
cp pintos.bin isodir/boot/pintos.bin
grub-mkrescue isodir -o pintos.iso
qemu-system-i386 -cdrom pintos.iso
