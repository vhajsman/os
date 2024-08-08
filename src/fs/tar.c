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

const char* tar_findf(const char* tar_data, const char* filename) {
    const char *ptr = tar_data;

    while (*ptr) {
        tar_header_t* header = (tar_header_t*) ptr;
        size_t size = _oct2dec(header->size, sizeof(header->size));

        if (strcmp(header->name, filename) == 0) {
            return ptr + 512;
        }

        ptr += ((size + 511) / 512) * 512 + 512;
    }

    return NULL;
}

size_t tar_readf(const char* tar_data, const char* filename, char* buffer, size_t max_size) {
    const char *file_data = tar_findf(tar_data, filename);

    if (file_data == NULL) {
        debug_message("tar_readf(): File not found: ", "tar", KERNEL_ERROR);
        debug_append(filename);

        return 0;
    }

    tar_header_t* header = (tar_header_t*) (file_data - 512);
    size_t file_size = _oct2dec(header->size, sizeof(header->size));

    size_t to_read = (file_size < max_size) ? file_size : max_size;
    memcpy(buffer, file_data, to_read);

    return to_read;
}
