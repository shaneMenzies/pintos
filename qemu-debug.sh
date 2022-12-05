#!/bin/bash

echo 'done'
/home/srmenzies/pintos/qemu-uefi.sh -s -S -no-reboot -no-shutdown -d cpu_reset,guest_errors
