#ifndef SYMBOL_H
#define SYMBOL_H

#include "type.h"

typedef enum {
    SYMBOL_LOCAL,
    SYMBOL_PARAM,
    SYMBOL_GLOBAL
} symbol_t;

struct symbol {
    symbol_t kind;
    int which;
    struct type *type;
    char *name;

    // for functions
    int param_count;
    int local_count;
    int is_prototype_only;
};

struct symbol *symbol_create(symbol_t kind, int which, struct type *type, char *name);

// for codegen
char *symbol_code(struct symbol *s);

#endif
