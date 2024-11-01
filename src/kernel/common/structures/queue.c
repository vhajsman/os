#include "queue.h"
#include "memory/memory.h"
#include "console.h"
#include "string.h"

void queue_constructor(queue_t* queue) {
    queue->front = 0;
    queue->rear = -1;
    queue->count = 0;

    queue->capacity = QUEUE_INITIAL_CAPACITY;
    queue->buffer = malloc(sizeof(struct queue_element) * queue->capacity);

    if(queue->buffer == NULL) {
        puts("\nqueue_constructor(): not enough memory.");
        queue->capacity = 0;
    }
}

int queue_isEmpty(queue_t* queue) {
    return queue->count == 0;
}

int queue_isFull(queue_t* queue) {
    return queue->count == queue->capacity;
}

void queue_expand(queue_t* queue, int difference) {
    int ncapacity = queue->capacity + difference;
    struct queue_element* nbuffer = realloc(queue->buffer, sizeof(struct queue_element) * ncapacity);

    if(nbuffer == NULL) {
        puts("\nqueue_expand(): not enough memory.");
        return;
    }

    queue->buffer = nbuffer;
    queue->capacity = ncapacity;

    // return ncapacity;
}

void queue_shrink(queue_t* queue, int difference) {
    int ncappacity = queue->capacity - difference;

    if(ncappacity < QUEUE_INITIAL_CAPACITY)
        ncappacity = QUEUE_INITIAL_CAPACITY;

    struct queue_element* nbuffer = realloc(queue->buffer, sizeof(struct queue_element) * ncappacity);

    if(nbuffer == NULL) {
        puts("\nqueue_shrink(): memory error.");
        return;
    }

    queue->buffer = nbuffer;
    queue->capacity = ncappacity;
}

int queue_enqueue(queue_t* queue, void* data, size_t size) {
    if(queue_isFull(queue))
        queue_expand(queue, 1);
    
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->buffer[queue->rear].ptr = malloc(size);

    if(queue->buffer[queue->rear].ptr == NULL)
        return 1;
    
    memcpy(queue->buffer[queue->rear].ptr, data, size);

    queue->buffer[queue->rear].size = size;
    queue->count++;

    return 0;
}

int queue_dequeue(queue_t* queue, void** data, size_t* size) {
    if(queue_isEmpty(queue))
        return -1;
    
    *data = queue->buffer[queue->front].ptr;
    *size = queue->buffer[queue->front].size;

    queue->front = (queue->front + 1) % queue->capacity;
    queue->count--;

    if(queue->count > 0 && queue->count == queue->capacity / 4)
        queue_shrink(queue, 1);
    
    return 0;
}

int queue_clear(queue_t* queue) {
    int i = 0;
    while(!queue_isEmpty(queue)) {
        void* data;
        size_t size;

        queue_dequeue(queue, &data, &size);
        free(data);

        i++;
    }

    return i;
}

void queue_destroy(queue_t* queue) {
    queue_clear(queue);
    free(queue->buffer);
}
