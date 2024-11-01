#ifndef __QUEUE_H
#define __QUEUE_H

#include "types.h"

//#define QUEUE_MAX_SIZE 512
#define QUEUE_INITIAL_CAPACITY  4

struct queue_element {
    void*   ptr;
    size_t  size;
};

typedef struct {
    struct queue_element* buffer;
    int front;
    int rear;
    int count;
    int capacity;
} queue_t;

void queue_constructor(queue_t* queue);
int queue_isEmpty(queue_t* queue);
int queue_isFull(queue_t* queue);
void queue_expand(queue_t* queue, int difference);
void queue_shrink(queue_t* queue, int difference);
int queue_enqueue(queue_t* queue, void* data, size_t size);
int queue_dequeue(queue_t* queue, void** data, size_t* size);
int queue_clear(queue_t* queue);
void queue_destroy(queue_t* queue);

#endif
