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

#endif
