#include "fat.h"
#include "memory/memory.h"
#include "debug.h"

struct fs_dirent* fat32_readdir(fs_node_t* node, u32 index);
fs_node_t* fat32_finddir(fs_node_t* node, char* name);

static struct fs_dirent temp_dirent;

int fat32_init(fat32_kernel_info_t* kernelinfo, device_t* dev) {
    if(kernelinfo == NULL || dev == NULL) {
        debug_message("fat32_init(): invalid parameters", "fat32", KERNEL_ERROR);
        return 1;
    }

    fat32_bpb_t* bpb = (fat32_bpb_t*) kmalloc(sizeof(fat32_bpb_t));
    if(bpb == NULL || dev->mReadSector(dev, dev->context, 0, bpb)) {
        debug_message("fat32_init(): failed to read boot sector", "fat32", KERNEL_ERROR);
        
        kfree(bpb);
        return 1;
    }

    if(bpb->boot_signature != 0xAA55) {
        debug_message("fat32_init(): invalid boot signature", "fat32", KERNEL_ERROR);
        
        kfree(bpb);
        return 1;
    }

    kernelinfo->bps = bpb->bps;
    kernelinfo->spc = bpb->spc;
    kernelinfo->reserved_sectors = bpb->sectors_reserved;
    kernelinfo->num_fats = bpb->num_fats;
    kernelinfo->fat_size = (bpb->fat_size_16 != 0) ? bpb->fat_size_16 : bpb->fat_size_32;
    kernelinfo->fat_start = bpb->sectors_reserved;
    kernelinfo->heap_start = kernelinfo->fat_start + (kernelinfo->num_fats * kernelinfo->fat_size);
    kernelinfo->root = bpb->root_cluster;
    kernelinfo->dev = dev;

    fat32_fsinfo_t* fsinfo = malloc(sizeof(fat32_fsinfo_t));
    if(fsinfo == NULL) {
        debug_message("fat32_init(): failed to read fsinfo: not enough memory", "fat32", KERNEL_ERROR);
        
        kfree(bpb);
        return 1;
    }

    if(dev->mReadSector(dev, dev->context, bpb->fs_info, fsinfo) != 0) {
        debug_message("fat32_init(): failed to read fsinfo", "fat32", KERNEL_ERROR);

        kfree(bpb);
        kfree(fsinfo);
        return 1;
    }

    if(fsinfo->signature_lead != 0x41615252 || fsinfo->signature != 0x61417272) {
        debug_message("fat32_init(): invalid fsinfo signature", "fat32", KERNEL_ERROR);

        kfree(bpb);
        kfree(fsinfo);
        return 1;
    }

    kfree(bpb);
    kfree(fsinfo);
    return 0;
}

u32 fat32_cluster_to_sector(fat32_kernel_info_t* info, u32 cluster) {
    return info->heap_start + (cluster - 2) * info->spc;
}

int fat32_readTableEntry(fat32_kernel_info_t* info, u32 cluster, u32* cluster_next) {
    u32 fat_offset = cluster * 4;
    u32 fat_sector = info->fat_start + (fat_offset / info->bps);
    u32 entry_offset = fat_offset % info->bps;

    u8* buffer = kmalloc(info->bps);
    if(buffer == NULL) {
        debug_message("fat32_readTableEntry(): not enough memory", "fat32", KERNEL_ERROR);
        return 1;
    }

    if(info->dev->mReadSector(info->dev, info->dev->context, fat_sector, buffer) != 0) {
        debug_message("fat32_readTableEntry(): error reading sector", "fat32", KERNEL_ERROR);

        kfree(buffer);
        return 1;
    }

    *cluster_next = *((u32*) (buffer + entry_offset)) & 0x0FFFFFFF;

    kfree(buffer);
    return 0;
}

int fat32_readClusterChain(fat32_kernel_info_t* info, u32 cluster_start, u8* outbuffer, u32 filesize) {
    u32 cluster = cluster_start;
    u32 offset = 0;
    u32 cluster_size = info->bps * info->spc;

    u8* temp = (u8*) kmalloc(cluster_size);
    if(temp == NULL) {
        debug_message("fat32_readClusterChain(): not enough memory", "fat32", KERNEL_ERROR);
        return 1;
    }

    while(cluster < 0x0FFFFFF8) {
        u32 sector = fat32_cluster_to_sector(info, cluster);

        for(u32 i = 0; i < info->spc; i++) {
            if(info->dev->mReadSector(info->dev, info->dev->context, sector + i, temp + i * info->bps) != 0) {
                debug_message("fat32_readClusterChain(): failed to read sector: ", "fat32", KERNEL_ERROR);
                debug_number(sector + i, 10);
                
                kfree(temp);
                return 1;
            }
        }

        u32 to_copy = (filesize - offset > cluster_size) ? cluster_size : filesize - offset;
        memcpy(outbuffer + offset, temp, to_copy);
        offset += to_copy;

        if(offset >= filesize)
            break;

        if(fat32_readTableEntry(info, cluster, &cluster) != 0) {
            debug_message("fat32_readClusterChain(): failed to read FAT table entry", "fat32", KERNEL_ERROR);

            kfree(temp);
            return 1;
        }
    }

    kfree(temp);
    return 0;
}

int fat32_resolve(fat32_kernel_info_t* info, u32 cluster_start, const char* name, fat32_dirent_t* result) {
    u32 cluster = cluster_start;

    u8* buffer = (u8*) kmalloc(info->bps);
    if(buffer == NULL) {
        debug_message("fat32_resolve(): not enough memory", "fat32", KERNEL_ERROR);
        return 1;
    }

    while(cluster < 0x0FFFFFF8) {
        u32 sector = fat32_cluster_to_sector(info, cluster);

        for(u32 i = 0; i < info->spc; i++) {
            if(info->dev->mReadSector(info->dev, info->dev->context, sector + i, buffer) != 0) {
                debug_message("fat32_resolve(): failed to read sector: ", "fat32", KERNEL_ERROR);
                debug_number(sector + i, 10);
                
                kfree(buffer);
                return 1;
            }

            fat32_dirent_t* entry = (fat32_dirent_t*) buffer;

            for(int ii = 0; ii < info->bps / sizeof(fat32_dirent_t); ii++) {
                if((u8) entry[ii].name[0] == 0xE5 || entry[ii].attr == FAT_ATTR_LONG_NAME)
                    continue;

                if(memcmp(entry[ii].name, (const u8*) name, 11) == 0) {
                    memcpy(result, &entry[ii], sizeof(fat32_dirent_t));

                    kfree(buffer);
                    return 0;
                }
            }
        }

        if(fat32_readTableEntry(info, cluster, &cluster) != 0) {
            debug_message("fat32_resolve(): failed to read FAT table entry", "fat32", KERNEL_ERROR);

            kfree(buffer);
            return 1;
        }
    }

    debug_message("fat32_resolve(): dirent not found: ", "fat32", KERNEL_ERROR);
    debug_append(name);

    kfree(buffer);
    return 1;
}

u32 fat32_read(fs_node_t* node, u32 offset, u32 size, u8* buffer) {
    fat32_file_t* file = (fat32_file_t*) node->data;
    fat32_kernel_info_t* info = (fat32_kernel_info_t*) file->fs;

    if(offset + size > node->length)
        size = node->length - offset;

    u8* temp = (u8*) kmalloc(node->length);
    if(temp == NULL) {
        debug_message("fat32_read(): not enough memory", "fat32", KERNEL_ERROR);
        return 0;
    }

    if(fat32_readClusterChain(info, file->cluster_start, temp, node->length) != 0) {
        debug_message("fat32_read(): failed to read cluster chain", "fat32", KERNEL_ERROR);

        kfree(temp);
        return 0;
    }

    memcpy(buffer, temp + offset, size);

    kfree(temp);
    return size;
}

fs_node_t* fat32_createFsNode(fat32_kernel_info_t* info, fat32_dirent_t* dirent) {
    fs_node_t* node = (fs_node_t*) kmalloc(sizeof(fs_node_t));
    if(node == NULL) {
        debug_message("fat32_createFsNode(): not enough memory", "fat32", KERNEL_ERROR);
        return NULL;
    }

    memset(node, 0x00, sizeof(fs_node_t));

    strncpy(node->name, dirent->name, 11);
    node->length = dirent->filesize;
    node->flags = (dirent->attr & FAT_ATTR_DIRECTORY) ? FS_DIRECTORY : FS_FILE;

    fat32_file_t* file = (fat32_file_t*) kmalloc(sizeof(fat32_file_t));
    if(file == NULL) {
        debug_message("fat32_createFsNode(): not enough memory", "fat32", KERNEL_ERROR);

        kfree(node);
        return NULL;
    }

    file->fs = info;
    file->cluster_start = ((u32) dirent->first_cluster_high << 16) | dirent->first_cluster_low;
    file->filesize = dirent->filesize;
    file->is_dir = (dirent->attr & FAT_ATTR_DIRECTORY) ? true : false;

    node->read = fat32_read;
    node->data = file;

    return node;
}

fs_node_t* fat32_mntroot(fat32_kernel_info_t* info) {
    if(info == NULL) {
        debug_message("fat32_mntroot(): invalid parameters", "fat32", KERNEL_ERROR);
        return NULL;
    }

    fs_node_t* node = (fs_node_t*) kmalloc(sizeof(fs_node_t));
    if(node == NULL) {
        debug_message("fat32_mntroot(): not enough memory", "fat32", KERNEL_ERROR);
        return NULL;
    }

    fat32_file_t* file = (fat32_file_t*) kmalloc(sizeof(fat32_file_t));
    if(file == NULL) {
        debug_message("fat32_mntroot(): not enough memory", "fat32", KERNEL_ERROR);

        kfree(node);
        return NULL;
    }

    memset(node, 0, sizeof(fs_node_t));
    memset(file, 0, sizeof(fat32_file_t));

    file->fs = info;
    file->cluster_start = info->root;
    file->cluster_current = info->root;
    file->is_dir = true;

    node->flags = FS_DIRECTORY;
    node->data = file;
    node->read = fat32_read;
    node->readdir = fat32_readdir;
    node->finddir = fat32_finddir;
    strcpy(node->name, "/");

    return node;
}

struct fs_dirent* fat32_readdir(fs_node_t* node, u32 index) {
    if(!node || !(node->flags & FS_DIRECTORY))
        return NULL;

    fat32_file_t* file = (fat32_file_t*) node->data;
    fat32_kernel_info_t* info = file->fs;

    u32 cluster = file->cluster_start;
    u32 cluster_size = info->bps * info->spc;

    u8* buffer = (u8*) kmalloc(cluster_size);
    if(!buffer) {
        debug_message("fat32_readdir(): not enough memory", "fat32", KERNEL_ERROR);
        return NULL;
    }

    u32 current_index = 0;

    while(cluster < 0x0FFFFFF8) {
        u32 sector = fat32_cluster_to_sector(info, cluster);

        for(u32 i = 0; i < info->spc; i++) {
            if(info->dev->mReadSector(info->dev, info->dev->context, sector + i, buffer + i * info->bps) != 0) {
                debug_message("fat32_readdir(): sector read failed", "fat32", KERNEL_ERROR);

                kfree(buffer);
                return NULL;
            }
        }

        u32 entries_per_cluster = cluster_size / sizeof(fat32_dirent_t);
        fat32_dirent_t* entries = (fat32_dirent_t*) buffer;

        for(u32 i = 0; i < entries_per_cluster; i++) {
            fat32_dirent_t* e = &entries[i];

            if((u8)e->name[0] == 0x00)
                break;

            if((u8)e->name[0] == 0xE5 || (e->attr & FAT_ATTR_LONG_NAME))
                continue;

            if(current_index == index) {
                memset(&temp_dirent, 0, sizeof(struct fs_dirent));

                strncpy(temp_dirent.name, e->name, 11);
                temp_dirent.name[11] = '\0';
                temp_dirent.inode = ((u32)e->first_cluster_high << 16) | e->first_cluster_low;

                kfree(buffer);
                return &temp_dirent;
            }

            current_index++;
        }

        if(fat32_readTableEntry(info, cluster, &cluster) != 0) {
            debug_message("fat32_readdir(): table entry read failed", "fat32", KERNEL_ERROR);

            kfree(buffer);
            return NULL;
        }
    }

    kfree(buffer);
    return NULL;
}

fs_node_t* fat32_finddir(fs_node_t* node, char* name) {
    if(!node || !(node->flags & FS_DIRECTORY))
        return NULL;

    fat32_file_t* file = (fat32_file_t*) node->data;
    fat32_kernel_info_t* info = file->fs;

    u32 cluster = file->cluster_start;
    u32 cluster_size = info->bps * info->spc;

    u8* buffer = (u8*) kmalloc(cluster_size);
    if(!buffer) {
        debug_message("fat32_finddir(): not enough memory", "fat32", KERNEL_ERROR);
        return NULL;
    }

    while(cluster < 0x0FFFFFF8) {
        u32 sector = fat32_cluster_to_sector(info, cluster);

        for(u32 i = 0; i < info->spc; i++) {
            if(info->dev->mReadSector(info->dev, info->dev->context, sector + i, buffer + i * info->bps) != 0) {
                debug_message("fat32_finddir(): sector read failed", "fat32", KERNEL_ERROR);

                kfree(buffer);
                return NULL;
            }
        }

        fat32_dirent_t* entries = (fat32_dirent_t*) buffer;
        u32 entries_count = cluster_size / sizeof(fat32_dirent_t);

        for(u32 i = 0; i < entries_count; i++) {
            fat32_dirent_t* e = &entries[i];

            if((u8)e->name[0] == 0x00)
                break;

            if((u8)e->name[0] == 0xE5 || (e->attr & FAT_ATTR_LONG_NAME))
                continue;

            char entryname[12] = {0};
            strncpy(entryname, e->name, 11);
            entryname[11] = '\0';

            if(strncmp(entryname, name, 11) == 0) {
                fs_node_t* found = fat32_createFsNode(info, e);

                kfree(buffer);
                return found;
            }
        }

        if (fat32_readTableEntry(info, cluster, &cluster) != 0) {
            debug_message("fat32_finddir(): table entry read failed", "fat32", KERNEL_ERROR);

            kfree(buffer);
            return NULL;
        }
    }

    kfree(buffer);
    return NULL;
}

fs_node_t* fat32_mount(device_t* dev) {
    fat32_kernel_info_t* info = (fat32_kernel_info_t*) kmalloc(sizeof(fat32_kernel_info_t));
    if(info == NULL)
        return NULL;

    if(fat32_init(info, dev) != 0) {
        kfree(info);
        return NULL;
    }

    return fat32_mntroot(info);
}


void fat32_register() {
    fs_driver_register("fat32", fat32_mount);
}
