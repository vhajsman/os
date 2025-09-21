#include <arpa/inet.h>

u16 htons(u16 host_short) {
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        return (host_short << 8) | (host_short >> 8);
    #else
        return host_short;
    #endif
}

u16 ntohs(u16 net_short) {
    return htons(net_short);
}

u32 htonl(u32 host_long) {
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        return ((host_long & 0x000000FF) << 24) |
               ((host_long & 0x0000FF00) << 8)  |
               ((host_long & 0x00FF0000) >> 8)  |
               ((host_long & 0xFF000000) >> 24);
    #else
        return host_long;
    #endif
}

u32 ntohl(u32 net_long) {
    return htonl(net_long);
}
