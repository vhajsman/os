#include "video/vga.h"
#include "ioport.h"
#include "types.h"

// =========================================================
// ===== VGA ENTRIES
// =========================================================

u8 vga_entryColor(enum vga_color fg, enum vga_color bg) {
	return fg | bg << 4;
}
 
u16 vga_entry(unsigned char uc, u8 color) {
	return (u16) uc | (u16) color << 8;
}


// =========================================================
// ===== VGA CURSOR
// =========================================================

u8 _cursor_start = 0;
u8 _cursor_end = 15;

coords _cursor_pos;

void cursor_set(u8 start, u8 end) {
	outportb(0x3D4, 0x0A);
	outportb(0x3D5, (inportb(0x3D5) & 0xC0) | start);

	outportb(0x3D4, 0x0B);
	outportb(0x3D5, (inportb(0x3D5) & 0xE0) | start);

	_cursor_start = start;
	_cursor_end = end;
}

void cursor_enable() {
	cursor_set(_cursor_start, _cursor_end);
}

void cursor_disable() {
	outportb(0x3D4, 0x0A);
	outportb(0x3D5, 0x0B);
}

void cursor_update(coords pos) {
	u16 _pos = pos.y * VGA_WIDTH + pos.x;

	outportb(0x3D4, 0x0F);
	outportb(0x3D5, (u8) (_pos & 0xFF));

	outportb(0x3D4, 0x0E);
	outportb(0x3D5, (u8) ((_pos >> 8) & 0xFF));

	_cursor_pos = pos;
}

coords cursor_get() {
	return _cursor_pos;
}
