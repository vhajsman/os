#ifndef __TAR_H
#define __TAR_H

#include "types.h"
#include "fs.h"

typedef struct tar_header {
    char name[100];       /* Name of the file */
    char mode[8];         /* File mode */
    char uid[8];          /* Owner's numeric user ID */
    char gid[8];          /* Group's numeric user ID */
    char size[12];        /* File size in bytes (octal) */
    char mtime[12];       /* Last modification time in numeric Unix time format */
    char checksum[8];     /* Checksum for header record */
    char typeflag;        /* Type flag: '0' = normal file, '5' = directory */
    char linkname[100];   /* Name of linked file */
    char magic[6];        /* UStar indicator "ustar" */
    char version[2];      /* UStar version "00" */
    char uname[32];       /* Owner user name */
    char gname[32];       /* Owner group name */
    char devmajor[8];     /* Device major number */
    char devminor[8];     /* Device minor number */
    char prefix[155];     /* Filename prefix */
    char pad[12];         /* Padding */
} tar_header_t;

const char* tar_findf(const char* tar_data, const char* filename);
size_t tar_readf(const char* tar_data, const char* filename, char* buffer, size_t max_size);
void tar_list(const char* tar_data);

struct fs_dirent* tar_readdir(fs_node_t* node, u32 index);
fs_node_t* tar_finddir(fs_node_t* node, char* name);
u32 tar_read(fs_node_t* node, u32 offset, u32 size, u8* buffer);

#endif
