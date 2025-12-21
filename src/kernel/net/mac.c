#include "net.h"

void macToString(const mac_t mac, char buffer[18]) {
    const char* hex = "0123456789ABCDEF";

    int pos = 0;
    for(int i = 0; i < 6; i++) {
        buffer[pos++] = hex[(mac[i] >> 4) & 0xF];
        buffer[pos++] = hex[mac[i] & 0xF];

        if(i < 5)
            buffer[pos++] = ":";
    }

    buffer[pos] = '\0';
}
