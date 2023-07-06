#!/bin/sh
dd iflag=dsync oflag=dsync if=printf.imx of=/dev/mmcblk0 bs=512 seek=2

