#ifndef __MOUSE_H
#define __MOUSE_H

#include "ioport.h"

void mouse_write(u8 command);
u8 mouse_read();
void mouse_init();

enum mouse_event_type {
    mouse_event_none,
    mouse_event_move,
    mouse_event_scroll,
    mouse_event_button_pressed_l,
    mouse_event_button_pressed_r,
    mouse_event_button_release_l,
    mouse_event_button_release_r
};

struct mouse_event {
    enum mouse_event_type type;
    int position_x;
    int position_y;
    int vscroll;
};

void mouse_installEventHandler(void(*mouseEventHandler)(struct mouse_event* event));

#endif
