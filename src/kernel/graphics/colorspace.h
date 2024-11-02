#ifndef __COLORSPACE_H
#define __COLORSPACE_H

#include "types.h"

typedef u32 rgb_t;
typedef u32 rgba_t;

#define MAKE_RGB(R, G, B)       (((u32)(R) << 16) | ((u32)(G) << 8) | (u32)(B))
#define MAKE_RGBA(R, G, B, A)   (((u32)(A) << 24) | MAKE_RGB(R, G, B))

#define INVERT_RGB(rgb)         (MAKE_RGB(  \
    255 - ((rgb >> 16) & 0xFF),             \
    255 - ((rgb >> 8) & 0xFF),              \
    255 - (rgb & 0xFF)                      \
))

#define RGB_COLOR_BLACK         MAKE_RGB(0, 0, 0)
#define RGB_COLOR_WHITE         MAKE_RGB(255, 255, 255)
#define RGB_COLOR_RED           MAKE_RGB(255, 0,   0  )
#define RGB_COLOR_GREEN         MAKE_RGB(0,   255, 0  )
#define RGB_COLOR_BLUE          MAKE_RGB(0,   0,   255)
#define RGB_COLOR_YELLOW        MAKE_RGB(255, 255, 0  )
#define RGB_COLOR_CYAN          MAKE_RGB(0,   255, 255)
#define RGB_COLOR_MAGENTA       MAKE_RGB(255, 0,   255)
#define RGB_COLOR_GRAY          MAKE_RGB(128, 128, 128)
#define RGB_COLOR_LIGHT_GRAY    MAKE_RGB(211, 211, 211)
#define RGB_COLOR_DARK_GRAY     MAKE_RGB(169, 169, 169)
#define RGB_COLOR_ORANGE        MAKE_RGB(255, 165, 0  )
#define RGB_COLOR_PURPLE        MAKE_RGB(128, 0,   128)
#define RGB_COLOR_BROWN         MAKE_RGB(165, 42,  42 )
#define RGB_COLOR_PINK          MAKE_RGB(255, 192, 203)
#define RGB_COLOR_NAVY          MAKE_RGB(0,   0,   128)
#define RGB_COLOR_TEAL          MAKE_RGB(0,   128, 128)
#define RGB_COLOR_OLIVE         MAKE_RGB(128, 128, 0  )
#define RGB_COLOR_LIGHT_BLUE    MAKE_RGB(173, 216, 230)
#define RGB_COLOR_VIOLET        MAKE_RGB(238, 130, 238)
#define RGB_COLOR_SALMON        MAKE_RGB(250, 128, 114)
#define RGB_COLOR_GOLD          MAKE_RGB(255, 215, 0  )
#define RGB_COLOR_SILVER        MAKE_RGB(192, 192, 192)
#define RGB_COLOR_LIME          MAKE_RGB(0  , 255, 0  )

void decodeRGB(rgb_t rgb, u8* decoded);     // rgb =  [r, g, b]
void decodeRGBA(rgba_t rgba, u8* decoded);  // rgba = [r, g, b, a]


#endif
