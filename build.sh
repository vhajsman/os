rm $(find ./build/obj/ -type f)

mkdir -p ./build/obj/../bin
cd ./src

for ff in $(find -type d); do
    mkdir -p ../build/obj/$ff
done


for fa in $(find -name "*.asm" -type f); do
    nasm -felf32 -o ../build/obj/$fa.o  $fa
done

for fc in $(find -name "*.c" -type f); do
    gcc -I kernel -I drivers -I kernel/type -I kernel/structures -I . \
        -m32 -std=gnu99 -ffreestanding -c \
        -Wall -Wextra \
        $fc -o ../build/obj/$fc.o
done

cd ..

ld -T config/linker.ld -o build/bin/krnl.bin $(find build/obj -name "*.o" -type f) \
    -m elf_i386 -nostdlib

sh ./script/imggen.sh

echo "DONE!"
