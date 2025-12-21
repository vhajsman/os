#include "fmt.h"

void fmt_u32_blocks(u32 value, char out[16], char fmt[2]) {
    char zero = fmt[0]; char sep = fmt[1];
    int blocks[4];

    blocks[3] = value % 10;     value /= 10;
    blocks[2] = value % 1000;   value /= 1000;
    blocks[1] = value % 1000;   value /= 1000;
    blocks[0] = value % 1000;   value /= 1000;

    int pos = 0;
    for(int i = 0; i < 4; i++) {
        if(i < 3) {
            out[pos++] = zero + (blocks[i] / 100 % 10);
            out[pos++] = zero + (blocks[i] / 10 % 10);
            out[pos++] = zero + (blocks[i] % 10);
        } else {
            out[pos++] = zero + blocks[i];
        }

        if(i < 3)
            out[pos++] = sep;
    }

    out[pos] = '\0';
}
