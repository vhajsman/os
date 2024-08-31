#include "fs.h"
#include "debug.h"
#include "string.h"
#include "memory/memory.h"

fs_node_t* fs_root = 0;

u32 fs_read(fs_node_t* node, u32 offset, u32 size, u8* buffer) {
    if(!node->read) {
        debug_message("read(): operation not supported", "fs", KERNEL_ERROR);
        return 0;
    }

    return node->read(node, offset, size, buffer);
}

u32 fs_write(fs_node_t* node, u32 offset, u32 size, u8* buffer) {
    if(!node->write) {
        debug_message("write(): operation not supported", "fs", KERNEL_ERROR);
        return 0;
    }

    return node->write(node, offset, size, buffer);
}

void fs_open(fs_node_t* node, u8 read, u8 write) {
    // TODO: handle the access

    if(!node->open) {
        debug_message("open(): operation not supported", "fs", KERNEL_ERROR);
        return;
    }

    node->open(node);
}

void fs_close(fs_node_t* node) {
    if(!node->close) {
        debug_message("close(): operation not supported", "fs", KERNEL_ERROR);
        return;
    }

    node->close(node);
}

struct fs_dirent* fs_readdir(fs_node_t* node, u32 index) {
    if(!node->readdir) {
        debug_message("readdir(): operation not supported", "fs", KERNEL_ERROR);
        return NULL;
    }

    // Check if node is a directory
    if((node->flags & 0x7) != FS_DIRECTORY) {
        debug_message("readdir(): not a directory", "fs", KERNEL_ERROR);
        return NULL;
    }

    return node->readdir(node, index);
}

struct fs_node* fs_finddir(fs_node_t* node, char* name) {
    if(!node->readdir) {
        debug_message("finddir(): operation not supported", "fs", KERNEL_ERROR);
        return NULL;
    }

    // Check if node is a directory
    if((node->flags & 0x7) != FS_DIRECTORY) {
        debug_message("finddir(): not a directory", "fs", KERNEL_ERROR);
        return NULL;
    }
    
    u32 index = 0;
    struct fs_dirent* dirent;

    while((dirent = node->readdir(node, index)) != NULL) {
        if (strcmp(dirent->name, name) == 0) {
            struct fs_node* found_node = (fs_node_t*) malloc(sizeof(fs_node_t));
            memset(found_node, 0, sizeof(fs_node_t));

            strcpy(found_node->name, dirent->name);
            found_node->inode = dirent->inode;
            found_node->flags = node->flags;

            if((found_node->flags & 0x7) == FS_DIRECTORY)
                return fs_finddir(found_node, name);

            return found_node;
        }

        index++;
    }

    return NULL;
}
