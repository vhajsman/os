#include "console.h"
#include "string.h"
#include "types.h"
#include "video/vga.h"
#include "hid/kbd.h"

// #include "hid/kbd.h"
// #include "hid/kbdscan.h"

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
 
void putc(char c) {
    if(c == '\n') {
        console_position.x = 0;
        console_position.y++;

        return;
    }

    if(c == '\b') {
        if(console_position.x == 0 && console_position.y != 0) {
            console_position.x = VGA_WIDTH;
            console_position.y --;
        }

        if(console_position.x == 0 && console_position.y == 0) {
            return;
        }

        console_position.x--;
    }

	if (++console_position.x == VGA_WIDTH) {
        if(console_position.y != VGA_HEIGHT) {
		    console_position.x = 0;
            console_position.y++;
        } else {
            for(size_t i = 0; i < VGA_WIDTH; i++) {
                console_buffer[i] = console_buffer[i + 1];
            }

            console_position.x = 0;
            console_position.y--;
        }
	}

	console_put(c, console_color, console_position.x, console_position.y);
}
 
void console_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		putc(data[i]);
}
 
void puts(const char* data) {
	console_write(data, strlen(data));
}

char getc() {
    char c;

    kbd_discard();
    while(1) {
        c = kbd_getLastChar();

        if(c != 0x00)
            return c;
    }

    return c;
}

char* gets(size_t bufferSize, char breaker) {
    char buffer[bufferSize];
    // buffer = 0;

    char l = 0;

    for(size_t i = 0; i < bufferSize; i ++) {
        l = getc();

        putc(l);

        if(l == breaker)
            break;
        
        buffer[i] = l;
    }

    return buffer;
}
