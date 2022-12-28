#!/bin/bash

IFS=' '

# Copy latest OVMF bios over
cp /usr/share/edk2-ovmf/x64/OVMF_CODE.fd $HOME/pintos/qemu_dir/OVMF_CODE.fd
cp /usr/share/edk2-ovmf/x64/OVMF_CODE.fd $HOME/pintos/qemu_dir/OVMF_CODE.secboot.fd

# Start the emulator with UEFI BIOS and latest uefi app image
qemu-system-x86_64 -m size=2048M -net none -cpu max,sse=on,sse2=on,avx=on -smp 2,cores=2,threads=1,dies=1,sockets=1 $* \
	-drive if=pflash,format=raw,unit=0,file=$HOME/pintos/qemu_dir/OVMF_CODE.fd,readonly=on \
	-drive if=pflash,format=raw,unit=1,file=$HOME/pintos/qemu_dir/OVMF_VARS.fd \
	-drive if=ide,format=raw,file=$HOME/pintos/uefi.img
