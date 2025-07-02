#include "fat.h"
#include "memory.h"
#include "debug.h"

int fat32_init(fat32_kernel_info_t* kernelinfo, device_t* dev) {
    if(kernelinfo == NULL || dev == NULL) {
        debug_message("fat32_init(): invalid parameters", "fat32", KERNEL_ERROR);
        return 1;
    }

    fat32_bpb_t* bpb = (fat32_bpb_t*) kmalloc(sizeof(fat32_bpb_t));
    if(bpb == NULL || dev->mReadSector(dev, dev->context, 0, bpb)) {
        debug_message("fat32_init(): failed to read boot sector", "fat32", KERNEL_ERROR);
        
        free(bpb);
        return 1;
    }

    if(bpb->boot_signature != 0xAA55) {
        debug_message("fat32_init(): invalid boot signature", "fat32", KERNEL_ERROR);
        
        free(bpb);
        return 1;
    }

    info->bps = bpb->bps;
    info->spc = bpb->spc;
    info->reserved_sectors = bpb->sectors_reserved;
    info->num_fats = bpb->num_fats;
    info->fat_size = (bpb->fat_size_16 != 0) ? bpb->fat_size_16 : bpb->fat_size_32;
    info->fat_start = bpb->sectors_reserved;
    info->heap_start = info->fat_start + (info->num_fats * info->fat_size);
    info->root = bpb->root_cluster;
    info->dev = dev;

    fat32_fsinfo_t* fsinfo = malloc(sizeof(fat32_fsinfo_t));
    if(fsinfo == NULL) {
        debug_message("fat32_init(): failed to read fsinfo: not enough memory", "fat32", KERNEL_ERROR);
        
        free(bpb);
        return 1;
    }

    if(dev->mReadSector(dev, dev->context, bpb->fs_info, fsinfo) != 0) {
        debug_message("fat32_init(): failed to read fsinfo", "fat32", KERNEL_ERROR);

        free(bpb);
        free(fsinfo);
        return 1;
    }

    if(fsinfo->signature_lead != 0x41615252 || fsinfo->signature != 0x61417272) {
        debug_message("fat32_init(): invalid fsinfo signature", "fat32", KERNEL_ERROR);

        free(bpb);
        free(fsinfo);
        return 1;
    }

    free(bpb);
    free(fsinfo);
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

        free(buffer);
        return 1;
    }

    *cluster_next = *((u32*) (buffer + entry_offset)) & 0x0FFFFFFF;

    free(buffer);
    return 0;
}

int fat32_readClusterChain(fat32_kernel_info_t* info, u32 cluster_start, u8* outbuffer, u32 filesize) {
    u32 cluster = cluster_start;
    u32 offset = 0;
    u32 cluster_size = info->bps * info->spc;

    u8* temp = (u8*) malloc(cluster_size)
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
                
                free(temp);
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

            free(temp);
            return 1;
        }
    }

    free(temp);
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
                
                free(temp);
                return 1;
            }

            fat32_dirent_t* entry = (fat32_dirent_t*) buffer;

            for(int ii = 0; ii < info->bps / sizeof(fat32_dirent_t); ii++) {
                if((u8) entry[ii].name[0] = 0xE5 || entry[ii].attr == FAT_ATTR_LONG_NAME)
                    continue;

                if(memcmp(entry[ii].name, name, 11) == 0) {
                    memcpy(result, &entry[ii], sizeof(fat32_dirent_t));

                    free(buffer);
                    return 0;
                }
            }
        }

        if(fat32_readTableEntry(info, cluster, &cluster) != 0) {
            debug_message("fat32_resolve(): failed to read FAT table entry", "fat32", KERNEL_ERROR);

            free(temp);
            return 1;
        }
    }

    debug_message("fat32_resolve(): dirent not found: ", "fat32", KERNEL_ERROR);
    debug_append(name);

    free(buffer);
    return 1;
}

fs_node_t* fat32_mntroot(fat32_fsinfo_t* fsinfo) {
    fs_node_t* node = kmalloc(sizeof(fs_node_t));
    if(node == NULL)
        return NULL;

    node->nodeptr = node;

    // TODO: Register the pseudomethods
}
