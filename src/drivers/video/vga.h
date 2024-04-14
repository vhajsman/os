#ifndef __VGA_H
#define __VGA_H

#include "types.h"


#define VGA_ADDRESS         0xB8000
#define VGA_TOTAL_ITEMS     2200

#define VGA_WIDTH           80
#define VGA_HEIGHT          24

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
    s32 x;   // Horizontal
    s32 y;   // Vertical

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

// =========================================================
// ===== VGA CURSOR
// =========================================================

void cursor_set(u8 start, u8 end);

void cursor_enable();
void cursor_disable();

void cursor_update(coords pos);
coords cursor_get();

#endif
