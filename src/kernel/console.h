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

void printf(const char *format, ...);

void console_initialize(void);
void setColor(u8 color);
void console_put(char c, u8 color, size_t x, size_t y);
void console_write(const char* data, size_t size);

void putc(char c);
void puts(const char* data);

char getc();
char* gets(size_t bufferSize, char breaker);

void colorPrint(const char* str, u8 color);

#endif