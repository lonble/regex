#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "list/list.h"
#include "stack/stack.h"
#include "queue/queue.h"
#include "set/set.h"
#include "automaton/nfa.h"
#include "automaton/dfa.h"
#include "automaton/convert.h"

static void test_list();
static void test_stack();
static void test_queue();
static void test_set();
static void test_nfa();

void test() {
    test_list();
    test_stack();
    test_queue();
    test_set();
    test_nfa();
}

static void test_list() {
    List *l1 = list_create();
    List *l2 = list_create();
    list_append(l1, &(char) { 'a' });
    list_append(l1, &(char) { 'b' });
    list_append(l2, &(char) { 'c' });
    list_append(l2, &(char) { 'd' });
    list_concatenate(l1, l2);
    char buff[5] = {};
    int index = 0;
    ListIterator *iter = list_create_iterator(l1);
    while (!list_iterator_end(iter)) {
        buff[index] = *(char *)list_iterate(iter);
        ++index;
        assert(index < 5);
    }
    list_free_iterator(iter);
    assert(strcmp(buff, "abcd") == 0);
    list_free(l1);
}

static void test_stack() {
    Stack *s = stack_create();
    stack_push(s, &(char) { 'a' });
    stack_push(s, &(char) { 'b' });
    stack_push(s, &(char) { 'c' });
    stack_push(s, &(char) { 'd' });
    assert(*(char *)stack_top(s) == 'd');
    char buff[5] = {};
    int index = 0;
    while (!stack_empty(s)) {
        buff[index] = *(char *)stack_pop(s);
        ++index;
        assert(index < 5);
    }
    assert(strcmp(buff, "dcba") == 0);
    stack_free(s);
}

static void test_queue() {
    Queue *q = queue_create();
    queue_enqueue(q, &(char) { 'a' });
    queue_enqueue(q, &(char) { 'b' });
    queue_enqueue(q, &(char) { 'c' });
    queue_enqueue(q, &(char) { 'd' });
    assert(*(char *)queue_head(q) == 'a');
    char buff[5] = {};
    int index = 0;
    while (!queue_empty(q)) {
        buff[index] = *(char *)queue_dequeue(q);
        ++index;
        assert(index < 5);
    }
    assert(strcmp(buff, "abcd") == 0);
    queue_free(q);
}

static bool test_set_equal(void *left, void *right, [[maybe_unused]]void *context) {
    if (*(char *)left == *(char *)right) {
        return true;
    }
    return false;
}

static void test_set() {
    Set *s1 = set_create(test_set_equal, nullptr);
    assert(set_add(s1, &(char) { 'a' }));
    assert(set_add(s1, &(char) { 'c' }));
    assert(set_add(s1, &(char) { 'b' }));
    assert(set_add(s1, &(char) { 'd' }));
    assert(!set_add(s1, &(char) { 'b' }));
    assert(!set_add(s1, &(char) { 'c' }));
    char buff[5] = {};
    int index = 0;
    SetIterator *iter = set_create_iterator(s1);
    while (!set_iterator_end(iter)) {
        buff[index] = *(char *)set_iterate(iter);
        ++index;
        assert(index < 5);
    }
    set_free_iterator(iter);
    assert(strcmp(buff, "acbd") == 0);
    
    set_free(s1);
}

static void print_automaton(struct Automaton *automaton) {
    ListIterator *state_iterator = list_create_iterator(automaton->states);
    while (!list_iterator_end(state_iterator)) {
        struct AutomatonState *state = list_iterate(state_iterator);
        printf("State %d:", state->id);
        if (state == automaton->start) {
            fputs(" start", stdout);
        }
        if (state->accept) {
            fputs(" accept", stdout);
        }
        putchar('\n');
        ListIterator *edge_iterator = list_create_iterator(state->edges);
        while (!list_iterator_end(edge_iterator)) {
            struct AutomatonEdge *edge = list_iterate(edge_iterator);
            char ch = edge->input;
            if (ch == AUTOMATON_EDGE_EMPTY) {
                ch = ' ';
            } else if (ch == AUTOMATON_EDGE_ANY) {
                ch = '*';
            }
            printf("    %c -> State %d\n", ch, edge->dest->id);
        }
        list_free_iterator(edge_iterator);
    }
    list_free_iterator(state_iterator);
}

static void test_nfa() {
    char buff[100] = {};
    fgets(buff, 100, stdin);
    buff[strlen(buff) - 1] = '\0';
    struct Automaton *nfa = automaton_generate_nfa(buff);
    puts("NFA:");
    print_automaton(nfa);
    puts("\nMap:");
    struct Automaton *dfa = automaton_generate_dfa_from_nfa(nfa);
    puts("\nDFA:");
    print_automaton(dfa);
    struct Automaton *minimized_dfa = automaton_generate_minimized_dfa(dfa);
    puts("\nMinimized DFA:");
    print_automaton(minimized_dfa);
    automaton_free(nfa);
    automaton_free(dfa);
    automaton_free(minimized_dfa);
}
