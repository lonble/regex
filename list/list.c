#include "list/list.h"

#include <stdlib.h>

typedef struct Node Node;

struct Node {
    void *data;
    Node *next;
};

struct List {
    Node *sentinel;
    Node *tail;
    unsigned int size;
};

struct ListIterator {
    Node *p;
};

List *list_create() {
    List *new_list = malloc(sizeof(List));
    new_list->sentinel = malloc(sizeof(Node));
    new_list->tail = new_list->sentinel;
    new_list->size = 0;
    new_list->sentinel->data = nullptr;
    new_list->sentinel->next = nullptr;
    return new_list;
}

unsigned int list_size(List *list) {
    return list->size;
}

void *list_head(List *list) {
    if (list->size == 0) {
        return nullptr;
    }
    return list->sentinel->next->data;
}

void list_append(List *list, void *data) {
    list->tail->next = malloc(sizeof(Node));
    list->tail = list->tail->next;
    list->tail->data = data;
    list->tail->next = nullptr;
    ++list->size;
}

void list_concatenate(List *left, List *right) {
    left->tail->next = right->sentinel->next;
    left->tail = right->tail;
    left->size += right->size;
    free(right->sentinel);
    free(right);
}

void list_free(List *list) {
    for (Node *iter = list->sentinel; iter != nullptr;) {
        Node *next = iter->next;
        free(iter);
        iter = next;
    }
    free(list);
}

ListIterator *list_create_iterator(List *list) {
    ListIterator *iterator = malloc(sizeof(ListIterator));
    iterator->p = list->sentinel;
    return iterator;
}

bool list_iterator_end(ListIterator *iterator) {
    if (iterator->p->next == nullptr) {
        return true;
    }
    return false;
}

void *list_iterate(ListIterator *iterator) {
    if (list_iterator_end(iterator)) {
        return nullptr;
    }
    iterator->p = iterator->p->next;
    return iterator->p->data;
}

void list_free_iterator(ListIterator *iterator) {
    free(iterator);
}
