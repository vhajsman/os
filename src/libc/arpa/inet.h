#ifndef __LIBC_ARPA_INET_H
#define __LIBC_ARPA_INET_H 1

#include <stdint.h>

u16 htons(u16 host_short);
u16 ntohs(u16 net_short);
u32 htonl(u32 host_long);
u32 ntohl(u32 net_long);

#endif
