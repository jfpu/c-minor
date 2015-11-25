#include <stdlib.h> // malloc
#include <string.h> // memset
#include "symbol.h"

struct symbol *symbol_create(symbol_t kind, int which, struct type *type, char *name) {
    struct symbol *s = (struct symbol *)malloc(sizeof(*s));
    memset(s, 0, sizeof(*s));

    s->kind = kind;
    s->which = which;
    s->type = type;
    s->name = name;
    s->param_count = 0;
    s->local_count = 0;
    return s;
}

char *symbol_code(struct symbol *s) {

}
