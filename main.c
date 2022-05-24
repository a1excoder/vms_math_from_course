#include <stdio.h>

#include "math_vms.h"

const union instruction ins[] = {
        {.as_arg64 = {.opcode = BC_PUSH, .arg = 13}},
        {BC_IREAD},
        {BC_IADD},
        {.as_arg64 = {.opcode = BC_PUSH, .arg = 8}},
        {BC_IADD},
        {BC_IPRINT},
        {BC_STOP}
    };

int main(int argc, char const *argv[])
{
    struct vm_state data_main = {.commands = ins};
    if (interpret(data_main)) puts("stack underflow");
    return 0;
}
