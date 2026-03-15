#include "automaton/nfa.h"

#include <stdlib.h>
#include <string.h>

#include "list/list.h"
#include "stack/stack.h"

#define START_SYMBOL ((signed char)0)
#define L_BRA ((signed char)'(')
#define R_BRA ((signed char)')')
#define OP_ALT ((signed char)'|')
#define OP_CLO ((signed char)'*')
#define OP_PLS ((signed char)'+')
#define OP_OPT ((signed char)'?')
#define ESCAPE ((signed char)'\\')
#define C_ANY ((signed char)'.')

enum : signed char {
    OP_CON = 1,
};

static signed char unary_operator_array[] = { OP_CLO, OP_PLS, OP_OPT };
// the later ones have higher priority
static signed char binary_operator_array[] = { OP_ALT, OP_CON };
static bool unary_operator_map[256] = {};
static signed char binary_operator_map[256] = {};

static bool is_unary_operator(signed char operator) {
    if (operator < 0) {
        return false;
    }
    static bool inited = false;
    if (!inited) {
        for (int iter = 0; iter < (int)sizeof(unary_operator_array); ++iter) {
            unary_operator_map[unary_operator_array[iter]] = true;
        }
        inited = true;
    }
    return unary_operator_map[operator];
}

static bool is_binary_operator(signed char operator) {
    if (operator < 0) {
        return false;
    }
    static bool inited = false;
    if (!inited) {
        for (int iter = 0; iter < (int)sizeof(binary_operator_array); ++iter) {
            binary_operator_map[binary_operator_array[iter]] = iter + 1;
        }
        inited = true;
    }
    return binary_operator_map[operator];
}

static bool is_operator(signed char operator) {
    return is_unary_operator(operator) || is_binary_operator(operator);
}

static signed char priority(signed char operator) {
    if (!is_binary_operator(operator)) {
        return 0;
    }
    return binary_operator_map[operator];
}

// Convert the regex to postfix.
// keep the original, allocate and return a new string.
static signed char *convert_regex(char const *str) {
    // handle escape characters
    signed char trimed_str[strlen(str) + 1];
    int index = 0;
    for (signed char const *iter = (void *)str; *iter != '\0'; ++iter) {
        if (*iter < 32 || *iter > 126) {
            return nullptr;
        } else if (*iter == ESCAPE) {
            ++iter;
            if (*iter == '\0') {
                return nullptr;
            }
            trimed_str[index] = -*iter;
        } else {
            trimed_str[index] = *iter;
        }
        ++index;
    }
    trimed_str[index] = '\0';

    // check if trimed_str is a valid regex
    int len = 0;
    int lbr = 0;
    int br = 0;
    signed char last_symbol = START_SYMBOL;
    for (signed char const *iter = trimed_str; *iter != '\0'; ++iter) {
        if (is_operator(*iter) || *iter == R_BRA) {
            if (last_symbol == START_SYMBOL || last_symbol == L_BRA
                || is_binary_operator(last_symbol)) {
                return nullptr;
            }
            if (*iter == R_BRA) {
                --lbr;
                ++br;
                if (lbr < 0) {
                    return nullptr;
                }
            }
        } else {
            if (last_symbol != START_SYMBOL && last_symbol != L_BRA
                && !is_binary_operator(last_symbol)) {
                // need to insert an OP_CON
                ++len;
            }
            if (*iter == L_BRA) {
                ++lbr;
                ++br;
            }
        }
        last_symbol = *iter;
        ++len;
    }
    if (lbr > 0) {
        return nullptr;
    }

    // insert implicit OP_CON
    signed char completed_str[len + 1];
    index = 0;
    last_symbol = START_SYMBOL;
    for (signed char const *iter = trimed_str; *iter != '\0'; ++iter) {
        if (!is_operator(*iter) && *iter != R_BRA
            && last_symbol != START_SYMBOL && last_symbol != L_BRA
            && !is_binary_operator(last_symbol)) {
            completed_str[index] = OP_CON;
            ++index;
        }
        completed_str[index] = *iter;
        ++index;
        last_symbol = *iter;
    }
    completed_str[index] = '\0';

    // convert
    signed char *new_str = malloc(len - br + 1);
    index = 0;
    struct Stack *operator_stack = stack_create();
    for (signed char const *iter = completed_str; *iter != '\0'; ++iter) {
        if (*iter == R_BRA || is_binary_operator(*iter)) {
            // lowest priority for R_BRA
            int pri = 0;
            if (is_binary_operator(*iter)) {
                pri = priority(*iter);
            }
            while (!stack_empty(operator_stack)
                   && *(signed char const *)stack_top(operator_stack) != L_BRA
                   && priority(*(signed char const *)stack_top(operator_stack)) >= pri) {
                new_str[index] = *(signed char const *)stack_pop(operator_stack);
                ++index;
            }
            if (*iter == R_BRA) {
                stack_pop(operator_stack);
            } else {
                stack_push(operator_stack, (void *)iter);
            }
        } else if (*iter == L_BRA) {
            stack_push(operator_stack, (void *)iter);
        } else {
            new_str[index] = *iter;
            ++index;
        }
    }
    while (!stack_empty(operator_stack)) {
        new_str[index] = *(signed char const *)stack_pop(operator_stack);
        ++index;
    }
    new_str[index] = '\0';
    stack_free(operator_stack);
    return new_str;
}

static struct Automaton *create_nfa_unit(signed char input) {
    struct AutomatonState *start = automaton_create_state();
    struct AutomatonState *end = automaton_create_state();
    if (input == C_ANY) {
        input = AUTOMATON_EDGE_ANY;
    } else if (input < 0) {
        input = -input;
    }
    automaton_state_transfer(start, end, input);
    end->accept = true;
    struct Automaton *new_nfa = malloc(sizeof(struct Automaton));
    new_nfa->start = start;
    new_nfa->end = end;
    new_nfa->states = list_create();
    list_append(new_nfa->states, start);
    list_append(new_nfa->states, end);
    return new_nfa;
}

// For all regular operations, if there is only one argument, it is modified in place.
// If there are two arguments, the left is modified to store the result.
// States of the right are reused while the right itself is freed.

static void nfa_concatenate(struct Automaton *left, struct Automaton *right) {
    if (left == nullptr || right == nullptr) {
        return;
    }
    left->end->accept = false;
    automaton_state_transfer(left->end, right->start, AUTOMATON_EDGE_EMPTY);
    left->end = right->end;
    list_concatenate(left->states, right->states);
    free(right);
}

static void nfa_alternate(struct Automaton *left, struct Automaton *right) {
    if (left == nullptr || right == nullptr) {
        return;
    }
    struct AutomatonState *start = automaton_create_state();
    struct AutomatonState *end = automaton_create_state();
    left->end->accept = false;
    right->end->accept = false;
    end->accept = true;
    automaton_state_transfer(start, left->start, AUTOMATON_EDGE_EMPTY);
    automaton_state_transfer(start, right->start, AUTOMATON_EDGE_EMPTY);
    automaton_state_transfer(left->end, end, AUTOMATON_EDGE_EMPTY);
    automaton_state_transfer(right->end, end, AUTOMATON_EDGE_EMPTY);
    list_concatenate(left->states, right->states);
    free(right);
    list_append(left->states, start);
    list_append(left->states, end);
    left->start = start;
    left->end = end;
}

static void nfa_closure(struct Automaton *nfa) {
    if (nfa == nullptr) {
        return;
    }
    struct AutomatonState *start = automaton_create_state();
    struct AutomatonState *end = automaton_create_state();
    nfa->end->accept = false;
    end->accept = true;
    automaton_state_transfer(start, nfa->start, AUTOMATON_EDGE_EMPTY);
    automaton_state_transfer(nfa->end, end, AUTOMATON_EDGE_EMPTY);
    automaton_state_transfer(start, end, AUTOMATON_EDGE_EMPTY);
    automaton_state_transfer(nfa->end, nfa->start, AUTOMATON_EDGE_EMPTY);
    list_append(nfa->states, start);
    list_append(nfa->states, end);
    nfa->start = start;
    nfa->end = end;
}

static void nfa_plus(struct Automaton *nfa) {
    if (nfa == nullptr) {
        return;
    }
    struct AutomatonState *start = automaton_create_state();
    struct AutomatonState *end = automaton_create_state();
    nfa->end->accept = false;
    end->accept = true;
    automaton_state_transfer(start, nfa->start, AUTOMATON_EDGE_EMPTY);
    automaton_state_transfer(nfa->end, end, AUTOMATON_EDGE_EMPTY);
    automaton_state_transfer(nfa->end, nfa->start, AUTOMATON_EDGE_EMPTY);
    list_append(nfa->states, start);
    list_append(nfa->states, end);
    nfa->start = start;
    nfa->end = end;
}

static void nfa_opt(struct Automaton *nfa) {
    if (nfa == nullptr) {
        return;
    }
    automaton_state_transfer(nfa->start, nfa->end, AUTOMATON_EDGE_EMPTY);
}

static void nfa_operate(struct Automaton *left, struct Automaton *right, signed char operator) {
    switch (operator) {
    case OP_CON:
        nfa_concatenate(left, right);
        break;
    case OP_ALT:
        nfa_alternate(left, right);
        break;
    case OP_CLO:
        nfa_closure(left);
        break;
    case OP_PLS:
        nfa_plus(left);
        break;
    case OP_OPT:
        nfa_opt(left);
        break;
    }
}

struct Automaton *automaton_generate_nfa(char const *str) {
    signed char const *regex = convert_regex(str);
    if (regex == nullptr) {
        return nullptr;
    }
    if (*regex == '\0') {
        struct AutomatonState *state = automaton_create_state();
        state->accept = true;
        state->edges = list_create();
        struct Automaton *nfa = malloc(sizeof(struct Automaton));
        nfa->start = state;
        nfa->end = state;
        nfa->states = list_create();
        list_append(nfa->states, state);
        return nfa;
    }
    struct Stack *operand_stack = stack_create();
    for (signed char const *iter = regex; *iter != '\0'; ++iter) {
        if (is_unary_operator(*iter)) {
            nfa_operate(stack_top(operand_stack), nullptr, *iter);
        } else if (is_binary_operator(*iter)) {
            struct Automaton *right = stack_pop(operand_stack);
            struct Automaton *left = stack_top(operand_stack);
            nfa_operate(left, right, *iter);
        } else {
            stack_push(operand_stack, create_nfa_unit(*iter));
        }
    }
    free((void *)regex);
    struct Automaton *result = stack_pop(operand_stack);
    stack_free(operand_stack);
    return result;
}
