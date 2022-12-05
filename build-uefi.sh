#!/bin/sh

# Buld pintos and the grub boot efi
make all
grub-mkstandalone grub_dir -O x86_64-efi --modules="normal part_msdos ext2 multiboot multiboot2 all_video loadbios loadenv" -o BOOTX64.EFI "boot/grub/grub.cfg=config/grub_uefi.cfg" 

# Create uefi boot image containing the app
dd if=/dev/zero of=./uefi.img bs=512 count=93750
parted ./uefi.img -s -a minimal mklabel gpt
parted ./uefi.img -s -a minimal mkpart EFI FAT16 2048s 93716s
parted ./uefi.img -s -a minimal toggle 1 boot

dd if=/dev/zero of=/tmp/part.img bs=512 count=91669
mformat -i /tmp/part.img -h 32 -t 32 -n 64 -c 1
mmd -i /tmp/part.img ::/boot
mmd -i /tmp/part.img ::/boot/grub
mmd -i /tmp/part.img ::/EFI
mmd -i /tmp/part.img ::/EFI/BOOT
mcopy -i /tmp/part.img ./BOOTX64.EFI ::/EFI/BOOT/BOOTX64.EFI
mcopy -i /tmp/part.img ./grub_dir/boot/grub/grub.cfg ::/boot/grub/grub.cfg
mcopy -i /tmp/part.img ./pintos_boot.bin ::/boot/pintos_boot.bin
mcopy -i /tmp/part.img ./pintos_kernel.bin ::/boot/pintos_kernel.bin
dd if=/tmp/part.img of=./uefi.img bs=512 count=91669 seek=2048 conv=notrunc
rm /tmp/part.img
