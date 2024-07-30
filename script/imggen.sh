echo "Generating image"

mkdir -p build/iso/boot/grub

cp config/grub.cfg build/iso/boot/grub/
cp build/bin/* build/iso/

grub-mkrescue -o build/cubebox.iso build/iso
