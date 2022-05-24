#ifndef MATH_VMS_H
#define MATH_VMS_H
#include <inttypes.h>
#include <stdbool.h>

enum opcode { BC_PUSH, BC_IPRINT, BC_IREAD, BC_IADD, BC_ISUB, BC_IMUL, BC_IDIV, BC_ICMP, BC_INEG, BC_SWAP, BC_DUP, BC_STOP };

struct stack_list {
    int64_t data;
    struct stack_list* next;
};

struct bc_arg64 {
    enum opcode opcode;
    int64_t arg;
};

union instruction {
    enum opcode opcode;
    struct bc_arg64 as_arg64;
};

struct vm_state {
    const union instruction* commands;
    struct stack_list* stack;
};

struct result_data {
    int64_t data;
    bool status;
};

uint8_t interpret(struct vm_state);
#endif // MATH_VMS_H