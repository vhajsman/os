#include "generictree.h"
#include "memory/memory.h"

gtree_t* tree_create() {
    return (gtree_t*) kcalloc(sizeof(gtree_t), 1);
}

gtreenode_t* tree_createNode(void* value) {
    gtreenode_t * n = kcalloc(sizeof(gtreenode_t), 1);
    
    n->value = value;
    n->children = linkedlist_create();
    
    return n;
}

gtreenode_t* tree_insert(gtree_t* tree, gtreenode_t* subroot, void* value) {
    gtreenode_t* treenode = kcalloc(sizeof(gtreenode_t), 1);

    treenode->children = linkedlist_create();
    treenode->value = value;

    if(!tree->root) {
        tree->root = treenode;

        return treenode;
    }

    linkedlist_insertFront(subroot->children, treenode);
    return treenode;
}

gtreenode_t* tree_findParent(gtree_t* tree, gtreenode_t* removeNode, int* child_index) {
    if(removeNode == tree->root) 
        return NULL;

    return tree_findParent_recur(tree, removeNode, tree->root, child_index);
}

gtreenode_t* tree_findParent_recur(gtree_t* tree, gtreenode_t* removeNode, gtreenode_t* subroot, int* child_index) {
    int idx;
    
    if((idx = linkedlist_contain(subroot->children, removeNode)) != -1) {
        *child_index = idx;
        return subroot;
    }

    list_foreach(child, subroot->children) {
        gtreenode_t * ret = tree_findParent_recur(tree, removeNode, child->val, child_index);
        
        if(ret != NULL)
            return ret;
    }

    return NULL;
}

void tree_remove(gtree_t* tree, gtreenode_t* removeNode) {
    int child_index = -1;

    gtreenode_t* parent = tree_findParent(tree, removeNode, &child_index);

    if(parent != NULL) {
        gtreenode_t * freethis = linkedlist_removeByIndex(parent->children, child_index);
        kfree(freethis);
    }
}

void tree_toList_recur(gtreenode_t* subroot, linkedlist_t* list) {
    if(subroot == NULL)
        return;

    list_foreach(child, subroot->children) {
        gtreenode_t* curr_treenode = (gtreenode_t*) child->val;
        void* curr_val = curr_treenode->value;

        linkedlist_insertBack(list, curr_val);
        tree_toList_recur(child->val, list);
    }
}

void tree_toList(gtree_t* tree, linkedlist_t* list) {
    tree_toList_recur(tree->root, list);
}

void tree_toArray(gtree_t* tree, void** array, int* size) {
    tree_toArray_recur(tree->root, array, size);
}

void tree_toArray_recur(gtreenode_t* subroot, void** array, int* size) {
    if(subroot== NULL)
        return;

    void* curr_val = (void*) subroot->value;
    
    array[*size] = curr_val;
    *size = *size + 1;

    list_foreach(child, subroot->children) {
        tree_toArray_recur(child->val, array, size);
    }
}
