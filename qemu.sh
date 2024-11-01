#! /bin/sh
qemu-system-x86_64                                          \
    -M q35                                                  \
    -cpu pentium                                            \
    -smp sockets=1,cores=1,threads=1                        \
    -m 256                                                  \
    -vga std                                                \
    -device ac97                                            \
    -netdev user,id=n0                                      \
    -device rtl8139,netdev=n0                               \
    -accel tcg,thread=multi,tb-size=2048                    \
    -boot d                                                 \
    -device intel-iommu                                     \
    -cdrom build/*.iso                                      \
    -chardev stdio,id=char0,logfile=serial.log,signal=off   \
    -serial chardev:char0
