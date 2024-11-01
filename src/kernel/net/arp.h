#ifndef __ARP_H
#define __ARP_H

#include "types.h"

#define ARP_OPCODE_REQUEST  1
#define ARP_OPCODE_REPLY    2

typedef struct {
    u8 dst_mac[6];      // Target MAC address
    u8 src_mac[6];      // Source MAC address
    u16 ether_type;     // EtherFrame type
    u16 htype;          // Hardware type
    u16 ptype;          // Protocol type
    u8 hlen;            // Length of hardware address
    u8 plen;            // Length of protocol address
    u16 opcode;         // Opcode (request or reply)
    u8 sender_mac[6];   // Sender MAC
    u32 sender_ip;      // Sender IP
    u8 target_mac[6];   // Target MAC
    u32 target_ip;      // Target IP
} arp_packet_t;

#endif
