#ifndef __NET_H
#define __NET_H

#include "kernel.h"

typedef u8 mac_t[6];
void macToString(const mac_t mac, char buffer[18]);

#endif

