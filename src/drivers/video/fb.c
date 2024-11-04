#define __mboot_info__needed_explicite 1

#include "fb.h"
#include "multiboot.h"
#include "graphics/geometry_2d.h"
#include "memory/memory.h"

#undef __mboot_info_needed_explicite

fb_t framebuffer_header;

void framebuffer_constructHeader(fb_t* header) {
    if(mboot_info == NULL)
        return;
    
    header->addr = (u32*)     mboot_info->framebuffer_addr;
    header->width =           mboot_info->framebuffer_width;
    header->height =          mboot_info->framebuffer_height;
    header->bytes_per_pixel = mboot_info->framebuffer_bpp / 8; // idk why, but bpp from multiboot is somehow incremented by 1
    header->pitch =           mboot_info->framebuffer_pitch == 0 ? header->width * header->bytes_per_pixel : mboot_info->framebuffer_pitch;

    header->length = header->width * header->height;
    header->size = header->length * header->bytes_per_pixel;
}

void framebuffer_init() {
    // Construct framebuffer_header based of information from multiboot protocol
    framebuffer_constructHeader(&framebuffer_header);

    framebuffer_clear();
}

#define _VMEM_RESOLVEADDR(ADDR) \
    ((u32*)((u8*)framebuffer_header.addr + (ADDR) * framebuffer_header.bytes_per_pixel))
#define _VMEM_WRITE(ADDR, VAL)  \
    *(_VMEM_RESOLVEADDR(ADDR)) = (u32)(VAL)
#define _VMEM_READ(ADDR)        \
    (_VMEM_RESOLVEADDR(ADDR))

void framebuffer_plot(int x, int y, rgb_t rgb) {
    if(x < 0 || y < 0 || x >= (int) framebuffer_header.width || y >= (int) framebuffer_header.height)
        return;
    
    _VMEM_WRITE(Calculate2DFieldIndex(x, y, framebuffer_header.width), (rgb & 0xFFFFFF) | (0 << 24));
}

void framebuffer_fillScreen(rgb_t rgb) {
    // for(int i = 0; i < (int) (framebuffer_header.width * framebuffer_header.height); i++)
    //     _VMEM_WRITE(i, rgb);

    for(int y = 0; y < framebuffer_header.height; y++)
        for(int x = 0; x < framebuffer_header.width; x++)
            //_VMEM_WRITE(Calculate2DFieldIndex(x, y, framebuffer_header.width), rgb);
            framebuffer_plot(x, y, rgb);
}

void framebuffer_clear() {
    framebuffer_fillScreen(MAKE_RGB(0xFF, 0xFF, 0xFF));
}

//int framebuffer_snapshot(u32** output, size_t* size) {
//    size_t fbsize = framebuffer_header.size;
//    u32* copiedbuffer = (u32*) malloc(fbsize);
//
//    if(copiedbuffer == NULL) {
//        *output = NULL;
//
//        if(size != NULL) 
//            *size = 0;
//
//        return -1;
//    }
//
//    for(size_t i = 0; i < framebuffer_header.length; i++)
//        copiedbuffer[i] = _VMEM_READ(i);
//
//    *output = copiedbuffer;
//
//    if(size != NULL) 
//        *size = fbsize;
//
//    return 0;
//}
