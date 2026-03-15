#pragma once

typedef struct Stack Stack;

extern Stack *stack_create();
extern bool stack_empty(Stack *stack);
extern void *stack_top(Stack *stack);
extern void *stack_pop(Stack *stack);
extern void stack_push(Stack *stack, void *data);
extern void stack_free(Stack *stack);
