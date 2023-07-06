#!/bin/sh
#===============================================================================
#          FILE:  qemu-start.sh
#         USAGE:  ./qemu-start.sh 
#   DESCRIPTION:  
#       OPTIONS:  ---
#  REQUIREMENTS:  ---
#          BUGS:  ---
#         NOTES:  ---
#        AUTHOR: LuoQiaofa (Luoqf), luoqiaofa@163.com
#  ORGANIZATION: 
#       CREATED: 06/29/2023 02:03:19 PM CST
#      REVISION:  ---
#===============================================================================
argc=$#
elf="out/luos.elf"
if [ $argc -eq 0 ]
then
    qemu-system-arm -M mcimx6ul-evk -smp 1 -m 512M -kernel ${elf} -nographic
else
    qemu-system-arm -M mcimx6ul-evk -smp 1 -m 512M -kernel ${elf} -nographic -s -S
fi

