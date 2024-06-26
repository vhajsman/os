#include "console.h"
#include "string.h"
#include "types.h"
#include "video/vga.h"
#include "hid/kbd.h"
#include "debug.h"

// #include "hid/kbd.h"
// #include "hid/kbdscan.h"

struct xy2d console_position;
u8* console_posx = &console_position.x;
u8* console_posy = &console_position.y;

static u16* const VGA_MEMORY = (u16*) 0xB8000;

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
    // console_position.x = 0;

    if(console_position.y >= VGA_HEIGHT) {
        console_scroll();
        return;
    }

    console_position.y ++;
}

// Makes a newline
void console_nl() {
    console_position.x = 0;

    if(console_position.y >= VGA_HEIGHT) {
        console_scroll();
        return;
    }

    console_position.y ++;
}

// void console_update() {
//     memmove(VGA_MEMORY, console_buffer);
// }

void putc(char c) {
    if(c == '\n') {
        // console_position.x = 0;
        // console_position.y++;

        console_nl();

        return;
    }

    // if(c == '\b') {
    //     console_put(' ', console_color, console_position.x, console_position.y);
    // 
    //     if(console_position.x == 0 && console_position.y == 0)
    //         return;
    // 
    //     if(console_position.x == 0 && console_position.y != 0) {
    //         console_position.x = VGA_WIDTH;
    //         console_position.y --;
    //     } else {
    //         console_position.x--;
    //     }
    // }

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

    if(console_position.x == VGA_WIDTH) {
        if(console_position.y == VGA_HEIGHT) {
            console_position.x = 0;
            console_position.y = 0;
    
            return;
        }
    
        console_position.x = 0;
        console_position.y =+ 1;
    }

	// console_put(c, console_color, console_position.x - 1, console_position.y); 

    console_put(c, console_color, console_position.x, console_position.y);

    if(++console_posx == VGA_WIDTH) {
        console_position.x = 0;

        if(++console_posy == VGA_HEIGHT) {
            console_scroll();
            // console_update();
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
    static char* buffer;
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
