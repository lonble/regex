#include "set/set.h"

#include <stdlib.h>

typedef struct Node Node;

struct Node {
    void *data;
    Node *next;
};

struct Set {
    Node *sentinel;
    Node *tail;
    set_element_equal_cb equal;
    void *context;
    unsigned int size;
};

struct SetIterator {
    Node *p;
};

Set *set_create(set_element_equal_cb equal, void *context) {
    Set *new_set = malloc(sizeof(Set));
    new_set->equal = equal;
    new_set->context = context;
    new_set->size = 0;
    new_set->sentinel = malloc(sizeof(Node));
    new_set->tail = new_set->sentinel;
    new_set->sentinel->data = nullptr;
    new_set->sentinel->next = nullptr;
    return new_set;
}

unsigned int set_size(Set *set) {
    return set->size;
}

bool set_add(Set *set, void *data) {
    for (Node *iter = set->sentinel->next; iter != nullptr; iter = iter->next) {
        if (set->equal(iter->data, data, set->context)) {
            return false;
        }
    }
    Node *node = malloc(sizeof(Node));
    node->data = data;
    node->next = nullptr;
    set->tail->next = node;
    set->tail = node;
    ++set->size;
    return true;
}

void set_add_unsafe(Set *set, void *data) {
    Node *node = malloc(sizeof(Node));
    node->data = data;
    node->next = nullptr;
    set->tail->next = node;
    set->tail = node;
    ++set->size;
}

bool set_equal(Set *left, Set *right) {
    if (left->equal != right->equal
        || left->context != right->context
        || left->size != right->size) {
        return false;
    }

    for (Node *li = left->sentinel->next; li != nullptr; li = li->next) {
        bool found = false;
        for (Node *ri = right->sentinel->next; ri != nullptr; ri = ri->next) {
            if (left->equal(li->data, ri->data, left->context)) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    return true;
}

void set_free(Set *set) {
    for (Node *iter = set->sentinel; iter != nullptr;) {
        Node *next = iter->next;
        free(iter);
        iter = next;
    }
    free(set);
}

SetIterator *set_create_iterator(Set *set) {
    SetIterator *iterator = malloc(sizeof(SetIterator));
    iterator->p = set->sentinel;
    return iterator;
}

bool set_iterator_end(SetIterator *iterator) {
    if (iterator->p->next == nullptr) {
        return true;
    }
    return false;
}

void *set_iterate(SetIterator *iterator) {
    if (set_iterator_end(iterator)) {
        return nullptr;
    }
    iterator->p = iterator->p->next;
    return iterator->p->data;
}

void set_free_iterator(SetIterator *iterator) {
    free(iterator);
}
