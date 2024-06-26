#ifndef __LIST_H
#define __LIST_H

#include "memory.h"
#include "console.h"

#define DEFINE_LIST(nodeType)           \
    struct nodeType;                    \
                                        \
    typedef struct nodeType##list {     \
        struct nodeType * head;         \
        struct nodeType * tail;         \
        spin_lock_t lock;               \
        size_t size;                    \
    } nodeType##_list_t;

#define DEFINE_LINK(nodeType)           \
    struct nodeType * next##nodeType;   \
    struct nodeType * prev##nodeType;   \
                                        \
    nodeType##_list_t * container;

#define INITIALIZE_LIST(list)           \
    list.head = list.tail = (void*) 0;  \
    list.size = 0;


#define IMPLEMENT_LIST(nodeType) \
    void append_##nodeType##_list(nodeType##_list_t* list, struct nodeType* node) {     \
        list->tail->next##nodeType = node;                                              \
        node->prev##nodeType = list->tail;                                              \
                                                                                        \
        list->tail = node;                                                              \
        node->next##nodeType = NULL;                                                    \
                                                                                        \
        list->size += 1;                                                                \
                                                                                        \
        if (list->head == NULL)                                                         \
            list->head = node;                                                          \
                                                                                        \
        node->container = list;                                                         \
    }                                                                                   \
                                                                                        \
    void push_##nodeType##_list(nodeType##_list_t* list, struct nodeType* node) {       \
        node->next##nodeType = list->head;                                              \
        node->prev##nodeType = NULL;                                                    \
                                                                                        \
        list->head = node;                                                              \
        list->size += 1;                                                                \
                                                                                        \
        if (list->tail == NULL)                                                         \
            list->tail = node;                                                          \
                                                                                        \
        node->container = list;                                                         \
    }                                                                                   \
                                                                                        \
    struct nodeType* peek_##nodeType##_list(nodeType##_list_t* list) {                  \
        return list->head;                                                              \
    }                                                                                   \
                                                                                        \
    struct nodeType* pop_##nodeType##_list(nodeType##_list_t* list) {                   \
        struct nodeType* res = list->head;                                              \
                                                                                        \
        list->head = list->head->next##nodeType;                                        \
        list->head->prev##nodeType = NULL;                                              \
                                                                                        \
        list->size -= 1;                                                                \
                                                                                        \
        if (list->head == NULL)                                                         \
            list->tail = NULL;                                                          \
                                                                                        \
        res->container = NULL;                                                          \
        return res;                                                                     \
    }                                                                                   \
                                                                                        \
    size_t size_##nodeType##_list(nodeType##_list_t* list) {                            \
        return list->size;                                                              \
    }                                                                                   \
                                                                                        \
    struct nodeType* next_##nodeType##_list(struct nodeType* node) {                    \
        return node->next##nodeType;                                                    \
    }                                                                                   \
                                                                                        \
    void remove_##nodeType (nodeType##_list_t* list, struct nodeType* node) {           \
        if (node->container == list) {                                                  \
            if (node->prev##nodeType == NULL) {                                         \
                list->head = node->next##nodeType;                                      \
            } else {                                                                    \
                node->prev##nodeType = node->next##nodeType;                            \
            }                                                                           \
                                                                                        \
            if (node->next##nodeType == NULL) {                                         \
                list->tail = node->prev##nodeType;                                      \
            } else {                                                                    \
                node->next##nodeType = node->prev##nodeType;                            \
            }                                                                           \
        }                                                                               \
                                                                                        \
        node->container = NULL;                                                         \
    }

#endif
