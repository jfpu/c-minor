#include <stdlib.h> // malloc
#include <string.h> // memset
#include "utility.h"
#include "decl.h"
#include "expr.h"
#include "stmt.h"
#include "type.h"

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
    if (!d) return;

    // indent
    _print_indent(indent);

    printf("%s: ", d->name);
    type_print(d->type);
    if (d->value) {
        printf(" = ");
        if (d->type->kind == TYPE_ARRAY) printf("{");
        expr_print(d->value);
        if (d->type->kind == TYPE_ARRAY) printf("}");
        printf(";\n");
    } else if (d->code) {
        printf(" = {\n");
        stmt_print(d->code, indent + 1);
        printf("}\n");
    } else {
        printf(";\n");
    }

    if (d -> next) decl_print(d->next, indent);
}
