#ifndef __FS_H
#define __FS_H

#include "types.h"

#define FILENAME_MAX_LENGTH 32
#define VOLUME_MAX 26

#define FS_FILE       0
#define FS_DIRECTORY  1
#define FS_INVALID    2

struct _fs_file {
	char    name[FILENAME_MAX_LENGTH];
	u32     flags;
	u32     fileLength;
	u32     id;
	u32     eof;
	u32     position;
	u32     currentCluster;
	u32     device;
};

typedef struct _fs_file     file_t;
typedef struct _fs_file*    fileptr_t;

struct _fs_filesystem {
	char name [8];

	file_t  (*directory)    (const char* dname);
	void    (*mount)        ();
	void    (*read)         (fileptr_t file, unsigned char* buffer, unsigned int length);
	void    (*close)        (fileptr_t);
	file_t  (*open)         (const char* fname);

};

typedef struct _fs_filesystem   filesystem_t;
typedef struct _fs_filesystem*  filesystemptr_t;

extern file_t volume_openf(const char* filename);
extern void volume_readf(fileptr_t file, unsigned char* buffer, unsigned int length);
extern void volume_close(fileptr_t file);

extern void volume_registerFs(filesystemptr_t fs, unsigned int device_id);
extern void volume_unregisterFs(filesystemptr_t fs);
extern void volume_unregisterFsById(unsigned int device_id);

#endif
