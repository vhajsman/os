#ifndef __UDP_H
#define __UDP_H

#include "types.h"

typedef struct {
    u16 src_port;        // Source port
    u16 dst_port;        // Destination port
    u16 length;          // Length of UDP segment
    u8 data[];           // Data (variable length)
} udp_packet_t;

#endif
