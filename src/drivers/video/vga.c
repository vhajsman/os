#include "video/vga.h"
#include "ioport.h"
#include "types.h"
#include "memory/memory.h"
#include "linkedlist.h"

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

void vga_charset_load(u8* charset, u16 bank) {
    vga_writeReg(VGA_SEQ_INDEX, 0x02, 0x04);
    vga_writeReg(VGA_SEQ_INDEX, 0x04, 0x07);
    vga_writeReg(VGA_GC_INDEX,  0x04, 0x02);
    vga_writeReg(VGA_GC_INDEX,  0x05, 0x00);

    u8* fontmem = (u8*) VGA_GFXCTRL_ADDRESS + bank * 4096;

    for(int i = 0; i < VGA_CHARSET_LENGTH * 16; i++) 
        fontmem[i] = charset[i];
    
    vga_writeReg(VGA_SEQ_INDEX, 0x02, 0x03);
    vga_writeReg(VGA_SEQ_INDEX, 0x04, 0x03);
    vga_writeReg(VGA_GC_INDEX,  0x04, 0x00);
}

void vga_charset_read(u8* charset, u16 bank) {
    vga_writeReg(VGA_SEQ_INDEX, 0x04, 0x02);
    vga_writeReg(VGA_GC_INDEX,  0x05, 0x00);

    u8* fontmem = (u8*) VGA_GFXCTRL_ADDRESS + bank * 4096;

    for(int i = 0; i < VGA_CHARSET_LENGTH * 16; i++)
        charset[i] = fontmem[i];
}

vga_charset_bank* vga_charset_createBank() {
    vga_charset_bank* nbank = (vga_charset_bank*) malloc(sizeof(vga_charset_bank));
    if(nbank == NULL)
        return NULL;
    
    nbank->data = (u8*) malloc(VGA_CHARSET_BANK_SIZE);
    if(nbank->data == NULL) {
        free(nbank);
        return NULL;
    }

    return nbank;
}

void vga_charset_import(vga_charset_bank* bank, u8* charset) {
    if(bank == NULL || bank->data == NULL)
        return;
    
    for(int i = 0; i < VGA_CHARSET_BANK_SIZE; i++)
        bank->data[i] = charset[i];
}

void vga_charset_addBankToList(linkedlist_t* list, vga_charset_bank* bank) {
    linkedlist_push(list, (void*) bank);
}

void vga_charset_removeBankFromList(linkedlist_t* list, vga_charset_bank* bank) {
    linkedlist_removeNode(list, (linkedlist_node_t*) bank);
}

void vga_charset_loadFromBank(vga_charset_bank* bank) {

}

void vga_charset_selectBank(u8 bank) {
    vga_writeReg(VGA_SEQ_INDEX, 0x03, bank & 0x03);
}
