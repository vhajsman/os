#include "networking.h"

u16 ntohs(u16 netshort) {
    return (netshort << 8) | (netshort >> 8);
}