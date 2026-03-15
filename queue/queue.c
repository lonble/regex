#include "queue/queue.h"

#include <stdlib.h>

typedef struct Node Node;

struct Node {
    void *data;
    Node *next;
};

struct Queue {
    Node *sentinel;
    Node *tail;
};

Queue *queue_create() {
    Queue *new_queue = malloc(sizeof(Queue));
    new_queue->sentinel = malloc(sizeof(Node));
    new_queue->tail = new_queue->sentinel;
    new_queue->sentinel->data = nullptr;
    new_queue->sentinel->next = nullptr;
    return new_queue;
}

bool queue_empty(Queue *queue) {
    if (queue->sentinel->next == nullptr) {
        return true;
    }
    return false;
}

void queue_enqueue(Queue *queue, void *data) {
    queue->tail->next = malloc(sizeof(Node));
    queue->tail = queue->tail->next;
    queue->tail->data = data;
    queue->tail->next = nullptr;
}

void *queue_dequeue(Queue *queue) {
    if (queue_empty(queue)) {
        return nullptr;
    }
    Node *head = queue->sentinel->next;
    void *result = head->data;
    queue->sentinel->next = head->next;
    if (head == queue->tail) {
        queue->tail = queue->sentinel;
    }
    free(head);
    return result;
}

void *queue_head(Queue *queue) {
    if (queue_empty(queue)) {
        return nullptr;
    }
    return queue->sentinel->next->data;
}

void queue_free(Queue *queue) {
    while (!queue_empty(queue)) {
        queue_dequeue(queue);
    }
    free(queue->sentinel);
    free(queue);
}
