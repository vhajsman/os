#include "fstab.h"
#include "initrd.h"
#include "string.h"
#include "debug.h"
#include "fs.h"

int fstab_mount_line(char* buffer) {
    if(buffer == NULL) {
        debug_message("fstab_mount_line(): invalid parameters", "fstab", KERNEL_ERROR);
        return 1;
    }

    const char* curr = buffer;

    while(*curr != '\0') {
        char device[16];
        char mountpoint[256];
        char fs_type[8] = {0};

        file_permissions_t permissions = 0;;

        const char* line_start = curr;
        const char* line_end   = line_start;

        while(*line_end != '\0' && *line_end != '\n')
            line_end++;
        
        int line_len = line_end - line_start;

        if(line_len >= 512) {
            debug_message("line too long", "fstab", KERNEL_ERROR);
            return 103;
        }

        char line[512] = {0};

        for(int i = 0; i < line_len; i++)
            line[i] = line_start[i];

        line[line_len] = '\0';

        int idx = 0, 
            dev_idx = 0,
            mnt_idx = 0,
            fs_idx = 0,
            perm = 0;

        while(line[idx] != '\0') {
            while(line[idx] == ' ')
                idx++;
            
            if(dev_idx == 0 && line[idx] != '\0') {
                while(line[idx] != ' ' && line[idx] != '\0' && dev_idx < MAX_DEVICES)
                    device[dev_idx++] = line[idx++];
                
                device[dev_idx] = '\0';
            }

            if(mnt_idx == 0 && line[idx] != '\0') {
                while(line[idx] == ' ')
                    idx++;
                
                while(line[idx] != ' ' && line[idx] != '\0')
                    mountpoint[mnt_idx++] = line[idx++];
                
                mountpoint[mnt_idx] = '\0';
            }

            if(fs_idx == 0 && line[idx] != '\0') {
                while(line[idx] == ' ')
                    idx++;
                
                while(line[idx] != ' ' && line[idx] != '\0')
                    fs_type[fs_idx++] = line[idx++];
                
                fs_type[fs_idx] = '\0';
            }

            if(!perm && line[idx] != '\0') {
                while(line[idx] == ' ')
                    idx++;
                
                while(line[idx] >= '0' && line[idx] <= '9')
                    permissions = permissions * 10 + (line[idx++] - '0');
                
                perm = 1;
            }
        }

        if(device[0] != '\0' && mountpoint[0] != '\0' && fs_type[0] != '\0') {
            int dev_idx = device_findByUniqueId(device);
            if(dev_idx < 0) {
                debug_message("device not found: ", "fstab", KERNEL_ERROR);
                debug_append(device);
                
                return 101;
            }

            device_t* dev = device_get(dev_idx);
            int mountresult = fs_mount(dev, mountpoint, fs_type, permissions);

            debug_message("attemping to mount: ", "fstab", KERNEL_MESSAGE);
            debug_append(device);
            debug_append(" -> ");
            debug_append(mountpoint);
            debug_append(", ");
            debug_append(fs_type);
            debug_append(": ");
            debug_number(mountresult, 16);

            if(mountpoint != 0) {
                return 200 + mountresult;
            }
        }

        curr = (*line_end == '\n') ? line_end + 1: line_end;
    }

    return 0;
}

int fstab_mount(char* buffer) {
    if(buffer == NULL) {
        debug_message("fstab_mount(): invalid parameters", "fstab", KERNEL_ERROR);
        return 1;
    }

    const char* curr = buffer;
    int overall = 0;
    int count = 0;

    while(*curr != '\0') {
        const char* line_start = curr;
        const char* line_end   = line_start;

        while(*line_end != '\0' && *line_end != '\n')
            line_end++;
        
        int line_len = line_end - line_start;

        if(line_len > 0) {
            char line[512] = {0};
            for(int i = 0; i < line_len && i < sizeof(line) - 1; i++)
                line[i] = line_start[i];
            
            line[line_len] = '\0';

            int result = fstab_mount_line(line);
            if(result != 0)
                overall = result;
        }

        curr = (*line_end == '\n') ? line_end + 1 : line_end;
        count++;
    }

    debug_message("fstab_mount(): mounted ", "fstab", KERNEL_MESSAGE);
    debug_number(count, 10);
    debug_append(" devices with result overall status code: ");
    debug_number(overall, 16);

    return overall;
}
