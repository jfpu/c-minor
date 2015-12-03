#include <stdlib.h> // malloc
#include <string.h> // memset
#include "symbol.h"
#include <math.h>   // log10, ceil

struct symbol *symbol_create(symbol_t kind, int which, struct type *type, char *name) {
    struct symbol *s = (struct symbol *)malloc(sizeof(*s));
    memset(s, 0, sizeof(*s));

    s->kind = kind;
    s->which = which;
    s->type = type;
    s->name = name;
    s->param_count = 0;
    s->local_count = 0;
    s->is_prototype_only = 1;
    return s;
}

char *symbol_code(struct symbol *s) {
    // for variables only, emits code that refers to the symbol
    if (!s) {
        fprintf(stderr, "calling symbol_code with NULL symbol\n");
        exit(1);
    }

    switch (s->kind) {
        case SYMBOL_GLOBAL: {
            // globals are referred to by label
            // need relative addressing
            int name_len = strlen(s->name);
            char *str = (char *)malloc(name_len + 6 * sizeof(char));
            sprintf(str, "%s(%%rip)", s->name);
            return str;
        }
        case SYMBOL_LOCAL: {
            // locals are accessed with offset(%rbp)
            int offset = 8 + s->param_count * 8 + s->which * 8;
            int num_len = (int)ceil(log10(offset));
            char *str = (char *)malloc(num_len + 8 * sizeof(char));
            sprintf(str, "-%d(%%rbp)", offset);
            return str;
        }
        case SYMBOL_PARAM: {
            // params are accessed with offset(%rbp) too
            int offset = 8 + s->which * 8;
            int num_len = (int)ceil(log10(offset));
            char *str = (char *)malloc(num_len + 8 * sizeof(char));
            sprintf(str, "-%d(%%rbp)", offset);
            return str;
        }
    }
}
