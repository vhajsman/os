#include "linkedlist.h"
#include "memory/memory.h"
#include "kernel.h"

#define list_size linkedlist_size

linkedlist_t* linkedlist_create() {
    linkedlist_t* list = malloc(sizeof(linkedlist_t));
    return list;
}

u32 linkedlist_size(linkedlist_t* list) {
    return (!list) ? 0 : list->size;
}

void* linkedlist_removeNode(linkedlist_t* list, linkedlist_node_t* node) {
    void* val = node->val;

    if(list->head == node)
        return linkedlist_removeFront(list);

    if(list->tail == node)
        return linkedlist_removeBack(list);

    node->next->prev = node->prev;
    node->prev->next = node->next;
    list->size--;

    free(node);
    return val;
}

linkedlist_node_t* linkedlist_insertFront(linkedlist_t* list, void* val) {
    linkedlist_node_t* t = malloc(sizeof(linkedlist_node_t));
    
    list->head->prev = t;
    t->next = list->head;
    t->val = val;

    if(!list->head)
        list->tail = t;

    list->head = t;
    list->size ++;

    return t;
}

void linkedlist_insertBack(linkedlist_t* list, void* val) {
    linkedlist_node_t* t = malloc(sizeof(linkedlist_node_t));

    t->prev = list->tail;

    if(list->tail)
        list->tail->next = t;

    t->val = val;

    if(!list->head)
        list->head = t;

    list->tail = t;
    list->size++;
}

void* linkedlist_removeFront(linkedlist_t* list) {
    if(!list->head)
        return NULL;

    linkedlist_node_t* t = list->head;
    void* val = t->val;
    
    list->head = t->next;

    if(list-> head)
        list->head->prev = NULL;

    free(t);

    list->size--;
    return val;
}

void* linkedlist_removeBack(linkedlist_t* list) {
    if(!list->head)
        return NULL;

    linkedlist_node_t* t = list->tail;
    void* val = t->val;

    list->tail = t->prev;

    if(list->tail)
        list->tail->next = NULL;

    free(t);

    list->size--;
    return val;
}

void linkedlist_push(linkedlist_t* list, void* val) {
    linkedlist_insertBack(list, val);
}

linkedlist_node_t* linkedlist_pop(linkedlist_t* list) {
    if(!list->head)
        return NULL;

    linkedlist_node_t* t = list->tail;
    list->tail = t->prev;

    if(list->tail)
        list->tail->next = NULL;

    list->size--;
    return t;
}

void linkedlist_unqueue(linkedlist_t* list, void* val) {
    linkedlist_insertFront(list, val);
}

linkedlist_node_t* linkedlist_dequeue(linkedlist_t* list) {
    return linkedlist_pop(list);
}

void* linkedlist_peekFront(linkedlist_t* list) {
	if(!list->head)
        return NULL;

	return list->head->val;
}

void* linkedlist_peekBack(linkedlist_t* list) {
	if(!list->tail)
        return NULL;

	return list->tail->val;
}

int linkedlist_contain(linkedlist_t* list, void* val) {
    int idx = 0;

    list_foreach(listnode, list) {
        if(listnode->val == val)
            return idx;

        idx++;
    }

    return -1;
}

linkedlist_node_t* linkedlist_getNodeByIndex(linkedlist_t* list, u32 index) {
    if(index >= list_size(list))
        return NULL;

    u32 curr = 0;

    list_foreach(listnode, list) {
        if(index == curr) 
            return listnode;

        curr++;
    }

    return NULL;
}

void* linkedlist_removeByIndex(linkedlist_t* list, int index) {
    linkedlist_node_t* node = linkedlist_getNodeByIndex(list, index);
    return linkedlist_removeNode(list, node);
}

void linkedlist_destroy(linkedlist_t* list) {
	linkedlist_node_t* node = list->head;

	while(node != NULL) {
		linkedlist_node_t* save = node;
		node = node->next;

		free(save);
	}
	
	free(list);
}

void linkedlist_destroyNode(linkedlist_node_t * node) {
	free(node);
}
