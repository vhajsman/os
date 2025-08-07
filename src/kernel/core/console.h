#ifndef __CONSOLE_H
#define __CONSOLE_H

#include "video/vga.h"
#include "types.h"

#define MAXIMUM_PAGES  16

#define SCROLL_UP     1
#define SCROLL_DOWN   2

typedef struct kernel_console {
    u16*    videomem_addr;                              // pointer to video memory to use
    u16     console_buffer[VGA_WIDTH * VGA_HEIGHT];     // pointer to console buffer
    int     depth;

    u8      position_x;
    u8      position_y;
    u8      size_x;
    u8      size_y;
    u16     color;

    u8      cursor_enable;
    u8      cursor_position_x;
    u8      cursor_position_y;
    u8      cursor_scanline_a;
    u8      cursor_scanline_b;

    void (*stdio_putc)(const char c);
} Console;

void console_switch(Console* console);

extern struct xy2d console_position;
extern u8* console_posx;
extern u8* console_posy;

void printf(const char *format, ...);

void console_initialize(void);
void setColor(u8 color);
void console_put(char c, u8 color, size_t x, size_t y);
void console_write(const char* data, size_t size);

void putc(char c);
void puts(const char* data);

char getc();
void gets(char* buffer, size_t bufferSize);

void colorPrint(const char* str, u8 color);

void console_gotoxy(u8 x, u8 y);
coords console_wherexy();

u8 console_wherex();
u8 console_wherey();

void console_scroll();
void console_nli();
void console_nl();

// Wait for keypress (any key) and returns the scancode
char console_wait();

#define console_cursor_hide console_cursor_disable

void console_cursor_enable(u8 scanline_start, u8 scanline_end);
void console_cursor_disable();
void console_cursor_move(int x, int y);
u16 console_cursor_locate(void);
void console_cursor_show();

// =========================================================
// ===== KEY BUFFER
// =========================================================

void keybuffer_disable();
void keybuffer_enable(u8 printOnAppend);
void keybuffer_set(unsigned char* val);
void keybuffer_append(char c);
void keybuffer_discard();
const unsigned char* keybuffer_read();
const unsigned char* keybuffer_wait(char breaker);
void keybuffer_nowait();

extern size_t _keyb_size;

#endif