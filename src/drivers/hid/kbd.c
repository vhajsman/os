#include "kbd.h"
#include "ioport.h"
#include "kernel.h"
#include "hid/kbdscan.h"
#include "irq/irqdef.h"
#include "irq/isr.h"
#include "console.h"
#include "memory/memory.h"
#include "string.h"


static BOOL _kbd_enable;

u8 _ctrl, _shift, _alt;
u8 _capslock, _numlock, _scrollock;
u8 _scan;

u8 _self, _diagnosis;

char _last_char;

#define _KEYBOARD_MAX_RESENDS 5
#define _KEYBOARD_TIMEOUT_CYCLES 1000

//void resend();

u8 kbd_readStatus() {
    return inportb(KEYBOARD_STATUS_PORT);
}

u8 kbd_readEncBuffer() {
    return inportb(KEYBOARD_DATA_PORT);
}

void kbd_sendEncCommand(u8 command) {
    while(1) {
        if((kbd_readStatus() & KEYBOARD_CTRL_STATUS_MASK_IN_BUF) == 0)
            break;
    }

    outportb(KEYBOARD_DATA_PORT, command);
}

void kbd_sendCommand(u8 command) {
    while(1) {
        if((kbd_readStatus() & KEYBOARD_CTRL_STATUS_MASK_IN_BUF) == 0)
            break;
    }

    outportb(KEYBOARD_COMMAND_PORT, command);
}

#define SEND_COMMAND(COMMAND)   kbd_sendCommand(COMMAND)
#define SEND_DATA(DATA)         kbd_sendEncCommand(COMMAND)

void kbd_setLeds(int n, int c, int s) {
    u8 data = (s ? 1 : 0) | (n ? 2 : 0) | (c ? 4 : 0);

    kbd_sendEncCommand(KEYBOARD_COMMAND_SET_LEDS);
    kbd_sendEncCommand(data);
}

void kbd_irq(REGISTERS* r) {
    IGNORE_UNUSED(r);

    //asm("add esp, 12");
    //asm("pushad");
    //asm("cli");

    //static int ext = 0;

    char c;

    u8 scancode;
    int setLeds = 0;

    if(kbd_readStatus() & KEYBOARD_CTRL_STATUS_MASK_OUT_BUF) {
        scancode = kbd_readEncBuffer();
        c = kbd_toChar(scancode, _capslock || _shift, _alt);

        if(scancode > 0x58)
            return;

        _scan = scancode;
        _last_char = c;

        if(_shift == 1)
            _shift = 0;

        if(scancode & 0x80) {
            scancode -= 0x80;

            switch(scancode) {
                case SCAN_CODE_KEY_LEFT_CTRL:
                //case SCAN_CODE_KEY_RIGHT_CTRL:
                    _ctrl = 0;
                    break;
                
                case SCAN_CODE_KEY_LEFT_SHIFT:
                case SCAN_CODE_KEY_RIGHT_SHIFT:
                    _shift = 0;
                    break;

                case SCAN_CODE_KEY_ALT:
                    _alt = 0;
                    break;

                default:
                    break;
            }
        } else {
            _scan = scancode;

            switch (scancode) {
                case SCAN_CODE_KEY_LEFT_CTRL:
                //case SCAN_CODE_KEY_RIGHT_CTRL:
                    _ctrl = 1;
                    break;
                
                case SCAN_CODE_KEY_LEFT_SHIFT:
                    puts("S");
                    _shift = 1;
                    break;
                
                // ? idk why but doesnt work lmao
                //case SCAN_CODE_LEFT_SHIFT_RELEASE:
                //case SCAN_CODE_RIGHT_SHIFT_RELEASE:
                //    _shift = 0;
                //    break;

                case SCAN_CODE_KEY_ALT:
                    _alt = 1;
                    break;



                //case SCAN_CODE_KEY_SCROLL_LOCK:
                //case SCAN_CODE_KEY_CAPS_LOCK:
                //case SCAN_CODE_KEY_NUM_LOCK:


                case SCAN_CODE_KEY_CAPS_LOCK:
                    _capslock = !_capslock;
                    setLeds = 1;
                    break;
                
                case SCAN_CODE_KEY_NUM_LOCK:
                    _numlock = !_numlock;
                    setLeds = 1;
                    break;
                
                case SCAN_CODE_KEY_SCROLL_LOCK:
                    _scrollock = !_scrollock;
                    setLeds = 1;
                    break;



                default:
                    break;
            }
        }

        switch (scancode) {
            case KEYBOARD_RESPONSE_SELF_FAIL:
                _self = 1;
                break;
            
            case KEYBOARD_RESPONSE_RESEND:
                //resend();
                break;

            default:
                break;
        }
    }

    if(setLeds)
        kbd_setLeds(_numlock, _capslock, _scrollock);

     //puts("Keyboard interrupt.");
     //puts("Key: ");
     //putc(c);
     //putc('\n');


    //_scan = scancode;
    //_last_char = c;

    keybuffer_append(c);
}

u8 kbd_getLastKey() {
    return _scan;
}

char kbd_getLastChar() {
    return _last_char;
}

void kbd_discard() {
    _scan = 0;
    _last_char = 0;
}

void kbd_enable() {
    _kbd_enable = TRUE;
}

void kbd_disable() {
    _kbd_enable = FALSE;
}

char kbd_toChar(u8 scancode, u8 uppercase, u8 altgr) {
    #define ___K(n, u, alt)         \
        if(altgr) {                 \
            return alt;             \
        }                           \
                                    \
        if(_capslock || _shift) {   \
            return u;               \
        }                           \
                                    \
        return n;
    
    switch (scancode) {
        case SCAN_CODE_KEY_0:       return '0'; break;
        case SCAN_CODE_KEY_1:       return '1'; break;
        case SCAN_CODE_KEY_2:       return '2'; break;
        case SCAN_CODE_KEY_3:       return '3'; break;
        case SCAN_CODE_KEY_4:       return '4'; break;
        case SCAN_CODE_KEY_5:       return '5'; break;
        case SCAN_CODE_KEY_6:       return '6'; break;
        case SCAN_CODE_KEY_7:       return '7'; break;
        case SCAN_CODE_KEY_8:       return '8'; break;
        case SCAN_CODE_KEY_9:       return '9'; break;

        case SCAN_CODE_KEY_Q:       ___K('q', 'Q', '\\'); break;
        case SCAN_CODE_KEY_W:       ___K('w', 'W', '|');  break;
        case SCAN_CODE_KEY_E:       ___K('e', 'E', 0); break;
        case SCAN_CODE_KEY_R:       ___K('r', 'R', '\r'); break;
        case SCAN_CODE_KEY_T:       ___K('t', 'T', 0); break;
        case SCAN_CODE_KEY_Y:       ___K('y', 'Y', 0); break;
        case SCAN_CODE_KEY_U:       ___K('u', 'U', 0); break;
        case SCAN_CODE_KEY_I:       ___K('i', 'I', 0); break;
        case SCAN_CODE_KEY_O:       ___K('o', 'O', 0); break;
        case SCAN_CODE_KEY_P:       ___K('p', 'P', 0); break;
        case SCAN_CODE_KEY_A:       ___K('a', 'A', '~'); break;
        case SCAN_CODE_KEY_S:       ___K('s', 'S', 0); break;
        case SCAN_CODE_KEY_D:       ___K('d', 'D', 0); break;
        case SCAN_CODE_KEY_F:       ___K('f', 'F', '['); break;
        case SCAN_CODE_KEY_G:       ___K('g', 'G', ']'); break;
        case SCAN_CODE_KEY_H:       ___K('h', 'H', '`'); break;
        case SCAN_CODE_KEY_J:       ___K('j', 'J', '\''); break;
        case SCAN_CODE_KEY_K:       ___K('k', 'K', 0); break;
        case SCAN_CODE_KEY_L:       ___K('l', 'L', 0); break;
        case SCAN_CODE_KEY_Z:       ___K('z', 'Z', 0); break;
        case SCAN_CODE_KEY_X:       ___K('x', 'X', '#'); break;
        case SCAN_CODE_KEY_C:       ___K('c', 'C', '&'); break;
        case SCAN_CODE_KEY_V:       ___K('v', 'V', '@'); break;
        case SCAN_CODE_KEY_B:       ___K('b', 'B', '{'); break;
        case SCAN_CODE_KEY_N:       ___K('n', 'N', '}'); break;
        case SCAN_CODE_KEY_M:       ___K('m', 'M', '^'); break;
        case SCAN_CODE_KEY_SPACE:   return ' '; break;

        case SCAN_CODE_KEY_MINUS:
        case SCAN_CODE_KEY_KEYPAD_MINUS:
            return '-';
            break;
        case SCAN_CODE_KEY_KEYPAD_PLUS:
            return '+';
            break;

        case SCAN_CODE_KEY_EQUAL:   return '='; break;

        case SCAN_CODE_KEY_SQUARE_OPEN_BRACKET:
            ___K('[', '{', '/');
            break;
        case SCAN_CODE_KEY_SQUARE_CLOSE_BRACKET:
            ___K(']', '}', ')');
            break;

        case SCAN_CODE_KEY_SEMICOLON:
            ___K(':', '"', ';');
            break;
        case SCAN_CODE_KEY_SINGLE_QUOTE:
            ___K('\'', '|', '\\');
            break;

        case SCAN_CODE_KEY_COMMA:   ___K(',', '<', '?'); break;
        case SCAN_CODE_KEY_DOT:     ___K('.', '>', ':'); break;

        case SCAN_CODE_KEY_ASTERISK:return '*'; break;

        case SCAN_CODE_KEY_FORESLHASH:
            ___K('_', '-', '*');
            break;

        case SCAN_CODE_KEY_BACKSPACE:
            return '\b';
            break;
        case SCAN_CODE_KEY_TAB:
            return '\t';
            break;
        case SCAN_CODE_KEY_ENTER:
            return '\n';
            break;
    
        default:
            // Invalid scancode
            return 0x00;
    }
}

void kbd_init() {
    isr_registerInterruptHandler(IRQ_BASE + IRQ1_KEYBOARD, kbd_irq);

    kbd_enable();

    // puts("Waiting for KBD to respond...\n");

    
    kbd_sendCommand(KEYBOARD_COMMAND_ECHO);
    for(int i = 0; i < 10000; i ++) {
        if(kbd_readStatus() == KEYBOARD_RESPONSE_ECHO) {
            goto echoOk;
            return;
        }
    }


    // puts("KBD: Keyboard not responding.\n");

    return;


    echoOk:
        puts("KBD: OK\n");
        return;
}


/*
void setKeyHandler(void(*handler)(char c)) {
    keyh = handler;
}

void unsetKeyHandler() {
    keyh = 0;
}
*/

// =========================================================
// ===== KEY BUFFER
// =========================================================
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

    console_gotoxy(console_position.x - 1, console_position.y);
    putc('\0');

    return;
}

void keybuffer_append(char c) {
    if(_ignore == c)
        return;

    if(c == '\b') {
//        if(_keyb_index == 0)
//            return;
//
//        _keyb_index--;
//        _keyb[_keyb_index] = '\0';
//
//        console_gotoxy(console_position.x - 1, console_position.y);
//        putc(' ');
//
//        debug_message("Backspace insert", "keybuffer", KERNEL_MESSAGE);
//
//        return;

        keybuffer_backspace(0);
    }

    if (_keyb_enable && _keyb_index < _keyb_size) {

        _keyb[_keyb_index] = c;
        _keyb_index++;

        if (_keyb_putc) {
            putc(c);
        }
        
        debug_message("Key append", "keybuffer", KERNEL_MESSAGE);
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