#ifndef __VFS_H
#define __VFS_H

#include "kernel.h"
#include "generictree.h"

#define VFS_EXT2_MAGIC  0xeeee2222

#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR_STRING "/"
#define PATH_UP  ".."
#define PATH_DOT "."
#define VFS_EXT2_MAGIC 0xeeee2222

#define O_RDONLY     0x0000
#define O_WRONLY     0x0001
#define O_RDWR       0x0002
#define O_APPEND     0x0008
#define O_CREAT      0x0200
#define O_TRUNC      0x0400
#define O_EXCL       0x0800
#define O_NOFOLLOW   0x1000
#define O_PATH       0x2000

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x04
#define FS_BLOCKDEVICE 0x08
#define FS_PIPE        0x10
#define FS_SYMLINK     0x20
#define FS_MOUNTPOINT  0x40

#define     _IFMT   0170000 /* type of file */
#define     _IFDIR  0040000 /* directory */
#define     _IFCHR  0020000 /* character special */
#define     _IFBLK  0060000 /* block special */
#define     _IFREG  0100000 /* regular */
#define     _IFLNK  0120000 /* symbolic link */
#define     _IFSOCK 0140000 /* socket */
#define     _IFIFO  0010000 /* fifo */

struct vfs_node;

typedef u32     (*get_file_size_callback)       (struct vfs_node* node);
typedef u32     (*read_callback)                (struct vfs_node*, u32, u32, char*);
typedef u32     (*write_callback)               (struct vfs_node *, u32, u32, char *);
typedef void    (*open_callback)                (struct vfs_node*, u32 flags);
typedef void    (*create_callback)              (struct vfs_node*, char* name, u16 permission);
typedef void    (*unlink_callback)              (struct vfs_node*, char* name);
typedef void    (*mkdir_callback)               (struct vfs_node*, char* name, u16 permission);
typedef int     (*ioctl_callback)               (struct vfs_node*, int request, void* argp);
typedef int     (*get_size_callback)            (struct vfs_node*);
typedef void    (*chmod_callback)               (struct vfs_node*, u32 mode);
typedef char**  (*listdir_callback)             (struct vfs_node*);
typedef void    (*close_callback)               (struct vfs_node*);
typedef struct  dirent *(*readdir_callback)     (struct vfs_node*, u32);
typedef struct  vfs_node *(*finddir_callback)   (struct vfs_node*, char* name);

struct vfs_node {
    char name[256];
    void* device;
    u32 mask;
    u32 uid;
    u32 gid;
    u32 flags;
    u32 inode_num;
    u32 size;
    u32 fs_type;
    u32 open_flags;

    u32 create_time;
    u32 access_time;
    u32 modified_time;

    u32 offset;

    unsigned nlink;
    int refcount;

    read_callback read;
    write_callback write;
    open_callback open;
    close_callback close;
    readdir_callback readdir;
    finddir_callback finddir;
    create_callback create;
    unlink_callback unlink;
    mkdir_callback mkdir;
    ioctl_callback ioctl;
    get_size_callback getSize;
    chmod_callback chmod;
    get_file_size_callback getFileSize;

    listdir_callback listdir;
};

typedef struct vfs_node vfs_node_t;

typedef struct vfs_entry {
    char* name;
    vfs_node_t* file;
}vfs_entry_t;

u32 vfs_getFileSize(vfs_node_t* node);

unsigned int vfs_read(vfs_node_t* node, unsigned int offset, unsigned int size, char* buffer);
unsigned int vfs_write(vfs_node_t* node, unsigned int offset, unsigned int size, char* buffer);

int vfs_ioctl(vfs_node_t *node, int request, void * argp);
void vfs_unlink(char* name);
void vfs_chmod(vfs_node_t* node, u32 mode);

char* expandPath(char* input);
vfs_node_t* vfs_file_open(const char* file_name, unsigned int flags);
int vfs_createFile(char* name, unsigned short permission);
void vfs_mkdir(char* name, unsigned short permission);
vfs_node_t* vfs_finddir(vfs_node_t* node, char* name);
struct dirent* vfs_readdir(vfs_node_t* node, unsigned int index);
void vfs_open(vfs_node_t* node, unsigned int flags);
void vfs_close(vfs_node_t* node);

void vfs_init();

void vfs_mountDevice(char* mountpoint, vfs_node_t* node);
void vfs_mount_recur(char* path, gtreenode_t* subroot, vfs_node_t* fs_obj);
void vfs_mount(char * path, vfs_node_t * fs_obj);
vfs_node_t* vfs_getMountPoint(char** path);
vfs_node_t* vgs_getMountPoint_recur(char** path, gtreenode_t* subroot);

#endif
