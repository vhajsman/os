#! /bin/sh

rm $(find ./build/obj/ -type f) -f
mkdir -p ./build/obj/../bin
cd ./src

for ff in $(find -type d); do
    mkdir -p ../build/obj/$ff
done

for fa in $(find -name "*.asm" -type f); do
    nasm -felf32 -o ../build/obj/$fa.o  $fa
done

for fc in $(find -name "*.c" -type f); do
    gcc -I kernel -I drivers -I kernel/common/structures -I kernel/common -I libc -I kernel/core -I shell -I fs -I . \
        -m32 -std=gnu99 -ffreestanding -c \
        -Wall -Wextra \
        -D _BUILD_INSIDERS \
        -D _OPT_SEED_ENTROPY_RTC \
        -D _OPT_SEED_ENTROPY_CPUID \
        $fc -o ../build/obj/$fc.o
done

cd ..

ld -T config/linker.ld -o build/bin/krnl.tmp.elf $(find build/obj -name "*.o" -type f) \
    -m elf_i386 -nostdlib

nm -n build/bin/krnl.tmp.elf > build/kernel.map

cat build/kernel.map | awk '$2 == "T" { printf("    { 0x%s, \"%s\" },\n", $1, $3); }' > src/kernel/core/kernel.sym.entries

echo '#include "kernel/core/symbols.h"' > src/kernel/core/kernel.sym.c
echo 'symbol_t kernel_symbols[] = {' >> src/kernel/core/kernel.sym.c
cat src/kernel/core/kernel.sym.entries >> src/kernel/core/kernel.sym.c
echo '};' >> src/kernel/core/kernel.sym.c
echo 'size_t kernel_symbol_count = sizeof(kernel_symbols) / sizeof(kernel_symbols[0]);' >> src/kernel/core/kernel.sym.c

gcc -I src -m32 -std=gnu99 -ffreestanding -c src/kernel/core/kernel.sym.c -o build/obj/kernel/core/kernel.sym.c.o

ld -T config/linker.ld -o build/bin/krnl.bin $(find build/obj -name "*.o" -type f) \
    -m elf_i386 -nostdlib

sh ./script/initrd.sh
sh ./script/imggen.sh

echo "DONE!"