#include "vfs.h"
#include "memory/memory.h"
#include "string.h"
#include "fs.h"
#include "debug.h"
#include "errno.h"

gtree_t* vfs_tree;
vfs_node_t* vfs_root;

u32 vfs_getFileSize(vfs_node_t* node) {
    if(node && node->getFileSize)
        return node->getFileSize(node);
    
    return 0;
}

unsigned int vfs_read(vfs_node_t* node, unsigned int offset, unsigned int size, char* buffer) {
    if(node && node->read)
        return node->read(node, offset, size, buffer);
    
    return 0xFFFFFFFF;
}

unsigned int vfs_write(vfs_node_t* node, unsigned int offset, unsigned int size, char* buffer) {
    if(node && node->write)
        return node->write(node, offset, size, buffer);
    
    return 0xFFFFFFFF;
}

void vfs_open(vfs_node_t* node, unsigned int flags) {
    if(!node)
        return;

    if(node->refcount >= 0)
        node->refcount++;

    node->open(node, flags);
}

void vfs_close(vfs_node_t* node) {
    if(!node || node == vfs_root || node->refcount == -1) 
        return;

    node->refcount--;

    if(node->refcount == 0)
        node->close(node);
}

void vfs_chmod(vfs_node_t* node, u32 mode) {
    if(node->chmod)
        return node->chmod(node, mode);
}

struct dirent* vfs_readdir(vfs_node_t* node, unsigned int index) {
    if(node && (node->flags & FS_DIRECTORY) && node->readdir)
        return node->readdir(node, index);

    return NULL;
}

vfs_node_t* vfs_finddir(vfs_node_t* node, char* name) {
    if(node && (node->flags & FS_DIRECTORY) && node->finddir)
        return node->finddir(node, name);

    return NULL;
}

int vfs_ioctl(vfs_node_t *node, int request, void* argp) {
    if(!node) 
        return -1;

    if(node->ioctl) 
        return node->ioctl(node, request, argp);
    
    return ENOTTY;
}

void vfs_mkdir(char* name, unsigned short permission) {
    int i = strlen(name);

    char* dirname = strdup(name);
    char* save_dirname = dirname;
    char* parent_path = "/";

    while(i >= 0) {
        if(dirname[i] == '/') {
            if(i != 0) {
                dirname[i] = '\0';
                parent_path = dirname;
            }

            dirname = &dirname[i+1];
            break;
        }

        i--;
    }

    vfs_node_t* parent_node = vfs_file_open(parent_path, 0);
    
    if(!parent_node)
        kfree(save_dirname);

    if(parent_node->mkdir)
        parent_node->mkdir(parent_node, dirname, permission);

    kfree(save_dirname);

    vfs_close(parent_node);
}

int vfs_createFile(char* name, unsigned short permission) {
    int i = strlen(name);

    char* dirname = strdup(name);
    char* save_dirname = dirname;
    char* parent_path = "/";

    while(i >= 0) {
        if(dirname[i] == '/') {
            if(i != 0) {
                dirname[i] = '\0';
                parent_path = dirname;
            }

            dirname = &dirname[i+1];
            break;
        }

        i--;
    }

    vfs_node_t* parent_node = vfs_file_open(parent_path, 0);
    
    if(!parent_node) {
        kfree(save_dirname);
        return -1;
    }

    if(parent_node->create)
        parent_node->create(parent_node, dirname, permission);

    kfree(save_dirname);
    vfs_close(parent_node);

    return 0;


}

void vfs_unlink(char* name) {
    int i = strlen(name);

    char* dirname = strdup(name);
    char* save_dirname = dirname;
    char* parent_path = "/";

    while(i >= 0) {
        if(dirname[i] == '/') {
            if(i != 0) {
                dirname[i] = '\0';
                parent_path = dirname;
            }

            dirname = &dirname[i+1];
            break;
        }

        i--;
    }

    vfs_node_t * parent_node = vfs_file_open(parent_path, 0);

    if(!parent_node)
        kfree(save_dirname);

    if(parent_node->unlink)
        parent_node->unlink(parent_node, dirname);

    kfree(save_dirname);
    vfs_close(parent_node);
}

char* expandPath(char* input) {
    linkedlist_t* input_list = string_split(input, "/", NULL);
    char* ret = linkedlist_toString(input_list, "/");

    return ret;
}

vfs_node_t* vgs_getMountPoint_recur(char** path, gtreenode_t* subroot) {
    int found = 0;
    char* curr_token = strsep(path, "/");
    
    if(curr_token == NULL || !strcmp(curr_token, "")) {
        struct vfs_entry * ent = (struct vfs_entry*) subroot->value;
        return ent->file;
    }
    
    list_foreach(child, subroot->children) {
        gtreenode_t* tchild = (gtreenode_t*) child->val;
        struct vfs_entry* ent = (struct vfs_entry*) (tchild->value);

        if(strcmp(ent->name, curr_token) == 0) {
            found = 1;
            subroot = tchild;
            break;
        }
    }

    if(!found) {
        *path = curr_token;
        return ((struct vfs_entry*) (subroot->value))->file;
    }
    
    return vgs_getMountPoint_recur(path, subroot);
}

vfs_node_t* vfs_getMountPoint(char** path) {
    if(strlen(*path) > 1 && (*path)[strlen(*path) - 1] == '/')
        *(path)[strlen(*path) - 1] = '\0';

    if(!*path || *(path)[0]!= '/') 
        return NULL;

    if(strlen(*path) == 1) {
        *path = '\0';
        struct vfs_entry* ent = (struct vfs_entry*) vfs_tree->root->value;

        return ent->file;
    }

    (*path)++;

    return vgs_getMountPoint_recur(path, vfs_tree->root);
}

vfs_node_t* vfs_file_open(const char* file_name, unsigned int flags) {
    char* curr_token = NULL;
    char* filename = strdup(file_name);
    char* free_filename = filename;
    char* save = strdup(filename);
    char* original_filename = filename;
    char* new_start = NULL;

    vfs_node_t* nextnode = NULL;
    vfs_node_t* startpoint = vfs_getMountPoint(&filename);

    if(!startpoint) 
        return NULL;

    if(filename)
        new_start = strstr(save + (filename - original_filename), filename);

    while( filename != NULL  && ((curr_token = strsep(&new_start, "/")) != NULL)) {
        nextnode = vfs_finddir(startpoint, curr_token);

        if(!nextnode) 
            return NULL;

        startpoint = nextnode;
    }

    if(!nextnode)
        nextnode = startpoint;

    vfs_open(nextnode, flags);

    kfree(save);
    kfree(free_filename);

    return nextnode;
}

void vfs_init() {
    vfs_tree = tree_create();
    struct vfs_entry* root = kmalloc(sizeof(struct vfs_entry));

    root->name = strdup("root");
    root->file = NULL;

    tree_insert(vfs_tree, NULL, root);

    debug_message("Filesystem init ok", "VFS", KERNEL_OK);
}

void vfs_mountDevice(char* mountpoint, vfs_node_t* node) {
    vfs_mount(mountpoint, node);
}

void vfs_mount_recur(char* path, gtreenode_t* subroot, vfs_node_t* fs_obj) {
    int found = 0;
    char* curr_token = strsep(&path, "/");

    if(curr_token == NULL || !strcmp(curr_token, "")) {
        struct vfs_entry* ent = (struct vfs_entry*) subroot->value;

        if(ent->file) {
            debug_message("Path already mounted: ", "VFS", KERNEL_ERROR);
            debug_append(path);

            return;
        }

        if(!strcmp(ent->name, "/")) 
            vfs_root = fs_obj;

        ent->file = fs_obj;

        return;
    }

    list_foreach(child, subroot->children) {
        gtreenode_t* tchild = (gtreenode_t*) child->val;
        struct vfs_entry* ent = (struct vfs_entry*) (tchild->value);
        
        if(strcmp(ent->name, curr_token) == 0) {
            found = 1;
            subroot = tchild;
        }
    }

    if(!found) {
         struct vfs_entry* ent = kcalloc(sizeof(struct vfs_entry), 1);

         ent->name = strdup(curr_token);
         subroot = tree_insert(vfs_tree, subroot, ent);
    }
    
    vfs_mount_recur(path, subroot, fs_obj);
}

void vfs_mount(char * path, vfs_node_t * fs_obj) {
    fs_obj->refcount = -1;
    fs_obj->fs_type= 0;

    if(path[0] == '/' && strlen(path) == 1) {
        struct vfs_entry* ent = (struct vfs_entry*) vfs_tree->root->value;
        if(ent->file) {
            debug_message("Path already mounted: ", "VFS", KERNEL_ERROR);
            debug_append(path);

            return;
        }

        vfs_root = fs_obj;
        ent->file = fs_obj;

        debug_message("Mount device ", "VFS", KERNEL_OK);
        debug_append(path);

        return;
    }

    vfs_mount_recur(path + 1, vfs_tree->root, fs_obj);
}
