qemu-system-x86_64 -kernel build/bin/krnl.bin -m 64 -chardev stdio,id=char0,logfile=serial.log,signal=off -serial chardev:char0