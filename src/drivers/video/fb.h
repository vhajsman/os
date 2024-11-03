#ifndef __FB_H
#define __FB_H

#include "types.h"
#include "graphics/colorspace.h"

typedef struct /*framebuffer_header*/ {
    u32* addr;

    u32 width;
    u32 height;
    u32 pitch;

    u8 bytes_per_pixel;

    size_t size;
    size_t length;
} fb_t;

#ifdef __framebuffer_header__needed_explicite
extern fb_t framebuffer_header;
#endif

void framebuffer_init();

// Construct framebuffer header based of information from multiboot protocol
void framebuffer_constructHeader(fb_t* header);

void framebuffer_plot(int x, int y, rgb_t rgb);
void framebuffer_fillScreen(rgb_t rgb);
void framebuffer_clear();

int framebuffer_snapshot(u32** output, size_t* size);

#endif
