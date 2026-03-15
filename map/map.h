#pragma once

struct MapPair {
    void *key;
    void *value;
};

// if map_get() failed, .value = nullptr
struct MapGetResult {
    bool success;
    void *value;
};

typedef struct Map Map;
typedef struct MapPair MapPair;
typedef struct MapIterator MapIterator;
typedef struct MapGetResult MapGetResult;
typedef bool (*map_key_equal_cb)(void *key1, void *key2, void *context);

extern Map *map_create(map_key_equal_cb equal, void *context);
extern unsigned int map_size(Map *map);
extern MapGetResult map_get(Map *map, void *key);
// add the element directly without duplicate check
extern void map_add_unsafe(Map *map, void *key, void *value);
extern void map_free(Map *map);

extern MapIterator *map_create_iterator(Map *map);
// if map_iterator_end(iterator) returns false,
// then you can use map_iterate(iterator) to get next data
extern bool map_iterator_end(MapIterator *iterator);
extern MapPair map_iterate(MapIterator *iterator);
extern void map_free_iterator(MapIterator *iterator);
