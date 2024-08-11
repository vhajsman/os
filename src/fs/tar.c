#include "tar.h"
#include "types.h"
#include "string.h"
#include "debug.h"

size_t _oct2dec(const char* str, size_t size) {
    size_t result = 0;

    while (size--)
        result = result * 8 + (*str++ - '0');

    return result;
}

// const char* tar_findf(const char* tar_data, const char* filename) {
//     const char *ptr = tar_data;
// 
//     while (*ptr) {
//         tar_header_t* header = (tar_header_t*) ptr;
// 
//         if(header->name[0] == '\0')
//             return NULL;
// 
//         size_t size = _oct2dec(header->size, sizeof(header->size));
// 
//         if (strcmp(header->name, filename) == 0) {
//             return ptr + 512;
//         }
// 
//         size_t file_blocks = (size + 511) / 512;
//         ptr += 512 + file_blocks * 512;
//     }
// 
//     return NULL;
// }

size_t tar_readf(const char* tar_data, const char* filename, char* buffer, size_t max_size) {
    const char* ptr = tar_data;
    const tar_header_t* header;
    
    while (*ptr) {
        header = (const tar_header_t*) ptr;
        size_t fsize = _oct2dec(header->size, sizeof(header->size));
        
        if (strcmp(header->name, filename) == 0) {
            if (fsize <= max_size) {
                memcpy(buffer, ptr + sizeof(tar_header_t), fsize);
                buffer[fsize] = '\0';

                return fsize;
            } else {
                debug_message("tar_readf(): insufficent buffer size", "tar", KERNEL_ERROR);
                return 0;
            }
        }
        
        size_t file_blocks = (fsize + 511) / 512;
        ptr += 512 + file_blocks * 512;
    }
    
    debug_message("tar_readf(): file not found", "tar", KERNEL_ERROR);
    return 0;
}


void tar_list(const char* tar_data) {
    debug_message("Tar directory listing for '/': ", "tar", KERNEL_MESSAGE);

    const tar_header_t* header;
    const char* current = tar_data;

    while (1) {
        header = (const tar_header_t*) current;

        if (header->name[0] == '\0')
            return;

        debug_append("\n                  ");
        debug_append(header->name);

        size_t file_size = _oct2dec(header->size, sizeof(header->size));
        size_t file_blocks = (file_size + 512 - 1) / 512;
        current += 512 + file_blocks * 512;
    }
}
