#include "automaton/dfa.h"

#include <stdlib.h>
#include <stdint.h>

#include "map/map.h"

static bool address_equal(void *data1, void *data2, [[maybe_unused]] void *context) {
    if (data1 == data2) {
        return true;
    }
    return false;
}

// make edge of any pattern unique
static void clean_dfa_edge(struct Automaton *dfa) {
    ListIterator *state_iter = list_create_iterator(dfa->states);
    while (!list_iterator_end(state_iter)) {
        struct AutomatonState *state = list_iterate(state_iter);
        struct AutomatonState *default_dest = nullptr;
        ListIterator *edge_iter = list_create_iterator(state->edges);
        while (!list_iterator_end(edge_iter)) {
            struct AutomatonEdge *edge = list_iterate(edge_iter);
            if (edge->input == AUTOMATON_EDGE_ANY) {
                default_dest = edge->dest;
                break;
            }
        }
        list_free_iterator(edge_iter);
        if (default_dest == nullptr) {
            continue;
        }
        List *new_edges = list_create();
        edge_iter = list_create_iterator(state->edges);
        while (!list_iterator_end(edge_iter)) {
            struct AutomatonEdge *edge = list_iterate(edge_iter);
            if (edge->input == AUTOMATON_EDGE_ANY || edge->dest != default_dest) {
                list_append(new_edges, edge);
            } else {
                free(edge);
            }
        }
        list_free_iterator(edge_iter);
        list_free(state->edges);
        state->edges = new_edges;
    }
    list_free_iterator(state_iter);
}

static struct Automaton *generate_dfa_from_partition_list(
    List *partition_list, struct Automaton *origin_dfa) {
    struct Automaton *dfa = malloc(sizeof(struct Automaton));
    dfa->start = nullptr;
    dfa->end = nullptr;
    dfa->states = list_create();

    // key: partition, value: new_state
    Map *state_map = map_create(address_equal, nullptr);
    // key: old_state, value: partition
    Map *partition_map = map_create(address_equal, nullptr);
    ListIterator *partition_iter = list_create_iterator(partition_list);
    while (!list_iterator_end(partition_iter)) {
        List *partition = list_iterate(partition_iter);
        struct AutomatonState *head = list_head(partition);
        struct AutomatonState *new_state = automaton_create_state();
        new_state->accept = head->accept;
        list_append(dfa->states, new_state);
        map_add_unsafe(state_map, partition, new_state);

        ListIterator *state_iter = list_create_iterator(partition);
        while (!list_iterator_end(state_iter)) {
            struct AutomatonState *state = list_iterate(state_iter);
            map_add_unsafe(partition_map, state, partition);
            if (state == origin_dfa->start) {
                dfa->start = new_state;
            }
        }
        list_free_iterator(state_iter);
    }
    list_free_iterator(partition_iter);

    MapIterator *new_state_iter = map_create_iterator(state_map);
    while (!map_iterator_end(new_state_iter)) {
        MapPair pair = map_iterate(new_state_iter);
        List *partition = pair.key;
        struct AutomatonState *new_state = pair.value;
        struct AutomatonState *head = list_head(partition);
        ListIterator *edge_iter = list_create_iterator(head->edges);
        while (!list_iterator_end(edge_iter)) {
            struct AutomatonEdge *edge = list_iterate(edge_iter);
            List *dest_partition = map_get(partition_map, edge->dest).value;
            struct AutomatonState *dest = map_get(state_map, dest_partition).value;
            automaton_state_transfer(new_state, dest, edge->input);
        }
        list_free_iterator(edge_iter);
    }
    map_free_iterator(new_state_iter);
    map_free(partition_map);
    map_free(state_map);

    clean_dfa_edge(dfa);
    return dfa;
}

struct Automaton *automaton_generate_minimized_dfa(struct Automaton *dfa) {
    clean_dfa_edge(dfa);
    List *partition_list = list_create();

    // initialize slpitted states
    List *unaccepted_states = list_create();
    List *accepted_states = list_create();
    ListIterator *state_iter = list_create_iterator(dfa->states);
    while (!list_iterator_end(state_iter)) {
        struct AutomatonState *state = list_iterate(state_iter);
        if (state->accept) {
            list_append(accepted_states, state);
        } else {
            list_append(unaccepted_states, state);
        }
    }
    list_free_iterator(state_iter);
    list_append(partition_list, accepted_states);

    if (list_size(unaccepted_states) != 0) {
        list_append(partition_list, unaccepted_states);

        // split
        while (true) {
            List *new_partition_list = list_create();

            // key: state, value: old_partition
            Map *partition_map = map_create(address_equal, nullptr);
            // initialize old partition map
            ListIterator *partition_iter = list_create_iterator(partition_list);
            while (!list_iterator_end(partition_iter)) {
                List *old_partition = list_iterate(partition_iter);
                ListIterator *state_iter = list_create_iterator(old_partition);
                while (!list_iterator_end(state_iter)) {
                    struct AutomatonState *state = list_iterate(state_iter);
                    map_add_unsafe(partition_map, state, old_partition);
                }
                list_free_iterator(state_iter);
            }
            list_free_iterator(partition_iter);

            // split each partition
            partition_iter = list_create_iterator(partition_list);
            while (!list_iterator_end(partition_iter)) {
                List *old_partition = list_iterate(partition_iter);
                List *splitted_partitions = list_create();
                // key: new_partition, value: map[input, old_partition]
                Map *partition_conditions = map_create(address_equal, nullptr);

                ListIterator *state_iter = list_create_iterator(old_partition);
                while (!list_iterator_end(state_iter)) {
                    struct AutomatonState *state = list_iterate(state_iter);
                    // key: input, value: old_partition
                    Map *edge_map = map_create(address_equal, nullptr);

                    // initialize edge map
                    ListIterator *edge_iter = list_create_iterator(state->edges);
                    while (!list_iterator_end(edge_iter)) {
                        struct AutomatonEdge *edge = list_iterate(edge_iter);
                        List *dest_partition = map_get(partition_map, edge->dest).value;
                        map_add_unsafe(edge_map, (void *)(intptr_t)edge->input, dest_partition);
                    }
                    list_free_iterator(edge_iter);

                    // create or join a new partition
                    List *target_partition = nullptr;
                    ListIterator *new_partition_iter = list_create_iterator(splitted_partitions);
                    while (!list_iterator_end(new_partition_iter)) {
                        List *new_partition = list_iterate(new_partition_iter);
                        Map *condition = map_get(partition_conditions, new_partition).value;
                        if (map_size(condition) != map_size(edge_map)) {
                            continue;
                        }
                        bool eligible = true;
                        MapIterator *condition_iter = map_create_iterator(condition);
                        while (!map_iterator_end(condition_iter)) {
                            MapPair pair = map_iterate(condition_iter);
                            MapGetResult result = map_get(edge_map, pair.key);
                            if (!result.success || result.value != pair.value) {
                                eligible = false;
                                break;
                            }
                        }
                        map_free_iterator(condition_iter);
                        if (eligible) {
                            target_partition = new_partition;
                            break;
                        }
                    }
                    list_free_iterator(new_partition_iter);
                    if (target_partition != nullptr) {
                        map_free(edge_map);
                    } else {
                        target_partition = list_create();
                        list_append(splitted_partitions, target_partition);
                        map_add_unsafe(partition_conditions, target_partition, edge_map);
                    }
                    list_append(target_partition, state);
                }
                list_free_iterator(state_iter);

                // free partition_conditions
                MapIterator *condition_iter = map_create_iterator(partition_conditions);
                while (!map_iterator_end(condition_iter)) {
                    MapPair pair = map_iterate(condition_iter);
                    map_free(pair.value);
                }
                map_free(partition_conditions);

                list_concatenate(new_partition_list, splitted_partitions);
                list_free(old_partition);
            }
            list_free_iterator(partition_iter);

            bool finished = false;
            if (list_size(partition_list) == list_size(new_partition_list)) {
                finished = true;
            }
            map_free(partition_map);
            list_free(partition_list);
            partition_list = new_partition_list;
            if (finished) {
                break;
            }
        }
    }

    struct Automaton *minimized_dfa = generate_dfa_from_partition_list(partition_list, dfa);

    // free partition_list
    ListIterator *partition_iter = list_create_iterator(partition_list);
    while (!list_iterator_end(partition_iter)) {
        list_free(list_iterate(partition_iter));
    }
    list_free_iterator(partition_iter);
    list_free(partition_list);

    return minimized_dfa;
}
