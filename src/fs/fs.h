#ifndef __FS_H
#define __FS_H

#include "types.h"
#include "device.h"

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

    union {
        u32 length;
        u32 size;
    };
    
    u32 impl;

    u32 (*read) (struct fs_node* node, u32 offset, u32 size, u8* buffer);
    u32 (*write) (struct fs_node* node, u32 offset, u32 size, u8* buffer);

    void (*open) (struct fs_node* node);
    void (*close) (struct fs_node* node);

    struct fs_dirent* (*readdir) (struct fs_node* node, u32);
    struct fs_node* (*finddir) (struct fs_node*, char* name);

    struct fs_node* nodeptr;

    u32 type;
    void* data;
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

int fs_readfilenode(fs_node_t* filenode, char* buffer, u32 buffer_size);
int fs_readfile(const char* path, char* buffer, u32 buffer_size, fs_node_t* currentnode);

int fs_cat(fs_node_t* node);


#define FILE_PERMISSION_READ    (1 << 0)
#define FILE_PERMISSION_WRITE   (1 << 1)
#define FILE_PERMISSION_EXECUTE (1 << 2)
#define FILE_PERMISSION_SPECIAL (1 << 3)

#define FILE_PERMISSION_SET(PERMISSIONS, FLAG)      ((PERMISSIONS) |=  (FLAG))
#define FILE_PERMISSION_CLEAR(PERMISSIONS, FLAG)    ((PERMISSIONS) &= ~(FLAG))
#define FILE_PERMISSION_TOGGLE(PERMISSIONS, FLAG)   ((PERMISSIONS) ^=  (FLAG))
#define FILE_PERMISSION_CHECK(PERMISSIONS, FLAG)    (((PERMISSIONS) &  (FLAG)) != 0)

typedef u8 file_permissions_t;


#define MAX_MOUNT_POINTS 16

struct fs_mnt {
    char mountpoint[256];
    char fs_type[8];
    device_t* dev;
    file_permissions_t permission;
    fs_node_t* node;
};

// int fs_mount(device_t* dev, char* mnt, char* fs_type, file_permissions_t permissions);
// int fs_unmount(char* mnt);
// struct fs_mnt* fs_findmnt(char* mnt);
// bool fs_ismounted(device_t* dev);
// char* fs_findmntpath(device_t* dev);

int fs_mount(device_t* dev, const char* mnt, const char* fs_type, file_permissions_t permissions);
int fs_unmount(const char* mnt);

int findFreeMntField();
struct fs_mnt* findMntByName(const char* mnt);

bool fs_ismounted(device_t* dev);
char* fs_findMntByDevice(device_t* dev);

void fs_mounts_debug();

// =========================================================
// =================== FILESYSTEM DRIVERS (backend/fs_drivers.c)
// =========================================================

#ifndef MAX_FS_DRIVERS
#define MAX_FS_DRIVERS 24
#endif

#include "fs.h"
#include "debug.h"

typedef fs_node_t* (*fs_mount_callback_t)(device_t* dev);

typedef struct fs_driver_entry {
    const char* name;
    fs_mount_callback_t mount_callback;
} fs_driver_entry_t;

extern fs_driver_entry_t fs_drivers[MAX_FS_DRIVERS];
extern struct fs_mnt* fs_mounts[MAX_MOUNT_POINTS];
extern struct fs_mnt mnt_storage[MAX_MOUNT_POINTS];

/**
 * @brief Registers the filesystem driver. Returns 0 if success, 1 otherwise
 * 
 * @param name filesystem name
 * @param mount_callback mount callback
 * @return int 
 */
int fs_driver_register(const char* name, fs_mount_callback_t mount_callback);

/**
 * @brief Gets function callback. Returns the callback pointer if found, NULL otherwise
 * 
 * @param name filesystem name
 * @return fs_mount_callback_t 
 */
fs_mount_callback_t fs_driver_getCallback(char* name);

#endif
