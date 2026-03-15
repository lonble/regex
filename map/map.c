#include "map/map.h"

#include <stdlib.h>

#include "set/set.h"

struct Map {
    Set *core;
    map_key_equal_cb equal;
    void *context;
};

struct MapIterator {
    SetIterator *core;
};

static bool wrapped_equal(void *data1, void *data2, void *context) {
    MapPair *pair1 = data1, *pair2 = data2;
    Map *map = context;
    if (map->equal(pair1->key, pair2->key, map->context)) {
        return true;
    }
    return false;
}

Map *map_create(map_key_equal_cb equal, void *context) {
    Map *new_map = malloc(sizeof(Map));
    new_map->equal = equal;
    new_map->context = context;
    new_map->core = set_create(wrapped_equal, new_map);
    return new_map;
}

unsigned int map_size(Map *map) {
    return set_size(map->core);
}

MapGetResult map_get(Map *map, void *key) {
    SetIterator *iter = set_create_iterator(map->core);
    while (!set_iterator_end(iter)) {
        MapPair *pair = set_iterate(iter);
        if (map->equal(pair->key, key, map->context)) {
            set_free_iterator(iter);
            return (MapGetResult) { .success = true, .value = pair->value };
        }
    }
    set_free_iterator(iter);
    return (MapGetResult) { .success = false, .value = nullptr };
}

void map_add_unsafe(Map *map, void *key, void *value) {
    MapPair *pair = malloc(sizeof(MapPair));
    pair->key = key;
    pair->value = value;
    set_add_unsafe(map->core, pair);
}

void map_free(Map *map) {
    SetIterator *iter = set_create_iterator(map->core);
    while (!set_iterator_end(iter)) {
        MapPair *pair = set_iterate(iter);
        free(pair);
    }
    set_free_iterator(iter);
    set_free(map->core);
    free(map);
}

MapIterator *map_create_iterator(Map *map) {
    MapIterator *iterator = malloc(sizeof(MapIterator));
    iterator->core = set_create_iterator(map->core);
    return iterator;
}

bool map_iterator_end(MapIterator *iterator) {
    return set_iterator_end(iterator->core);
}

MapPair map_iterate(MapIterator *iterator) {
    if (map_iterator_end(iterator)) {
        return (MapPair) {};
    }
    return *(MapPair *)set_iterate(iterator->core);
}

void map_free_iterator(MapIterator *iterator) {
    set_free_iterator(iterator->core);
    free(iterator);
}
