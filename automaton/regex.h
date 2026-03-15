#pragma once

struct Automaton;

extern struct Automaton *automaton_compile(char const *str);
extern bool automaton_match(struct Automaton *automaton, char const *str);
extern void automaton_destroy(struct Automaton *automaton);
