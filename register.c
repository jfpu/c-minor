#include "register.h"

// we'll keep track of allocation status of all 16 registers
int register_allocation_table[16] = {
    //  rax rbx rcx rdx rsi rdi rsp rbp
        1,  0,  1,  1,  1,  1,  1,  1,
    //  r8  r9  r10 r11 r12 r13 r14 r15
        1,  1,  0,  0,  0,  0,  0,  0
};

const char *register_name(int r) {
    static const char *register_name_table[16] = {
        "%%rax", "%%rbx", "%%rcx", "%%rdx",
        "%%rsi", "%%rdi", "%%rsp", "%%rbp",
        "%%r8",  "%%r9",  "%%r10", "%%r11",
        "%%r12", "%%r13", "%%r14", "%%r15"
    };

    if (r >= 0 && r < 16) {
        return register_name_table[r];
    } else {
        return "error";
    }
}

int register_alloc() {

}

void register_free(int r) {

}
