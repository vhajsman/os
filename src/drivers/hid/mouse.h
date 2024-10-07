#ifndef __MOUSE_H
#define __MOUSE_H

#include "ioport.h"

void mouse_write(u8 command);
u8 mouse_read();
void mouse_init();

#endif
