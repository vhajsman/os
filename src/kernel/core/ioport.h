#ifndef __IOPORT_H
#define __IOPORT_H

#include "types.h"

u8      inportb(u16 port);
void    outportb(u16 port, u8 val);
u16     inports(u16 port);
void    outports(u16 port, u16 data);
u32     inportl(u16 port);
void    outportl(u16 port, u32 data);

u32 inportsl(u16 port);
void insl(u16 port, unsigned int *addr, u32 count);

#endif