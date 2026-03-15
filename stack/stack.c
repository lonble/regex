#include "stack/stack.h"

#include <stdlib.h>

typedef struct Node Node;

struct Node {
    void *data;
    Node *last;
};

struct Stack {
    Node *top;
};

Stack *stack_create() {
    Stack *new_stack = malloc(sizeof(Stack));
    new_stack->top = nullptr;
    return new_stack;
}

bool stack_empty(Stack *stack) {
    if (stack->top == nullptr) {
        return true;
    }
    return false;
}

void *stack_top(Stack *stack) {
    if (stack_empty(stack)) {
        return nullptr;
    }
    return stack->top->data;
}

void *stack_pop(Stack *stack) {
    if (stack_empty(stack)) {
        return nullptr;
    }
    Node *top = stack->top;
    void *result = top->data;
    stack->top = top->last;
    free(top);
    return result;
}

void stack_push(Stack *stack, void *data) {
    Node *new_node = malloc(sizeof(Node));
    new_node->data = data;
    new_node->last = stack->top;
    stack->top = new_node;
}

void stack_free(Stack *stack) {
    while (!stack_empty(stack)) {
        stack_pop(stack);
    }
    free(stack);
}
