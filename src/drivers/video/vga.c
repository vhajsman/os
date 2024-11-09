#include "video/vga.h"
#include "ioport.h"
#include "types.h"
#include "memory/memory.h"
#include "linkedlist.h"
#include "string.h"

u8 vga_entryColor(enum vga_color fg, enum vga_color bg) {
	return fg | bg << 4;
}
 
u16 vga_entry(unsigned char uc, u8 color) {
	return (u16) uc | (u16) color << 8;
}

u16 vga_cw; // VGA Character-meassured width
u16 vga_ch; // VGA Character-meassured height
u16 vga_pw; // VGA Pixel-meassured width
u16 vga_ph; // VGA Pixel-meassured height

u16 vga_glyph_width;
u16 vga_glyph_height;

volatile u8* vga_vmem;

void vga_setVgaMode(    u16 width, u16 height, u16 glyph_width, u16 glyph_height, u32 vmem_addr) {
//    mode->width             = width;
//    mode->height            = height;
//
//    mode->vga_glyph_width   = vga_glyph_width;
//    mode->vga_glyph_height  = vga_glyph_height;
//
//    mode->vmem = (volatile u8*) vmem_addr;

    vga_pw = width;
    vga_ph = height;
    vga_glyph_width  = glyph_width;
    vga_glyph_height = glyph_height;

    vga_cw = vga_pw / vga_glyph_width;
    vga_ch = vga_ph / vga_glyph_height;

    vga_vmem = (volatile u8*) vmem_addr;
}

void vga_setVideoMode(u8 mode) {
    asm volatile ("int $0x10" : : "a"(0x00 | mode));
}

void vga_setTextMode() {
    vga_setVideoMode(0x03);
}

void vga_plot(unsigned int x, unsigned int y, u8 color) {
    if(x > vga_pw || y > vga_ph) // OVERFLOW
        return;

    vga_vmem[y * vga_pw + x] = color;
}

void vga_fill(u8 color) {
    for(unsigned int iy = 0; iy < vga_ph; iy++) {
        for(unsigned int ix = 0; ix < vga_pw; ix++) {
            vga_plot(ix, iy, color);
        }
    }
}


// ===========================================================================================================================================================================
// ===== LOW-LEVEL IO
// ===========================================================================================================================================================================
//

void vga_writeReg(u16 port, u8 idx, u8 value) {
    outportb(port, idx);
    outportb(port + 1, value);
}

// ===========================================================================================================================================================================
// ===== CHARACTER SETS, FONTS
// ===========================================================================================================================================================================
//

linkedlist_t* vga_charset_list;

int vga_charset_import(vga_charset_t* charset, unsigned int idx) {
    if(charset == NULL)
        return -1;
    
    if(idx < 0) {
        linkedlist_insertFront(vga_charset_list, (vga_charset_t*) charset);
        return linkedlist_size(vga_charset_list) - 1;
    }

    linkedlist_node_t* node = linkedlist_getNodeByIndex(vga_charset_list, idx);
    if(node == NULL /*|| (node->next == NULL && node->prev == NULL)*/)
        return -2;

    node->val = (vga_charset_t*) charset;
    return 0;
}

void vga_charset_export(vga_charset_t* charset, unsigned int idx) {
    if(charset == NULL)
        return;
    
    const linkedlist_node_t* node = linkedlist_getNodeByIndex(vga_charset_list, idx);
    if(linkedlist_size(vga_charset_list) < idx || node->val == NULL) {
        charset = NULL;
        return;
    }

    memcpy((vga_charset_t*) charset, node->val, sizeof(vga_charset_t));
}

void vga_charset_write(unsigned int charset_idx) {
    vga_charset_t* cs = (vga_charset_t*) malloc(sizeof(vga_charset_t));
    if(cs == NULL)
        return;
    
    vga_charset_export(cs, charset_idx);
    if(cs == NULL) {
        free(cs);
        return;
    }
    
    u8* fontmem = (u8*) VGA_GFXCTRL_ADDRESS;
    memcpy(fontmem, cs->glpyhs, sizeof(cs->glpyhs));

    free(cs);
}

void vga_charset_read(vga_charset_t* charset) {
    if(charset == NULL)
        return;
    
    u8* fontmem = (u8*) VGA_GFXCTRL_ADDRESS;
    memcpy(charset->glpyhs, fontmem, sizeof(charset->glpyhs));
}

// ===========================================================================================================================================================================
// ===== INIT
// ===========================================================================================================================================================================
//

void vga_init() {
    vga_charset_list = linkedlist_create();
    
    vga_charset_t* cs = (vga_charset_t*) malloc(sizeof(vga_charset_t));
    if(cs != NULL) {
        vga_charset_read(cs);
        vga_charset_import(cs, 0);
    }

    linkedlist_debug(vga_charset_list);
}
