#ifndef __GENERICTREE_H
#define __GENERICTREE_H

#include "linkedlist.h"

typedef struct gtreenode {
    linkedlist_t* children;
    void* value;
}gtreenode_t;

typedef struct gtree {
    gtreenode_t* root;
}gtree_t;

gtree_t* tree_create();
gtreenode_t* tree_createNode(void* value);
gtreenode_t* tree_insert(gtree_t* tree, gtreenode_t* subroot, void* value);
void tree_remove(gtree_t* tree, gtreenode_t* removeNode);

gtreenode_t* tree_findParent(gtree_t* tree, gtreenode_t* removeNode, int* child_index);
gtreenode_t* tree_findParent_recur(gtree_t* tree, gtreenode_t* removeNode, gtreenode_t* subroot, int* child_index);

void tree_toList_recur(gtreenode_t* subroot, linkedlist_t* list);
void tree_toList(gtree_t* tree, linkedlist_t* list);

void tree_toArray(gtree_t* tree, void** array, int* size);
void tree_toArray_recur(gtreenode_t* subroot, void** array, int* size);

#endif
