#include "automaton/common.h"

#include <stdlib.h>

#include "list/list.h"

struct AutomatonState *automaton_create_state() {
    static int id = 0; // todo: delete
    struct AutomatonState *state = malloc(sizeof(struct AutomatonState));
    state->accept = false;
    state->id = ++id; // todo: delete
    state->edges = list_create();
    return state;
}

struct AutomatonEdge *automaton_create_edge(signed char input, struct AutomatonState *dest) {
    struct AutomatonEdge *edge = malloc(sizeof(struct AutomatonEdge));
    edge->input = input;
    edge->dest = dest;
    return edge;
}

void automaton_state_transfer(
    struct AutomatonState *source, struct AutomatonState *dest, signed char input) {
    if (source == nullptr || dest == nullptr) {
        return;
    }
    struct AutomatonEdge *edge = automaton_create_edge(input, dest);
    list_append(source->edges, edge);
}

void automaton_free(struct Automaton *automaton) {
    ListIterator *state_iter = list_create_iterator(automaton->states);
    while (!list_iterator_end(state_iter)) {
        struct AutomatonState *state = list_iterate(state_iter);
        ListIterator *edge_iter = list_create_iterator(state->edges);
        while (!list_iterator_end(edge_iter)) {
            free(list_iterate(edge_iter));
        }
        list_free_iterator(edge_iter);
        list_free(state->edges);
        free(state);
    }
    list_free_iterator(state_iter);
    list_free(automaton->states);
    free(automaton);
}
