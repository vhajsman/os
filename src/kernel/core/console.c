#include "console.h"
#include "string.h"
#include "types.h"
#include "video/vga.h"
#include "hid/kbd.h"
#include "debug.h"
#include "ioport.h"

struct xy2d console_position;
    u8* console_posx = &console_position.x;
    u8* console_posy = &console_position.y;

u16* const VGA_MEMORY = (u16*) 0xB8000;

void printf(const char *format, ...) {
    char **arg = (char **)&format;
    char buf[32];
    int c;

    arg++;

    memset(buf, 0, sizeof(buf));

    while ((c = *format++) != 0) {
        if (c != '%') {
            putc(c);
        } else {
            char *p, *p2;
            int pad0 = 0, pad = 0;

            c = *format++;

            if (c == '0') {
                pad0 = 1;
                c = *format++;
            }

            if (c >= '0' && c <= '9') {
                pad = c - '0';
                c = *format++;
            }

            switch (c) {
                case 'd':
                case 'u':
                case 'x':
                    itoa(buf, c, *((int *)arg++));
                    p = buf;

                    goto string;

                    break;

                case 's':
                    p = *arg++;
                    if (!p)
                        p = "(null)";

                string:
                    for (p2 = p; *p2; p2++);
                    for (; p2 < p + pad; p2++)
                        putc(pad0 ? '0' : ' ');

                    while (*p)
                        putc(*p++);

                    break;

                default:
                    putc(*((int *)arg++));
                    break;
            }
        }
    }
}

coords console_position;

size_t console_row;
size_t console_column;

u8 console_color;
u16* console_buffer;
 
void console_initialize(void) {
	console_position.x = 0;
    console_position.y = 0;

	console_color = vga_entryColor(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	console_buffer = (u16*) 0xB8000;

	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			console_buffer[index] = vga_entry(' ', console_color);
		}
	}
}
 
void setColor(u8 color) {
	console_color = color;
}
 
void console_put(char c, u8 color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	console_buffer[index] = vga_entry(c, color);
}

#include "string.h"

void console_scroll() {
    for(u8 y = 0; y < VGA_HEIGHT; y++) {
        for(u8 x = 0; x < VGA_WIDTH; x++) {
            console_buffer[y * VGA_WIDTH + x] = console_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
}

// Makes a newline but keeps current X coordination
void console_nli() {
    if(console_position.y >= VGA_HEIGHT) {
        console_scroll();
        return;
    }

    console_position.y ++;
}

// Makes a newline
void console_nl() {
    console_position.x = 0;
    console_position.y++;

    if (console_position.y == VGA_HEIGHT) {
        console_scroll();

        console_position.y = VGA_HEIGHT - 1;
    }
}

void putc(char c) {
    if (c == '\n') {
        console_nl();
        return;
    }

    console_put(c, console_color, console_position.x, console_position.y);
    console_position.x++;

    if (console_position.x == VGA_WIDTH) {
        console_position.x = 0;
        console_position.y++;

        if (console_position.y == VGA_HEIGHT) {
            console_scroll();
            console_position.y = VGA_HEIGHT - 1;
        }
    }
}
 
void console_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		putc(data[i]);
}
 
void puts(const char* data) {
	console_write(data, strlen(data));
}

char getc() {
    return console_wait();
}

void gets(char* buffer, size_t bufferSize/*, char breaker*/) {
    if (buffer == NULL || bufferSize == 0) {
        debug_message("Buffer size should not be NULL.", "getc()", KERNEL_ERROR);
        return;
    }

    console_cursor_show();
    console_cursor_move(console_position.x, console_position.y);

    char l = '\0';
    size_t i = 0;

    while (i < bufferSize - 1) {
        l = getc();
        console_cursor_move(console_position.x + 1, console_position.y);

        if (l == '\0') {
            debug_messagen("EOF or string terminator @: ", "getc()", KERNEL_MESSAGE, l, 10);
            break;
        }

        putc(l);
        buffer[i] = l;
        
        if (l == '\n') {
            debug_messagen("Breaker char @: ", "getc()", KERNEL_MESSAGE, l, 10);
            break;
        }

        i++;
    }
    
    buffer[i] = '\0';

    console_cursor_hide();
}

void colorPrint(const char* str, u8 color) {
    u8 prevcolor = console_color;
    console_color = color;

    puts(str);

    console_color = prevcolor;
}

void console_gotoxy(u8 x, u8 y) {
    console_position.x = x;
    console_position.y = y;
}

coords console_wherexy() {
    return console_position;
}

u8 console_wherex() {
    return console_position.x;
}

u8 console_wherey() {
    return console_position.x;
}

char console_wait() {
    kbd_discard();
    char scancode;

    while(1) {
            scancode = kbd_getLastChar();

            if(scancode != 0x00)
                break;
    }

    return scancode;
}

u8 _cursor_visible = 1;
u8 _cursor_scanline_start;
u8 _cursor_scanline_end;

void console_cursor_enable(u8 scanline_start, u8 scanline_end) {
    _cursor_visible = 1;

    _cursor_scanline_start = scanline_start;
    _cursor_scanline_end   = scanline_end;

    outportb(0x3D4, 0x0A);
	outportb(0x3D5, (inportb(0x3D5) & 0xC0) | scanline_start);

	outportb(0x3D4, 0x0B);
	outportb(0x3D5, (inportb(0x3D5) & 0xE0) | scanline_end);
}

void console_cursor_disable() {
    _cursor_visible = 0;

    outportb(0x3D4, 0x0A);
	outportb(0x3D5, 0x20);
}

void console_cursor_move(int x, int y) {
	u16 pos = y * VGA_WIDTH + x;

	outportb(0x3D4, 0x0F);
	outportb(0x3D5, (u8) (pos & 0xFF));
	outportb(0x3D4, 0x0E);
	outportb(0x3D5, (u8) ((pos >> 8) & 0xFF));
}

u16 console_cursor_locate(void) {
    u16 pos = 0;

    outportb(0x3D4, 0x0F);
    pos |= inportb(0x3D5);

    outportb(0x3D4, 0x0E);
    pos |= ((u16) inportb(0x3D5)) << 8;

    return pos;
}

void console_cursor_show() {
    console_cursor_enable(_cursor_scanline_start, _cursor_scanline_end);
}

Console* console_current = NULL;

void console_switch(Console* console) {
    if(console == NULL)
        return;

    console_current = console;
    console_initialize();
    
    memcpy((void*) &console->videomem_addr, console->console_buffer, console->size_x * console->size_y);

    console_cursor_move(console->cursor_position_x, console->cursor_position_y);
    if(console->cursor_enable)
        console_cursor_enable(console->cursor_scanline_a, console->cursor_scanline_b);
}

void C_Console(Console* ptr, u16* videomem_addr, u8 size_x, u8 size_y, int depth, void (*stdio_putc)(const char c)) {
    if(ptr == NULL)
        return;

    ptr->videomem_addr = videomem_addr;
    ptr->depth = depth;
    ptr->size_x = size_x;
    ptr->size_y = size_y;
    ptr->stdio_putc = stdio_putc;

    ptr->position_x = 0;
    ptr->position_y = 0;
    ptr->color = 0x00;

    ptr->cursor_enable = 1;
    ptr->cursor_position_x = 0;
    ptr->cursor_position_y = 0;
    ptr->cursor_scanline_a = 0;
    ptr->cursor_scanline_b = 15;

    memset(ptr->console_buffer, 0x0000, ptr->size_x * ptr->size_y);
}