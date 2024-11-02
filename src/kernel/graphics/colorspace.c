#include "colorspace.h"

// rgb = [r, g, b]
void decodeRGB(rgb_t rgb, u8* decoded) {
    if(decoded == NULL)
        return;

    decoded[0] = (rgb >> 16) & 0xFF;
    decoded[1] = (rgb >> 8 ) & 0xFF;
    decoded[2] =  rgb        & 0xFF;
}

// rgba = [r, g, b, a]
void decodeRGBA(rgba_t rgba, u8* decoded) {
    if(decoded == NULL)
        return;
    
    decodeRGB(rgba, decoded);
    decoded[3] = (rgba >> 24) & 0xFF;
}
