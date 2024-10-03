#ifndef __INITRD_H
#define __INITRD_H

#include "types.h"
#include "multiboot.h"

void initrd_load(MULTIBOOT_INFO* mb_info);

extern u32 initrd_start;
extern u32 initrd_end;
extern u32 initrd_size;
extern const char* initrd_data;

int initrd_read(const char* filename, char* buffer, size_t buffer_size);

#endif
