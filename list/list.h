#pragma once

typedef struct List List;
typedef struct ListIterator ListIterator;

extern List *list_create();
extern unsigned int list_size(List *list);
extern void *list_head(List *list);
extern void list_append(List *list, void *data);
// This operation appends all nodes of right to left.
// The nodes of right will be reused, while right itself will be free.
extern void list_concatenate(List *left, List *right);
extern void list_free(List *list);

extern ListIterator *list_create_iterator(List *list);
// if list_iterator_end(iterator) returns false,
// then you can use list_iterate(iterator) to get next data
extern bool list_iterator_end(ListIterator *iterator);
extern void *list_iterate(ListIterator *iterator);
extern void list_free_iterator(ListIterator *iterator);
