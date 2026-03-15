#pragma once

#include "list/list.h"

#define AUTOMATON_EDGE_EMPTY ((signed char)0)
#define AUTOMATON_EDGE_ANY ((signed char)-'.')

struct AutomatonState {
    bool accept;
    List *edges;
};

struct AutomatonEdge {
    signed char input;
    struct AutomatonState *dest;
};

struct Automaton {
    struct AutomatonState *start;
    struct AutomatonState *end;
    List *states;
};

extern struct AutomatonState *automaton_create_state();
extern struct AutomatonEdge *automaton_create_edge(signed char input, struct AutomatonState *dest);
extern void automaton_state_transfer(
    struct AutomatonState *source, struct AutomatonState *dest, signed char input);
extern void automaton_free(struct Automaton *automaton);
