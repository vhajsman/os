echo "Generating image"

mkdir -p build/iso/boot/grub

cp res/cbos-grub-bg.png build/iso/boot/grub/background.png
cp config/grub.cfg build/iso/boot/grub/
cp build/bin/* build/iso/boot/

sudo grub-mkrescue -o build/cubebox.iso build/iso
