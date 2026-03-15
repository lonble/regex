#pragma once

typedef struct Set Set;
typedef struct SetIterator SetIterator;
typedef bool (*set_element_equal_cb)(void *data1, void *data2, void *context);

extern Set *set_create(set_element_equal_cb equal, void *context);
extern unsigned int set_size(Set *set);
extern bool set_add(Set *set, void *data);
// add the element directly without duplicate check
extern void set_add_unsafe(Set *set, void *data);
extern bool set_equal(Set *left, Set *right);
extern void set_free(Set *set);

extern SetIterator *set_create_iterator(Set *set);
// if set_iterator_end(iterator) returns false,
// then you can use set_iterate(iterator) to get next data
extern bool set_iterator_end(SetIterator *iterator);
extern void *set_iterate(SetIterator *iterator);
extern void set_free_iterator(SetIterator *iterator);
