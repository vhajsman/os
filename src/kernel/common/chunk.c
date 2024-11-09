#include "chunk.h"

void chunk_findchild(int chunk_size, int idx, int* chunk, int* chunk_idx) {
    *chunk      = idx / chunk_size;
    *chunk_idx  = idx % chunk_size;
}