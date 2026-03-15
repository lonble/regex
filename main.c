#include <stdio.h>
#include <string.h>

#include "automaton/regex.h"

int main() {
    char input[1000];

    // get regular expression from user input
    fputs("regex: ", stdout);
    if (fgets(input, sizeof(input), stdin) == nullptr) {
        putchar('\n');
        return 1;
    }
    input[strlen(input) - 1] = '\0';
    struct Automaton *automaton = automaton_compile(input);
    while (automaton == nullptr) {
        puts("invalid regular expression");
        fputs("regex: ", stdout);
        if (fgets(input, sizeof(input), stdin) == nullptr) {
            putchar('\n');
            return 1;
        }
        input[strlen(input) - 1] = '\0';
        automaton = automaton_compile(input);
    }

    // match texts from user input
    while (true) {
        fputs("text: ", stdout);
        if (fgets(input, sizeof(input), stdin) == nullptr) {
            putchar('\n');
            break;
        }
        input[strlen(input) - 1] = '\0';
        automaton_match(automaton, input) ? puts("true") : puts("false");
    }

    automaton_destroy(automaton);
    return 0;
}

// extern void test();
// int main() {
//     test();
// }
