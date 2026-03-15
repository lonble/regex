#include "automaton/convert.h"

#include <stdio.h> // tode: delete
#include <stdlib.h>
#include <stdint.h>

#include "list/list.h"
#include "queue/queue.h"
#include "set/set.h"
#include "map/map.h"

// return true if the final set contains an accepted state
static bool closure(Set *states_set) {
    bool accept = false;
    Queue *unscanned = queue_create();

    // initialize the queue with all sattes in the original set
    SetIterator *state_iter = set_create_iterator(states_set);
    while (!set_iterator_end(state_iter)) {
        queue_enqueue(unscanned, set_iterate(state_iter));
    }
    set_free_iterator(state_iter);

    // add closure states into the set
    while (!queue_empty(unscanned)) {
        struct AutomatonState *state = queue_dequeue(unscanned);
        if (state->accept) {
            accept = true;
        }
        ListIterator *edge_iter = list_create_iterator(state->edges);
        while (!list_iterator_end(edge_iter)) {
            struct AutomatonEdge *edge = list_iterate(edge_iter);
            if (edge->input == AUTOMATON_EDGE_EMPTY && set_add(states_set, edge->dest)) {
                queue_enqueue(unscanned, edge->dest);
            }
        }
        list_free_iterator(edge_iter);
    }

    queue_free(unscanned);
    return accept;
}

static bool address_equal(void *state1, void *state2, [[maybe_unused]] void *context) {
    if (state1 == state2) {
        return true;
    }
    return false;
}

static bool state_set_equal(void *set1, void *set2, [[maybe_unused]] void *context) {
    return set_equal(set1, set2);
}

// todo: delete
static void print_state_map(Map *state_map) {
    MapIterator *map_iter = map_create_iterator(state_map);
    while (!map_iterator_end(map_iter)) {
        MapPair pair = map_iterate(map_iter);
        fputs("Set [", stdout);
        SetIterator *state_iter = set_create_iterator(pair.key);
        while (!set_iterator_end(state_iter)) {
            struct AutomatonState *state = set_iterate(state_iter);
            printf("%d, ", state->id);
        }
        set_free_iterator(state_iter);
        struct AutomatonState *new_state = pair.value;
        printf("\b\b], State %d", new_state->id);
        if (new_state->accept) {
            fputs(" accept", stdout);
        }
        putchar('\n');
    }
    map_free_iterator(map_iter);
}

struct Automaton *automaton_generate_dfa_from_nfa(struct Automaton *nfa) {
    struct Automaton *dfa = malloc(sizeof(struct Automaton));
    // key: state_set, value: new_state
    Map *state_map = map_create(state_set_equal, nullptr);
    Queue *unscanned = queue_create();

    Set *init_state_set = set_create(address_equal, nullptr);
    set_add_unsafe(init_state_set, nfa->start);
    bool init_accpet = closure(init_state_set);

    struct AutomatonState *init_state = automaton_create_state();
    init_state->accept = init_accpet;
    map_add_unsafe(state_map, init_state_set, init_state);
    dfa->start = init_state;
    dfa->end = nullptr;
    dfa->states = list_create();
    list_append(dfa->states, init_state);

    queue_enqueue(unscanned, init_state_set);
    while (!queue_empty(unscanned)) {
        Set *state_set = queue_dequeue(unscanned);
        struct AutomatonState *cur_state = map_get(state_map, state_set).value;
        Map *edge_map = map_create(address_equal, nullptr);
        List *edge_any_list = list_create();

        SetIterator *state_iter = set_create_iterator(state_set);
        while (!set_iterator_end(state_iter)) {
            struct AutomatonState *state = set_iterate(state_iter);

            ListIterator *edge_iter = list_create_iterator(state->edges);
            while (!list_iterator_end(edge_iter)) {
                struct AutomatonEdge *edge = list_iterate(edge_iter);
                if (edge->input == AUTOMATON_EDGE_EMPTY) {
                    // ignore empty edge
                    continue;
                } else if (edge->input == AUTOMATON_EDGE_ANY) {
                    list_append(edge_any_list, edge);
                }

                MapGetResult result = map_get(edge_map, (void *)(intptr_t)edge->input);
                if (result.success) {
                    Set *dest_set = result.value;
                    set_add(dest_set, edge->dest);
                } else {
                    Set *dest_set = set_create(address_equal, nullptr);
                    set_add_unsafe(dest_set, edge->dest);
                    map_add_unsafe(edge_map, (void *)(intptr_t)edge->input, dest_set);
                }
            }
        }
        set_free_iterator(state_iter);

        MapIterator *edge_iter = map_create_iterator(edge_map);
        while (!map_iterator_end(edge_iter)) {
            MapPair pair = map_iterate(edge_iter);

            // extend any pattern to other characters
            if ((signed char)(intptr_t)pair.key != AUTOMATON_EDGE_ANY) {
                ListIterator *edge_any_iter = list_create_iterator(edge_any_list);
                while (!list_iterator_end(edge_any_iter)) {
                    struct AutomatonEdge *edge = list_iterate(edge_any_iter);
                    set_add(pair.value, edge->dest);
                }
                list_free_iterator(edge_any_iter);
            }

            bool accept = closure(pair.value);
            struct AutomatonState *new_state = nullptr;
            MapGetResult result = map_get(state_map, pair.value);
            if (result.success) {
                new_state = result.value;
                set_free(pair.value);
            } else {
                new_state = automaton_create_state();
                new_state->accept = accept;
                map_add_unsafe(state_map, pair.value, new_state);
                list_append(dfa->states, new_state);
                queue_enqueue(unscanned, pair.value);
            }
            automaton_state_transfer(cur_state, new_state, (signed char)(intptr_t)pair.key);
        }
        map_free_iterator(edge_iter);

        list_free(edge_any_list);
        map_free(edge_map);
    }
    queue_free(unscanned);

    // todo: delete
    print_state_map(state_map);

    MapIterator *state_iter = map_create_iterator(state_map);
    while (!map_iterator_end(state_iter)) {
        MapPair pair = map_iterate(state_iter);
        set_free(pair.key);
    }
    map_free_iterator(state_iter);
    map_free(state_map);

    return dfa;
}
