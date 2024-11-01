#ifndef __IP_H
#define __IP_H

#include "types.h"

typedef struct IPAddress {
    u8 addr[4];
} ipaddr_t;

void ipaddr_constructor(ipaddr_t* ip, u8 a, u8 b, u8 c, u8 d);
const char* ipaddr_str(ipaddr_t* ip);

#endif
