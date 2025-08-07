#include "kernel.h"
#include "hid/kbdscan.h"
#include "console.h"
#include "memory/memory.h"
#include "string.h"

#define KEYBUFFER_SIZE_DEFAULT  1024

u8 _keyb_enable = 0;
u8 _keyb_wait = 0;

u8 _keyb_putc = 0;

size_t _keyb_size = KEYBUFFER_SIZE_DEFAULT;
size_t _keyb_index = 0;

u8 _keyb_color_deleted;

char _ignore;

static unsigned char* _keyb;

void keybuffer_disable() {
    _keyb_enable = 0;
    debug_message("Disabled", "keybuffer", KERNEL_MESSAGE);
}

void keybuffer_enable(u8 printOnAppend) {
    _keyb_enable = 1;
    _keyb_putc = printOnAppend;

    _keyb_color_deleted = vga_entryColor(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    
    debug_message("Enabled", "keybuffer", KERNEL_MESSAGE);
}

void keybuffer_set(unsigned char* val) {
    if (val == NULL) {
        // Handle error: invalid buffer
        return;
    }

    _keyb = val;
    _keyb_index = 0;

    memset(_keyb, 0, _keyb_size);
}

void keybuffer_backspace(int pos) {
    IGNORE_UNUSED(pos);
    
    // TODO: Make use of pos argument

    if(_keyb_index == 0)
        return;
    
    _keyb_index = _keyb_index - 1;
    _keyb[_keyb_index] = '\0';

    if(console_position.x > 0) {
        console_gotoxy(console_position.x - 1, console_position.y);
    } else if(console_position.y > 0) {
        console_gotoxy(VGA_WIDTH - 1, console_position.y - 1);
    }

    putc(' ');
    
    if(console_position.x > 0) {
        console_gotoxy(console_position.x - 1, console_position.y);
    } else if(console_position.y > 0) {
        console_gotoxy(VGA_WIDTH - 1, console_position.y - 1);
    }

    return;
}

void keybuffer_append(char c) {
    if(_ignore == c)
        return;

    if(c == '\b') {
        keybuffer_backspace(0);
        return;
    }

    if(_keyb_enable && _keyb_index < _keyb_size) {
        _keyb[_keyb_index] = c;
        _keyb_index++;

        if(_keyb_putc) {
            putc(c);
        }
    }
}

void keybuffer_discard() {
    _keyb_index = 0;

    if (_keyb != NULL) {
        memset(_keyb, 0, _keyb_size);
    }

    debug_message("Discard", "keybuffer", KERNEL_MESSAGE);
}

const unsigned char* keybuffer_read() {
    if (_keyb == NULL) {
        return NULL;
    }

    _keyb[_keyb_index] = '\0';
    return _keyb;
}

const unsigned char* keybuffer_wait(char breaker) {
    _keyb_wait = 1;
    _ignore = breaker;

    debug_message("Waiting for breaker character", "keybuffer", KERNEL_MESSAGE);

    while (_keyb_wait) {
        if (_keyb_index > 0 && _keyb[_keyb_index - 1] == breaker) {
            putc(breaker);
            break;
        }

        if (_keyb_index >= _keyb_size) {
            break;
        }

        if (!_keyb_wait || !_keyb_enable) {
            puts("Keyboard disabled or wait cancelled while keybuffer waiting!! break.\n");
            break;
        }
    }

    keybuffer_nowait();
    return keybuffer_read();
}

void keybuffer_nowait() {
    debug_message("Escaped breaker character", "keybuffer", KERNEL_MESSAGE);

    _keyb_wait = 0;
}
