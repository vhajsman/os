#ifndef __VGA_H
#define __VGA_H

#include "types.h"
#include "linkedlist.h"

#define VGA_ADDRESS         0xB8000
#define VGA_GFXCTRL_ADDRESS 0xA0000
#define VGA_TOTAL_ITEMS     2200

#define VGA_WIDTH           80
#define VGA_HEIGHT          24

#define VGA_GLYPH_HEIGHT    16
#define VGA_GLYPH_WIDTH     8

// #define VGA_SEQ_INDEX       0x3C4
// #define VGA_SEQ_DATA        0x3C5
// #define VGA_GC_INDEX        0x3CE
// #define VGA_GC_DATA         0x3CF


extern u16 vga_cw; // VGA Character-meassured width
extern u16 vga_ch; // VGA Character-meassured height
extern u16 vga_pw; // VGA Pixel-meassured width
extern u16 vga_ph; // VGA Pixel-meassured height

extern u16 vga_glyph_width;
extern u16 vga_glyph_height;

void vga_init();


// | Register name	                port	index	mode 3h (80x25 text mode)	mode 12h (640x480 planar 16 color mode)	mode 13h (320x200 linear 256-color mode)	mode X (320x240 planar 256 color mode)
// +---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// | Mode Control                   0x3C0   0x10    0x0C                        0x01                                    0x41                                        0x41
// | Overscan Register              0x3C0   0x11    0x00                        0x00                                    0x00                                        0x00
// | Color Plane Enable             0x3C0   0x12    0x0F                        0x0F                                    0x0F                                        0x0F
// | Horizontal Panning             0x3C0   0x13    0x08                        0x00                                    0x00                                        0x00
// | Color Select                   0x3C0   0x14    0x00                        0x00                                    0x00                                        0x00
// | Miscellaneous Output Register  0x3C2   N/A	    0x67                        0xE3                                    0x63                                        0xE3
// | Clock Mode Register            0x3C4   0x01    0x00                        0x01                                    0x01                                        0x01
// | Character select               0x3C4   0x03    0x00                        0x00                                    0x00                                        0x00
// | Memory Mode Register           0x3C4   0x04    0x07                        0x02                                    0x0E                                        0x06
// | Mode Register                  0x3CE   0x05    0x10                        0x00                                    0x40                                        0x40
// | Miscellaneous Register         0x3CE   0x06    0x0E                        0x05                                    0x05                                        0x05
// | Horizontal Total               0x3D4   0x00    0x5F                        0x5F                                    0x5F                                        0x5F
// | Horizontal Display Enable End  0x3D4   0x01    0x4F                        0x4F                                    0x4F                                        0x4F
// | Horizontal Blank Start         0x3D4   0x02    0x50                        0x50                                    0x50                                        0x50
// | Horizontal Blank End           0x3D4   0x03    0x82                        0x82                                    0x82                                        0x82
// | Horizontal Retrace Start       0x3D4   0x04    0x55                        0x54                                    0x54                                        0x54
// | Horizontal Retrace End         0x3D4   0x05    0x81                        0x80                                    0x80                                        0x80
// | Vertical Total                 0x3D4   0x06    0xBF                        0x0B                                    0xBF                                        0x0D
// | Overflow Register              0x3D4   0x07    0x1F                        0x3E                                    0x1F                                        0x3E
// | Preset row scan                0x3D4   0x08    0x00                        0x00                                    0x00                                        0x00
// | Maximum Scan Line              0x3D4   0x09    0x4F                        0x40                                    0x41                                        0x41
// | Vertical Retrace Start         0x3D4   0x10    0x9C                        0xEA                                    0x9C                                        0xEA
// | Vertical Retrace End           0x3D4   0x11    0x8E                        0x8C                                    0x8E                                        0xAC
// | Vertical Display Enable End    0x3D4   0x12    0x8F                        0xDF                                    0x8F                                        0xDF
// | Logical Width                  0x3D4   0x13    0x28                        0x28                                    0x28                                        0x28
// | Underline Location             0x3D4   0x14    0x1F                        0x00                                    0x40                                        0x00
// | Vertical Blank Start           0x3D4   0x15    0x96                        0xE7                                    0x96                                        0xE7
// | Vertical Blank End             0x3D4   0x16    0xB9                        0x04                                    0xB9                                        0x06
// | Mode Control                   0x3D4   0x17    0xA3                        0xE3                                    0xA3                                        0xE3

////// #define VGA_REGISTER_INDEX      0x3C0
////// #define VGA_REGISTER_DATA       0x3C0
////// #define VGA_REGISTER_MISC_OUT_W 0x3C2
////// #define VGA_REGISTER_MISC_OUT_R 0x3C3
////// #define VGA_REGISTER_IDX0       0x3C4
////// #define VGA_REGISTER_IDX1       0x3CE
////// #define VGA_REGISETR_IDX2       0x3D4
////// #define VGA_REGISETR_DAC_MASK   0x3C6
////// #define VGA_REGISTER_DAC_CTRL0  0x3C7
////// #define VGA_REGISTER_DAC_CTRL1  0x3C8
////// #define VGA_REGISTER_DAC_CTRL2  0x3C9

// Taken from: http://files.osdev.org/mirrors/geezer/osd/graphics/modes.c
#define	VGA_AC_INDEX        0x3C0
#define	VGA_AC_WRITE        0x3C0
#define	VGA_AC_READ         0x3C1
#define	VGA_MISC_WRITE      0x3C2
#define VGA_SEQ_INDEX       0x3C4
#define VGA_SEQ_DATA        0x3C5
#define	VGA_DAC_READ_INDEX  0x3C7
#define	VGA_DAC_WRITE_INDEX 0x3C8
#define	VGA_DAC_DATA        0x3C9
#define	VGA_MISC_READ       0x3CC
#define VGA_GC_INDEX        0x3CE
#define VGA_GC_DATA         0x3CF
#define VGA_CRTC_INDEX      0x3D4
#define VGA_CRTC_DATA       0x3D5
#define	VGA_INSTAT_READ     0x3DA
#define	VGA_NUM_SEQ_REGS    5
#define	VGA_NUM_CRTC_REGS   25
#define	VGA_NUM_GC_REGS     9
#define	VGA_NUM_AC_REGS     21
#define	VGA_NUM_REGS        (1 + VGA_NUM_SEQ_REGS + VGA_NUM_CRTC_REGS + \
                                 VGA_NUM_GC_REGS  + VGA_NUM_AC_REGS)

// Assummed that X is horizontal axis
#define COORDS2D(X, Y)      \
    struct xy2d {           \
        .x = X;             \
        .y = Y;             \
                            \
        .u = (X * Y) + Y;   \
                            \
        .o = 2 * (X + Y);   \
        .S = X * Y;         \
    }

// Assummed that X is horizontal axis
#define DIMS2D(X, Y) COORDS2D(X, Y)

struct xy2d {
    u8 x;   // Horizontal
    u8 y;   // Vertical

    s32 o;  // Border
    s32 S;  // Surface
    
    union {
        s32 u;      // Diagonal
        s32 index;  // Index in a grid
    };
    
};

typedef struct xy2d coords;
typedef struct xy2d dimensions;

enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

/*  Specifies VGA entry color. And blinking effect possibly.

    === OUTPUT ==================
    Type: u8
    Outputs VGA entry color                                                                                                                                                           
    
    +-------+-------+-------+-------+-------+-------+-------+-------+
    | 7     | 6     | 5     | 4     | 3     | 2     | 1     | 0     |
    +-------+-------+-------+-------+-------+-------+-------+-------+
    | Blink | Background            | Foreground                    |
    +-------+-----------------------+-------------------------------+
*/
u8 vga_entryColor(enum vga_color fg, enum vga_color bg);
u16 vga_entry(unsigned char uc, u8 color);

// ===========================================================================================================================================================================
// ===== VGA CURSOR
// ===========================================================================================================================================================================

extern void cursor_update(const coords* pos);
extern void cursor_disable();
extern void cursor_enable(u8 cursor_start, u8 cursor_end);
extern void cursor_locate(coords* pos);

// ===========================================================================================================================================================================
// ===== GRAPHICS CONTROLER (gfxctrl)
// ===========================================================================================================================================================================
//
// http://www.osdever.net/FreeVGA/vga/graphreg.htm   ---   Hardware Level VGA and SVGA Video Programming Information Page - Graphics Registers 
//
// | Register Name              Port	Index	7	6	5	4	3	2	1	0
// +------------------------------------------------------------------------------------------
// | Graphics Mode Register     0x3CE   0x05                    R.M     W.M /////////////////   R.M = Read Mode, W.R = Write mode
// | Map Mask Register          0x3C4   0x02                    Memory Plane Write Enable ///
// | Enable Set/Reset Register  0x3CE   0x01                    Enable Set/Reset ////////////
// | Set/Reset Register         0x3CE   0x00                    Set/Reset Value /////////////
// | Data Rotate Register       0x3CE   0x03                L.O ///	Rotate Count ////////////   L.O = Logical Operation
// | Bit Mask Register          0x3CE   0x08    Bit Mask ////////////////////////////////////

#define VGA_GFXCTRL_REG_GRAPHICS_MODE   0x3ce   // READ_MODE [3]; WRITE_MODE [1..0]
#define VGA_GFXCTRL_REG_MAP_MASK        0x3c4   // MEMORY_PLANE_WRITE_ENABLE [3..0]
#define VGA_GFXCTRL_REG_SET_RESET_EN    0x3ce   // ENABLE_SET_RESET [3..0]
#define VGA_GFXCTRL_REG_SET_RESET       0x3ce   // SET_RESET_VALUE [3..0]
#define VGA_GFXCTRL_REG_DATA_ROTATE     0x3ce   // LOGICAL_OPERATION [4]; ROTATE_COUNT [3..0]
#define VGA_GFXCTRL_REG_BIT_MASK        0x3ce   // BIT_MASK [7..0]
#define VGA_GFXCTRL_IDX_GRAPHICS_MODE   0x005
#define VGA_GFXCTRL_IDX_MAP_MASK        0x002
#define VGA_GFXCTRL_IDX_SET_RESET_EN    0x001
#define VGA_GFXCTRL_IDX_SET_RESET       0x000
#define VGA_GFXCTRL_IDX_DATA_ROTATE     0x003
#define VGA_GFXCTRL_IDX_BIT_MASK        0x008

// struct vga_mode {
//     u16 width;
//     u16 height;
// 
//     u16 glyph_width;
//     u16 glyph_height;
// 
//     volatile u8* vmem;
// };


// ===========================================================================================================================================================================
// ===== CHARACTER SETS, FONTS
// ===========================================================================================================================================================================
//

#define VGA_CHARSET_LENGTH              256

typedef struct {
    u8 glpyhs[VGA_CHARSET_LENGTH][VGA_GLYPH_HEIGHT];
} vga_charset_t;

#ifdef __vga_charset_list__explicite_request
    extern linkedlist_t* vga_charset_list;
#endif

int vga_charset_import(vga_charset_t* charset, unsigned int idx);
void vga_charset_export(vga_charset_t* charset, unsigned int idx);

void vga_charset_write(unsigned int charset_idx);
void vga_charset_read(vga_charset_t* charset);

unsigned int vga_charset_what();


// ===========================================================================================================================================================================
// ===== GRAPHICS
// ===========================================================================================================================================================================
//

#define VGA_GFXMEM          0xA0000

#define VGA_PALLETE_IDX     0x3C8
#define VGA_PALLETE_COLOR   0x3C9


#endif
