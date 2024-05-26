#ifndef __LINKEDLIST_H
#define __LINKEDLIST_H

#include "types.h"

typedef struct linkedlist_node {
    struct linkedlist_node* prev;
    struct linkedlist_node* next;
    void* val;
} linkedlist_node_t;

typedef struct linkedlist {
    linkedlist_node_t* head;
    linkedlist_node_t* tail;
    u32 size;
} linkedlist_t;

#define list_foreach(t, list)   \
    for(linkedlist_node_t* t = list->head; t != NULL; t = t->next)

u32 linkedlist_size(linkedlist_t* list);

linkedlist_t* linkedlist_create();
void linkedlist_destroy(linkedlist_t* list);
void linkedlist_destroyNode(linkedlist_node_t * node);

void* linkedlist_removeNode(linkedlist_t* list, linkedlist_node_t* node);
void* linkedlist_removeFront(linkedlist_t* list);
void* linkedlist_removeBack(linkedlist_t* list);

linkedlist_node_t* linkedlist_insertFront(linkedlist_t* list, void* val);
void linkedlist_insertBack(linkedlist_t* list, void* val);

void linkedlist_push(linkedlist_t* list, void* val);
linkedlist_node_t* linkedlist_pop(linkedlist_t* list);

void linkedlist_unqueue(linkedlist_t* list, void* val);
linkedlist_node_t* linkedlist_dequeue(linkedlist_t* list);

void* linkedlist_peekFront(linkedlist_t* list);
void* linkedlist_peekBack(linkedlist_t* list);

int linkedlist_contain(linkedlist_t* list, void* val);

linkedlist_node_t* linkedlist_getNodeByIndex(linkedlist_t* list, int index);
void* linkedlist_removeByIndex(linkedlist_t* list, int index);

#endif
