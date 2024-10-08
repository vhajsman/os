#ifndef __CONSOLE_H
#define __CONSOLE_H

#include "video/vga.h"
#include "types.h"

#define MAXIMUM_PAGES  16

#define SCROLL_UP     1
#define SCROLL_DOWN   2

/*
void console_clear(VGA_COLOR_TYPE foreground, VGA_COLOR_TYPE background);
void console_init(VGA_COLOR_TYPE foreground, VGA_COLOR_TYPE background);

void console_scroll(int line_count);

void console_putchar(char ch);

void console_ungetchar();
void console_ungetchar_bound(u8 n);

void console_gotoxy(u16 x, u16 y);

void console_putstr(const char *str);


void getstr(char *buffer);
svoid getstr_bound(char *buffer, u8 bound);
*/

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

#endif