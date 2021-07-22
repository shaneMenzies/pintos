#!/bin/sh

make clean
make
dd if=/dev/zero of=pintos.img bs=512 count=131072
chmod a+rwx pintos.img
parted pintos.img -s -a minimal mklabel msdos
parted pintos.img -s -a minimal mkpart primary fat32 2048s 131071s
parted pintos.img -s -a minimal toggle 1 boot
losetup /dev/loop0 pintos.img
losetup /dev/loop1 pintos.img -o 1048576
mkdosfs -F32 -f 2 /dev/loop1
mount /dev/loop1 /mnt
grub-install --target=i386-pc --root-directory=/mnt --no-floppy --modules="normal part_msdos ext2 multiboot multiboot2" /dev/loop0
cp pintos_boot.bin /mnt/boot/pintos_boot.bin
cp pintos_kernel.bin /mnt/boot/pintos_kernel.bin
cp config/grub_bios.cfg /mnt/boot/grub/grub.cfg
sync
umount /mnt
losetup -d /dev/loop0
losetup -d /dev/loop1
make clean