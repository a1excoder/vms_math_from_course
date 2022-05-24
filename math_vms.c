#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

#include "math_vms.h"

typedef void (*stack_func_type) (struct vm_state*);

static bool push(struct stack_list** list, int64_t data) {
    bool status = false;
    struct stack_list* new_node = (struct stack_list*)malloc(sizeof(struct stack_list));

    if (NULL != new_node) {
        status = true;
        new_node->data = data;
        new_node->next = *list;

        *list = new_node;
    }

    return status;
}

static struct result_data pop(struct stack_list** list) {
    struct result_data data = {.status = false};
    struct stack_list* temp_delete = NULL;

    if (NULL != *list) {
        data.status = true;
        data.data = (*list)->data;

        temp_delete = *list;
        *list = (*list)->next;

        free(temp_delete);
    }

    return data;
}

static void stack_destroy(struct vm_state* vs) {
    struct result_data res = {.status = true};

    while (res.status) {
        res = pop(&vs->stack);
        if (res.status) printf("destroy | pop(%" PRId64 ")\n", res.data);
    }
}

static void lift_unop( struct stack_list** s, int64_t (f)(int64_t)) {
    struct result_data res = pop(s);
    if (res.status) push(s, f(res.data));
}

static void lift_binop( struct stack_list** s, int64_t (f)(int64_t, int64_t)) {
    int64_t temp_i64 = 0, temp_i64_2 = 0;

    struct result_data res = pop(s);
    if (res.status) temp_i64 = res.data;
    else return;

    res = pop(s);
    if (res.status) temp_i64_2 = res.data;
    else return;

    push(s, f(temp_i64, temp_i64_2));
}

static int64_t i64_add(int64_t a, int64_t b) { return a + b; }
static int64_t i64_sub(int64_t a, int64_t b) { return a - b; }
static int64_t i64_mul(int64_t a, int64_t b) { return a * b; }
static int64_t i64_div(int64_t a, int64_t b) { return a / b; }
static int64_t i64_cmp(int64_t a, int64_t b) { if (a > b) return 1; else if (a < b) return -1; return 0; }

static int64_t i64_neg(int64_t a) { return -a; }

static void interpret_iadd( struct vm_state* state ) {
    lift_binop(&state->stack, i64_add);
    state->commands = state->commands + 1;
}

static void interpret_isub( struct vm_state* state ) {
    lift_binop(&state->stack, i64_sub);
    state->commands = state->commands + 1;
}

static void interpret_imul( struct vm_state* state ) {
    lift_binop(&state->stack, i64_mul);
    state->commands = state->commands + 1;
}
static void interpret_idiv( struct vm_state* state ) {
    lift_binop(&state->stack, i64_div);
    state->commands = state->commands + 1;
}
static void interpret_icmp( struct vm_state* state ) {
    lift_binop(&state->stack, i64_cmp);
    state->commands = state->commands + 1;
}

static void interpret_ineg( struct vm_state* state) {
    lift_unop (& state->stack, i64_neg);
    state->commands = state->commands + 1;
}

static void interpret_push(struct vm_state* state) {
    push(&state->stack, state->commands->as_arg64.arg);
    state->commands = state->commands + 1;
}

static void interpret_iprint(struct vm_state* state) {
    struct result_data temp_res = pop(&state->stack);
    if (temp_res.status) {
        printf("%" PRId64 "\n", temp_res.data);
        push(&state->stack, temp_res.data);
    }

    state->commands = state->commands + 1;
}

static void interpret_iread(struct vm_state* state) {
    int64_t temp_i64;
    scanf_s("%" SCNd64, &temp_i64);
    push(&state->stack, temp_i64);

    state->commands = state->commands + 1;
}

static void interpret_swap(struct vm_state* state) {
    int64_t temp_i64 = 0, temp_i64_2 = 0;
    struct result_data res = pop(&state->stack);
    if (res.status) temp_i64 = res.data;
    else return;

    res = pop(&state->stack);
    if (res.status) temp_i64_2 = res.data;
    else return;

    push(&state->stack, temp_i64);
    push(&state->stack, temp_i64_2);

    state->commands = state->commands + 1;
}

static void interpret_dup(struct vm_state* state) {
    struct result_data res = pop(&state->stack);
    if (res.status) {
        push(&state->stack, res.data);
        push(&state->stack, res.data);
    }
}

static bool check_stack_values(size_t values, enum opcode this_opcode) {
    switch (this_opcode) {
        case BC_IADD:
        case BC_ISUB:
        case BC_IMUL:
        case BC_IDIV:
        case BC_ICMP:
        case BC_SWAP:
            if (values < 2) return false;
            return true;
        case BC_IPRINT:
        case BC_INEG:
        case BC_DUP:
            if (values < 1) return false;
            return true;
        default:
            return true;
    }
}

static stack_func_type my_funcs[] = {
    [BC_PUSH] = interpret_push,
    [BC_IPRINT] = interpret_iprint,
    [BC_IREAD] = interpret_iread,
    [BC_IADD] = interpret_iadd,
    [BC_ISUB] = interpret_isub,
    [BC_IMUL] = interpret_imul,
    [BC_IDIV] = interpret_idiv,
    [BC_ICMP] = interpret_icmp,
    [BC_INEG] = interpret_ineg,
    [BC_SWAP] = interpret_swap,
    [BC_DUP] = interpret_dup
};

uint8_t interpret(struct vm_state data) {
    int64_t temp_i64 = 0, temp_i64_2 = 0;
    size_t stack_values = 0;

    for (;;) {
        if (!check_stack_values(stack_values, data.commands->opcode)) {
            stack_destroy(&data);
            return 1;
        }

        switch (data.commands->opcode) {
        case BC_PUSH:
            my_funcs[BC_PUSH](&data);
            stack_values++;
            break;
        case BC_IPRINT:
            my_funcs[BC_IPRINT](&data);
            break;
        case BC_IREAD:
            my_funcs[BC_IREAD](&data);
            stack_values++;
            break;
        case BC_IADD:
            my_funcs[BC_IADD](&data);
            stack_values--;
            break;
        case BC_ISUB:
            my_funcs[BC_ISUB](&data);
            stack_values--;
            break;
        case BC_IMUL:
            my_funcs[BC_IMUL](&data);
            stack_values--;
            break;
        case BC_IDIV:
            my_funcs[BC_IDIV](&data);
            stack_values--;
            break;
        case BC_ICMP:
            my_funcs[BC_ICMP](&data);
            stack_values--;
            break;
        case BC_INEG:
            my_funcs[BC_INEG](&data);
            break;
        case BC_SWAP:
            my_funcs[BC_SWAP](&data);
        case BC_DUP:
            my_funcs[BC_DUP](&data);
            stack_values++;
        case BC_STOP:
            stack_destroy(&data);
            return 0;
        default:
            break;
        }
    }
    
    return 0;
}