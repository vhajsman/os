#! /bin/sh
mkdir initrd/
mkdir initrd/dev
mkdir initrd/mount
mkdir initrd/config
mkdir initrd/bin
cd initrd
tar -cvf ../build/iso/boot/initrd.tar *