#pragma once

typedef struct Queue Queue;

extern Queue *queue_create();
extern void queue_enqueue(Queue *queue, void *data);
extern void *queue_dequeue(Queue *queue);
extern void *queue_head(Queue *queue);
extern bool queue_empty(Queue *queue);
extern void queue_free(Queue *queue);
