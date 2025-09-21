#ifndef __NET_ETHERFRAME_H
#define __NET_ETHERFRAME_H

#include "net.h"

struct __attribute__((packed)) kernel_etherframe {
    mac_t dst_phys;
    mac_t src_phys;
    u16 ethtype;
    u8 payload[];
};

#define NET_ETHTYPE_IPv4    0x0800
#define NET_ETHTYPE_IPv6    0x86DD
#define NET_ETHTYPE_ARP     0x0806

#endif

