#ifndef __FS_H
#define __FS_H

#include "types.h"

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x03
#define FS_BLOCKDEVICE 0x04
#define FS_PIPE        0x05
#define FS_SYMLINK     0x06
#define FS_MOUNTPOINT  0x08

typedef struct fs_dirent {
    char name[128];
    u32 inode;
} fs_dirent_t;

typedef struct fs_node {
    char name[128];
    u32 mask;
    u32 uid;
    u32 gid;
    u32 flags;
    u32 inode;
    u32 length;
    u32 impl;

    u32 (*read) (struct fs_node* node, u32 offset, u32 size, u8* buffer);
    u32 (*write) (struct fs_node* node, u32 offset, u32 size, u8* buffer);

    void (*open) (struct fs_node* node);
    void (*close) (struct fs_node* node);

    struct fs_dirent* (*readdir) (struct fs_node* node, u32);
    struct fs_node* (*finddir) (struct fs_node*, char* name);

    struct fs_node* nodeptr;
} fs_node_t;

extern fs_node_t* fs_root;

u32 fs_read(fs_node_t* node, u32 offset, u32 size, u8 *buffer);
u32 fs_write(fs_node_t* node, u32 offset, u32 size, u8 *buffer);

void fs_open(fs_node_t* node, u8 read, u8 write);
void fs_close(fs_node_t* node);

struct fs_dirent* fs_readdir(fs_node_t* node, u32 index);
struct fs_node* fs_finddir(fs_node_t* node, char* name);

u32 fs_getfilesizen(fs_node_t* filenode);

void fs_destroy(fs_node_t* node);

/*
    A function to resolve a path from target fs node to path string (char*), defaulting
    from currentnode. Returns status code.

    If currentnode not specified (= NULL), root filesystem is used instead.
*/
int fs_resolvepath(const char* path, fs_node_t* currentnode, fs_node_t** target);

int fs_readfilen(fs_node_t* filenode, char* buffer, u32 buffer_size);
int fs_readfile(const char* path, char* buffer, u32 buffer_size, fs_node_t* currentnode);

int fs_cat(fs_node_t* node);

#endif
