#include "automaton/regex.h"

#include "list/list.h"
#include "automaton/common.h"
#include "automaton/nfa.h"
#include "automaton/dfa.h"
#include "automaton/convert.h"

struct Automaton *automaton_compile(char const *str) {
    struct Automaton *nfa = automaton_generate_nfa(str);
    if (nfa == nullptr) {
        return nullptr;
    }

    struct Automaton *dfa = automaton_generate_dfa_from_nfa(nfa);
    automaton_free(nfa);
    struct Automaton *minimized_dfa = automaton_generate_minimized_dfa(dfa);
    automaton_free(dfa);
    return minimized_dfa;
}

bool automaton_match(struct Automaton *automaton, char const *str) {
    if (str == nullptr) {
        return false;
    }
    struct AutomatonState *state = automaton->start;
    for (char const *iter = str; *iter != '\0'; ++iter) {
        struct AutomatonState *dest = nullptr;
        ListIterator *edge_iter = list_create_iterator(state->edges);
        while (!list_iterator_end(edge_iter)) {
            struct AutomatonEdge *edge = list_iterate(edge_iter);
            if (edge->input == AUTOMATON_EDGE_ANY) {
                dest = edge->dest;
                continue;
            } else if (edge->input == *iter) {
                dest = edge->dest;
                break;
            }
        }
        list_free_iterator(edge_iter);
        if (dest == nullptr) {
            return false;
        }
        state = dest;
    }
    if (state->accept) {
        return true;
    }
    return false;
}

void automaton_destroy(struct Automaton *automaton) {
    automaton_free(automaton);
}
