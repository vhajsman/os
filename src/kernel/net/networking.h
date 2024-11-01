#ifndef __NETWORKING_H
#define __NETWORKING_H

#include "kernel.h"

#define NET_PACKET_SIZE_MAX 40860

typedef struct {
    u8 data[NET_PACKET_SIZE_MAX];
    u32 length;
} tx_packet_t;

u16 ntohs(u16 netshort);

void packet_init();

#endif
