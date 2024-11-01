#ifndef __IP_H
#define __IP_H

#include "types.h"

typedef struct IPAddress {
    u8 addr[4];
} ipaddr_t;

void ipaddr_constructor(ipaddr_t* ip, u8 a, u8 b, u8 c, u8 d);
const char* ipaddr_str(ipaddr_t* ip);

typedef struct {
    u32 src_ip;          // Source IP address
    u32 dst_ip;          // Destination IP address
    u8  protocol;        // Protocol type (e.g., UDP, TCP)
    u8  ttl;             // Time to Live
    u8  data[];          // Data (variable length)
} ip_packet_t;

#endif
