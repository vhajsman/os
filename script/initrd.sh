#! /bin/sh
mkdir initrd/
mkdir initrd/dev
mkdir initrd/mount
mkdir initrd/config
mkdir initrd/bin
mkdir initrd/usr/libs --parent  

rm -rf initrd/usr/libs/*
cp -r src/lib* initrd/usr/libs/

cd initrd
tar -cvf ../build/iso/boot/initrd.tar *