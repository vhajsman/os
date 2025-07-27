#include "ioport.h"

u8 inportb(u16 port) {
    u8 ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));

    return ret;
}

void outportb(u16 port, u8 val) {
    asm volatile("outb %1, %0" :: "dN"(port), "a"(val));
}

u16 inports(u16 port) {
    u16 rv;
    asm volatile ("inw %1, %0" : "=a" (rv) : "dN" (port));

    return rv;
}

void outports(u16 port, u16 data) {
    asm volatile ("outw %1, %0" : : "dN" (port), "a" (data));
}

u16 inportw(u16 port) {
    u16 rv;
    asm volatile ("inw %1, %0" : "=a" (rv) : "dN" (port));

    return rv;
}

void outportw(u16 port, u16 data) {
    asm volatile ("outw %1, %0" : : "dN" (port), "a" (data));
}

u32 inportl(u16 port) {
    u32 rv;
    asm volatile ("inl %%dx, %%eax" : "=a" (rv) : "dN" (port));

    return rv;
}

void outportl(u16 port, u32 data) {
    asm volatile ("outl %%eax, %%dx" : : "dN" (port), "a" (data));
}

u32 inportsl(u16 port) {
    u32 result;
    __asm__ __volatile__ ("inl %1, %0" : "=a"(result) : "dN"(port));
    return result;
}

void insl(u16 port, unsigned int *addr, u32 count) {
    __asm__ __volatile__ (
        "cld; rep insl"
        : "+D"(addr), "+c"(count)
        : "d"(port)
        : "memory"
    );
}
