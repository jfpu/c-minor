#include <stdlib.h> // malloc
#include <string.h> // memset
#include "decl.h"

struct decl *decl_create(char *name, struct type *t, struct expr *v, struct stmt *c, struct decl *next) {
    struct decl *d = (struct decl *)malloc(sizeof(*d));
    memset(d, 0, sizeof(*d));

    d->name = name;
    d->type = t;
    d->value = v;
    d->code = c;
    d->next = next;
    return d;
}

struct decl *decl_list_prepend(struct decl *first, struct decl *rest) {
    first->next = rest;
    return first;
}

void decl_print(struct decl *d, int indent) {

}
