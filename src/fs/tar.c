#include "tar.h"
#include "types.h"
#include "string.h"
#include "debug.h"
#include "memory/memory.h"
#include "console.h"

size_t _oct2dec(const char* str, size_t size) {
    size_t result = 0;

    while (size--)
        result = result * 8 + (*str++ - '0');

    return result;
}

#define tar_getsize(str) _oct2dec(str, 12)

size_t tar_readf(const char* tar_data, const char* filename, char* buffer, size_t max_size) {
    const char* ptr = tar_data;
    const tar_header_t* header;
    
    while (*ptr) {
        header = (const tar_header_t*) ptr;
        size_t fsize = _oct2dec(header->size, sizeof(header->size));
        
        if (strcmp(header->name, (char*) filename) == 0) {
            if (fsize <= max_size) {
                memcpy(buffer, ptr + sizeof(tar_header_t), fsize);
                buffer[fsize] = '\0';

                return fsize;
            } else {
                debug_message("tar_readf(): insufficent buffer size: allocated", "tar", KERNEL_ERROR);
                debug_number(max_size, 10);
                debug_append(", needed ");
                debug_number(fsize, 10);

                return 0;
            }
        }
        
        size_t file_blocks = (fsize + 511) / 512;
        ptr += 512 + file_blocks * 512;
    }
    
    debug_message("tar_readf(): file not found: ", "tar", KERNEL_ERROR);
    debug_append(filename);
    
    return 0;
}


void tar_list(const char* tar_data) {
    debug_message("Tar directory listing for '/': ", "tar", KERNEL_MESSAGE);

    const tar_header_t* header;
    const char* current = tar_data;

    while(header != NULL && header->name[0] != '\n') {
        header = (const tar_header_t*) current;

        // if(header->name[0] == '\0')
        //     return;

        debug_append("\n                  ");
        debug_append(header->name);

        size_t file_size = _oct2dec(header->size, sizeof(header->size));
        size_t file_blocks = (file_size + 512 - 1) / 512;
        current += 512 + file_blocks * 512;
    }
}

struct fs_dirent* tar_readdir(fs_node_t* node, u32 index) {
    const char* data = (const char*) node->impl;
    tar_header_t* header = (tar_header_t*) data;

    u32 i = 0;
    while(header->name[0] != '\0') {
        debug_message("tar_readdir(): Index: ", "tar", KERNEL_MESSAGE);
        debug_number(i, 10);
        debug_message(" tar_readdir(): File name: ", "tar", KERNEL_MESSAGE);
        debug_append(header->name);

        if(i == index) {
            fs_dirent_t* dirent = (fs_dirent_t*) malloc(sizeof(fs_dirent_t));
            strncpy(dirent->name, header->name, 128);

            dirent->inode = i;
            return dirent;
        }

        u32 size = tar_getsize(header->size);
        header = (tar_header_t*) ((u8*) header + ((size + 511) & -511) + 512);

        i++;
    }

    return NULL;
}

fs_node_t* tar_finddir(fs_node_t* node, char* name) {
    if(node->impl == 0) {
        debug_message("tar_finddir(): node->impl is NULL.", "tar", KERNEL_ERROR);
        return NULL;
    }
    
    const char* data = (const char*) node->impl;
    tar_header_t* header = (tar_header_t*) data;

    debug_message("tar_finddir(): Searching for file: ", "tar", KERNEL_MESSAGE);
    debug_append(name);

    debug_message("tar_finddir(): Searched name (hex): ", "tar", KERNEL_MESSAGE);
    for (int i = 0; i < strlen(name); i++) {
        debug_number((u8) name[i], 16);
        debug_append(" ");
    }

    while(header->name[0] != '\0') {
        debug_message("tar_finddir(): Checking file: ", "tar", KERNEL_MESSAGE);
        debug_append(header->name);

        debug_message("tar_finddir(): Header name (hex): ", "tar", KERNEL_MESSAGE);
        for (int i = 0; i < 100; i++) {
            if (header->name[i] == '\0')
                break;

            debug_number((u8) header->name[i], 16);
            debug_append(" ");
        }

        if(! strcmp(header->name, name)) {
            debug_message("tar_finddir(): File found: ", "tar", KERNEL_MESSAGE);    
            debug_append(header->name);

            fs_node_t* _node = (fs_node_t*) malloc(sizeof(fs_node_t));
            memset(_node, 0, sizeof(fs_node_t));

            strcpy(_node->name, header->name);
            _node->flags = (header->typeflag == '5') ? FS_DIRECTORY : FS_FILE;
            _node->length = tar_getsize(header->size);
            _node->inode = node->inode;
            _node->impl = (u32) header;

            if(_node->flags == FS_DIRECTORY) { 
                _node->readdir = tar_readdir;
                _node->finddir = tar_finddir;
            } else {
                _node->read = tar_read;
            }

            return _node;
        }

        debug_message("tar_finddir(): Header pointer before move: ", "tar", KERNEL_MESSAGE);
        debug_number((u32) header, 16);

        u32 size = tar_getsize(header->size);
        header = (tar_header_t*) ((u8*) header + ((size + 511) & -511) + 512);

        debug_message("tar_finddir(): Header pointer after move: ", "tar", KERNEL_MESSAGE);
        debug_number((u32) header, 16);
    }

    debug_message("tar_finddir(): File not found: ", "tar", KERNEL_MESSAGE);
    debug_append(name);

    return NULL;
}

u32 tar_read(fs_node_t* node, u32 offset, u32 size, u8* buffer) {
    tar_header_t* header = (tar_header_t*) node->impl;
    u32 filesize = tar_getsize(header->size);

    if(offset > filesize)
        return 0;

    if(offset + size > filesize)
        size = filesize - offset;

    memcpy(buffer, ((u8*) header) + 512 + offset, size);

    return size;
}
