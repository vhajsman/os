#include "kbd.h"
#include "ioport.h"
#include "kernel.h"
#include "hid/kbdscan.h"
#include "irq/irqdef.h"
#include "irq/isr.h"
#include "console.h"
#include "memory/memory.h"
#include "string.h"
#include "time/timer.h"

void kbd_updateLeds();

void kbd_enable();
void kbd_disable();

u8 kbd_readStatus();
u8 kbd_readEncBuffer();
void kbd_sendData(u8 data);
void kbd_sendCommand(u8 command);

#define __KBD_STATE_NORMAL  0   // ...
#define __KBD_STATE_PREFIX  1   // 0xE0
#define __KBD_STATE_SPECIAL 2   // 0xE1

u8      _last_scancode;     // last scancode sent
char    _last_char;         // last character decoded
u8      _current_state;     // current keyboard state (prefix/normal key)
u8      _current_modifiers; // current key modifiers
u8      _enabled;           // keyboard enable/disable
u8      _init_done = 0;

static u8 _shift, _alt, _ctrl   = 0;
static u8 _caps, _num, _scroll  = 0;

void (*_event_callback)(kbd_event_t* event);
kbd_event_t _last_event;

kbd_event_t* kbd_getLastEvent() {
    return &_last_event;
}

#define KEY_COUNT 128

u8 key_down[KEY_COUNT]      = {0};
u32 key_lasttime[KEY_COUNT] = {0};
u32 _repeat_delay_ms        = PIT_FREQUENCY * 250 / 1000;
u32 _repeat_rate_ms         = PIT_FREQUENCY * 30  / 1000;

void process_scancode(u8 raw, u8 code, bool released) {
    debug_message("processing scancode: raw=", "kbd", KERNEL_MESSAGE);
    debug_number(raw, 16);

    if(raw == KEYBOARD_SCANCODE_PREFIX) {
        _current_state = __KBD_STATE_PREFIX;
        return;
    }

    if(released) {
        key_down[code] = 0;
//        return;
    }

    switch(code) {
        case SCAN_CODE_KEY_LEFT_SHIFT:  _shift = !released; return;
        case SCAN_CODE_KEY_LEFT_CTRL:   _ctrl  = !released; return;
        case SCAN_CODE_KEY_ALT:         _alt   = !released; return;

        case SCAN_CODE_KEY_CAPS_LOCK:
            if(!released) _caps ^= 1;
            kbd_updateLeds();
            return;

        case SCAN_CODE_KEY_NUM_LOCK:
            if(!released) _num ^= 1;
            kbd_updateLeds();
            return;

        case SCAN_CODE_KEY_SCROLL_LOCK:
            if(!released) _scroll ^= 1;
            kbd_updateLeds();
            return;

        default: break;
    }

    _last_scancode = code;
    _last_char = kbd_toChar(code, _shift ^ _caps, _alt);

    _last_event.scancode  = _last_scancode;
    _last_event.character = _last_char;

    _last_event.stmask    = (_caps  ? KEYBOARD_STMASK_CL    : 0) |
                            (_num   ? KEYBOARD_STMASK_NL    : 0) |
                            (_scroll? KEYBOARD_STMASK_SL    : 0);

    _last_event.modifiers = (_shift ? KEYBOARD_MASK_SHIFT   : 0) |
                            (_ctrl  ? KEYBOARD_MASK_CTRL    : 0) |
                            (_alt   ? KEYBOARD_MASK_ALT     : 0);

//    _last_event.evtype  = released
//                        ? KEYBOARD_EVENT_KEY_RELEASED
//                        : KEYBOARD_EVENT_KEY_PRESSED;

    _last_event.evtype = KEYBOARD_EVENT_KEY_PRESSED;
    _last_event.hanrtdone = 0;

    if(_event_callback)
        _event_callback(&_last_event);

    // pit_wait(_repeat_delay_ms);
}

u8 _scancode_isFirst = 1;

void kbd_irq(REGISTERS* r) {
    IGNORE_UNUSED(r);

    int _max_loops = 10;
    //while((kbd_readStatus() & KEYBOARD_CTRL_STATUS_MASK_OUT_BUF) && _max_loops--)
    //    (void) inportb(KEYBOARD_DATA_PORT);

    if(!_enabled || !_init_done) {
        goto cleanup;
        return;
    }

    if(_scancode_isFirst) {
        _scancode_isFirst = 0;

        goto cleanup;
        return;
    }

    kbd_disable();

    _max_loops = 10;
    while((kbd_readStatus() & KEYBOARD_CTRL_STATUS_MASK_OUT_BUF) && _max_loops--) {
        u32 now = pit_get();
        u8 raw = inportb(KEYBOARD_DATA_PORT);

        switch(raw) {
            case 0xFA:  debug_message("KEYBOARD_RESPONSE_ACK", "kbd", KERNEL_MESSAGE); continue;
            case 0xAA:  debug_message("KEYBOARD_SELF_TEST_OK", "kbd", KERNEL_MESSAGE); continue;
            case 0xEE:  debug_message("KEYBOARD_RESPONSE_ECHO","kbd", KERNEL_MESSAGE); continue;
        }

        if(raw == 0x00) {
            debug_message("invalid scancode: ", "kbd", KERNEL_ERROR);
            debug_number(raw, 16);

            continue;
        }

        bool released = raw & 0x80;
        u8 code = raw & 0x7F;

        if(released) {
            key_down[code] = 0;
            continue;
        }

        if(!key_down[code]) {
            key_down[code] = 1;
            key_lasttime[code] = now;
        }

        process_scancode(raw, code, released);
    }

    kbd_enable();
cleanup:
    while(kbd_readStatus() & KEYBOARD_CTRL_STATUS_MASK_OUT_BUF)
        (void) inportb(KEYBOARD_DATA_PORT);

    return;
}

void kbd_enable()   { _enabled = 1; }
void kbd_disable()  { _enabled = 0; }

/**
 * @brief read keyboards status port
 * 
 * @return u8 
 */
u8 kbd_readStatus() {
    return inportb(KEYBOARD_STATUS_PORT);
}

/**
 * @brief read keyboards data port
 * 
 * @return u8 
 */
u8 kbd_readEncBuffer() {
    return inportb(KEYBOARD_DATA_PORT);
}

/**
 * @brief write data to keyboard data port
 * 
 * @param data 
 */
void kbd_sendData(u8 data) {
    while(1)
        if((kbd_readStatus() & KEYBOARD_CTRL_STATUS_MASK_IN_BUF) == 0)
            break;

    outportb(KEYBOARD_DATA_PORT, data);
}

/**
 * @brief write data to keyboard command port (send command)
 * 
 * @param command 
 */
void kbd_sendCommand(u8 command) {
    while(1)
        if((kbd_readStatus() & KEYBOARD_CTRL_STATUS_MASK_IN_BUF) == 0)
            break;

    outportb(KEYBOARD_COMMAND_PORT, command);
}

void kbd_setLeds(int n, int c, int s) {
    u8 mask =   (s ? 1 : 0) | 
                (n ? 2 : 0) | 
                (c ? 4 : 0);

    kbd_sendData(KEYBOARD_COMMAND_SET_LEDS);
    while(inportb(KEYBOARD_DATA_PORT) != KEYBOARD_RESPONSE_ACK);

    kbd_sendData(mask);
    while(inportb(KEYBOARD_DATA_PORT) != KEYBOARD_RESPONSE_ACK);
}

void kbd_updateLeds() {
    kbd_setLeds(_num, _caps, _scroll);
}

void kbd_setEventHandler(void (*callback)(kbd_event_t* event)) {
    _event_callback = callback;
}

void kbd_init() {
    kbd_disable();
    _init_done = 0;

    while(kbd_readStatus() & KEYBOARD_CTRL_STATUS_MASK_OUT_BUF)
        (void) inportb(KEYBOARD_DATA_PORT);

    isr_registerInterruptHandler(IRQ_BASE + IRQ1_KEYBOARD, kbd_irq);
    kbd_setEventHandler(NULL);

    kbd_sendCommand(KEYBOARD_COMMAND_RESET);
    kbd_sendCommand(KEYBOARD_COMMAND_ECHO);

    int timeout = 10000;
    while(timeout--) {
        u8 val = kbd_readEncBuffer();
        if(val == KEYBOARD_RESPONSE_ECHO)
            break;
    }

    while(kbd_readStatus() & KEYBOARD_CTRL_STATUS_MASK_OUT_BUF)
        (void) inportb(KEYBOARD_DATA_PORT);

    kbd_enable();

    pit_wait(80);
    _init_done = 1;
    
    return;
}

u8 kbd_getLastKey()     { return _last_scancode; }
char kbd_getLastChar()  { return _last_char;     }

void kbd_discard() {
    _last_scancode = 0;
    _last_char = 0;

    _last_event.scancode    = 0x00;
    _last_event.stmask      = 0x00;
    _last_event.modifiers   = 0x00;
    _last_event.evtype      = 0x00;
    _last_event.character   = 0x00;
    _last_event.hanrtdone   = 1;
}




char kbd_toChar(u8 scancode, u8 uppercase, u8 altgr) {
    IGNORE_UNUSED(uppercase);

    #define ___K(n, u, alt) \
        return altgr    ? alt : (_caps || _shift ? u : n);
    
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

        case SCAN_CODE_KEY_Q:       ___K('q', 'Q', '\\');   break;
        case SCAN_CODE_KEY_W:       ___K('w', 'W', '|');    break;
        case SCAN_CODE_KEY_E:       ___K('e', 'E', 0);      break;
        case SCAN_CODE_KEY_R:       ___K('r', 'R', '\r');   break;
        case SCAN_CODE_KEY_T:       ___K('t', 'T', 0);      break;
        case SCAN_CODE_KEY_Y:       ___K('y', 'Y', 0);      break;
        case SCAN_CODE_KEY_U:       ___K('u', 'U', 0);      break;
        case SCAN_CODE_KEY_I:       ___K('i', 'I', 0);      break;
        case SCAN_CODE_KEY_O:       ___K('o', 'O', 0);      break;
        case SCAN_CODE_KEY_P:       ___K('p', 'P', 0);      break;
        case SCAN_CODE_KEY_A:       ___K('a', 'A', '~');    break;
        case SCAN_CODE_KEY_S:       ___K('s', 'S', 0);      break;
        case SCAN_CODE_KEY_D:       ___K('d', 'D', 0);      break;
        case SCAN_CODE_KEY_F:       ___K('f', 'F', '[');    break;
        case SCAN_CODE_KEY_G:       ___K('g', 'G', ']');    break;
        case SCAN_CODE_KEY_H:       ___K('h', 'H', '`');    break;
        case SCAN_CODE_KEY_J:       ___K('j', 'J', '\'');   break;
        case SCAN_CODE_KEY_K:       ___K('k', 'K', 0);      break;
        case SCAN_CODE_KEY_L:       ___K('l', 'L', 0);      break;
        case SCAN_CODE_KEY_Z:       ___K('z', 'Z', 0);      break;
        case SCAN_CODE_KEY_X:       ___K('x', 'X', '#');    break;
        case SCAN_CODE_KEY_C:       ___K('c', 'C', '&');    break;
        case SCAN_CODE_KEY_V:       ___K('v', 'V', '@');    break;
        case SCAN_CODE_KEY_B:       ___K('b', 'B', '{');    break;
        case SCAN_CODE_KEY_N:       ___K('n', 'N', '}');    break;
        case SCAN_CODE_KEY_M:       ___K('m', 'M', '^');    break;

        case SCAN_CODE_KEY_SPACE:   
            return ' '; 
            break;

        case SCAN_CODE_KEY_MINUS:
        case SCAN_CODE_KEY_KEYPAD_MINUS:
            return '-';
            break;

        case SCAN_CODE_KEY_KEYPAD_PLUS:
            return '+';
            break;

        case SCAN_CODE_KEY_EQUAL:
            return '='; 
            break;

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
