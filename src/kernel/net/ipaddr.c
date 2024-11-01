#include "ipaddr.h"
#include "string.h"
#include "memory/memory.h"

void ipaddr_constructor(ipaddr_t* ip, u8 a, u8 b, u8 c, u8 d) {
    if(ip == NULL)
        return;
        
    ip->addr[0] = a;
    ip->addr[1] = b;
    ip->addr[2] = c;
    ip->addr[3] = d;
}

const char* ipaddr_str(ipaddr_t* ip) {
    if(ip == NULL)
        return NULL;
    
    char* buff = (char*) malloc(16 * sizeof(char));
    if(buff == NULL)
        return NULL;

    char temp[4];
    int pos = 0;

    for(int i = 0; i < 4; i++) {
        itoa(buff[i], 10, temp);
        int len = strlen(temp);

        memcpy(buff + pos, temp, len);
        pos += len;

        if(i < 3)
            buff[pos++] = '.';
    }

    buff[pos] = '\0';
    return buff;
}