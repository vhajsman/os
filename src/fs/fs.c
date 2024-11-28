#include "fs.h"
#include "debug.h"
#include "string.h"
#include "memory/memory.h"
#include "console.h"
#include "kernel.h"
#include "device.h"

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
    IGNORE_UNUSED(read);
    IGNORE_UNUSED(write);

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

            if((found_node->flags & 0x7) == FS_DIRECTORY) {
                // fs_destroy(found_node);

                return fs_finddir(found_node, name);
            }

            // fs_destroy(found_node); 

            return found_node;
        }

        index++;
    }

    return NULL;
}

void fs_destroy(fs_node_t* node) {
    if(node == NULL)
        return;

    debug_message("fs_destroy(): destroy node: ", "fs", KERNEL_MESSAGE);
    debug_append(node->name);

    if(node->nodeptr != NULL) {
        fs_destroy(node->nodeptr);
        debug_append(" (nodeptr)");
    }

    free(node);
}

int fs_resolvepath(const char* path, fs_node_t* currentnode, fs_node_t** target) {
    fs_node_t* current = currentnode == NULL ? fs_root : currentnode;
    *target = (fs_node_t*) NULL;

    if (path[0] == '/')
        path++;

    char* path_copy = strdup(path);
    char* token =     strtok(path_copy, "/");

    while(token != NULL) {
        debug_message("fs_resolvepath(): Resolving component: ", "fs", KERNEL_MESSAGE);
        debug_append(token);

        if(!(current->flags & FS_DIRECTORY)) {
            debug_message("fs_resolvepath(): not a directory: ", "fs", KERNEL_ERROR);
            debug_append(path);

            free(path_copy);
            return 1;
        }

        if(!current->finddir) {
            debug_message("fs_resolvepath(): finddir() not supported by target fs", "fs", KERNEL_ERROR);
            
            free(path_copy);
            return 2;
        }

        current = current->finddir(current, token);

        if(current == NULL) {
            debug_message("fs_resolvepath(): not found: ", "fs", KERNEL_ERROR);
            debug_append(path);

            free(path_copy);
            return 3;
        }

        token = strtok(NULL, "/");
    }

    *target = current;

    free(path_copy);
    return 0;
}

u32 fs_getfilesizen(fs_node_t* filenode) {
    return filenode->length;
}

int fs_readfilen(fs_node_t* filenode, char* buffer, u32 buffer_size) {
    if(filenode == NULL) {
        debug_message("fs_readfilenode(): invalid file node.", "fs", KERNEL_ERROR);
        return -1;
    }

    if(filenode->read == NULL) {
        debug_message("fs_readfilenode(): read() not supported by target fs.", "fs", KERNEL_ERROR);
        return -2;
    }

    if(filenode->length > buffer_size) {
        debug_message("fs_readfilenode(): not enough memory", "fs", KERNEL_ERROR);
        return -3;
    }

    u32 bytesread = filenode->read(filenode, 0, filenode->length, (u8*) buffer);

    if(bytesread != filenode->length) {
        debug_message("fs_readfilenode(): read incomplete", "fs", KERNEL_ERROR);
        return -4;
    }

    return bytesread;
}

int fs_readfile(const char* path, char* buffer, u32 buffer_size, fs_node_t* currentnode) {
    fs_node_t* node;

    if(fs_resolvepath(path, currentnode, &node) != 0) {
        return -5;
    }

    return fs_readfilen(node, buffer, buffer_size);
}

int fs_cat(fs_node_t* node) {
    if(node == NULL) {
        debug_message("fs_cat(): node null", "fs", KERNEL_MESSAGE);
        return 0;
    }

    u32 length = node->length + 2;

    char buffer[length];
    memset(buffer, '\0', length);

    if(fs_readfilen(node, buffer, length) != (int) node->length) {
        debug_message("fs_cat(): read failed. filesize not equal the excepted.", "fs", KERNEL_ERROR);
        return 1;
    }

    puts(buffer);
    return 0;
}

bool fs_isinitrd(const char* path) {
    if(path[0] != '/')
        return false;

    if(path[1] == 'm' && path[2] == 'n' && path[3] == 't' && path[4] == '/')
        return false;

    return true;
}

struct fs_mnt* fs_mounts = NULL;

int fs_mount(device_t* dev, char* mnt, char* fs_type, file_permissions_t permissions) {
    if(dev == NULL || mnt == NULL) {
        debug_message("fs_mount(): invalid parameters", "fs", KERNEL_ERROR);
        return 1;
    }

    struct fs_mnt* new_mount = (struct fs_mnt*) malloc(sizeof(struct fs_mnt));
    if(mnt == NULL) {
        debug_message("fs_mount(): not enough memory", "fs", KERNEL_ERROR);
        return 2;
    }

    strncpy(new_mount->mountpoint, mnt, sizeof(new_mount->mountpoint) - 1);
    new_mount->mountpoint[sizeof(new_mount->mountpoint) - 1] = '\0';

    strncpy(new_mount->fs_type, fs_type, sizeof(new_mount->fs_type) - 1);
    new_mount->fs_type[sizeof(new_mount->fs_type) - 1] = '\0';

    new_mount->dev = dev;
    new_mount->permission = permissions;

    new_mount->next = NULL;

    if(fs_mounts == NULL) {
        fs_mounts = new_mount;
    } else {
        struct fs_mnt* temp = fs_mounts;

        while(temp->next != NULL)
            temp = temp->next;
        temp->next = new_mount;
    }

    return 0;
}

int fs_unmount(char* mnt) {
    if(mnt == NULL) {
        debug_message("fs_unmount(): invalid parameters", "fs", KERNEL_ERROR);
        return 1;
    }

    struct fs_mnt* prev = NULL;
    struct fs_mnt* curr = fs_mounts;
    
    while(curr != NULL) {
        if(strcmp(curr->mountpoint, mnt) == 0) {
            if(prev == NULL) {
                fs_mounts = curr->next;
            } else {
                prev->next = curr->next;
            }

            free(curr);
            return 0;
        }

        prev = curr;
        curr = curr->next;
    }

    debug_message("fs_unmount(): mountpoint not found", "fs", KERNEL_ERROR);
    return 2;
}

struct fs_mnt* fs_findmnt(char* mnt) {
    if(mnt == NULL) {
        debug_message("fs_findmnt(): invalid parameters", "fs", KERNEL_ERROR);
        return NULL;
    }

    struct fs_mnt* curr = fs_mounts;
    while(curr != NULL) {
        if(strcmp(curr->mountpoint, mnt) == 0)
            return curr;
        curr = curr->next;
    }

    debug_message("fs_findmnt(): mountpoint not found", "fs", KERNEL_ERROR);
    return NULL;
}
