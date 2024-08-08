#ifndef __TAR_H
#define __TAR_H

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


#endif
