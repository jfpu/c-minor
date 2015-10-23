#include <stdlib.h> // malloc
#include <string.h> // memset
#include "stmt.h"

struct stmt *stmt_create(stmt_kind_t kind, struct decl *d, struct expr *init_expr, struct expr *e, struct expr *next_expr, struct stmt *body, struct stmt *else_body) {
    struct stmt *s = (struct stmt *)malloc(sizeof(*s));
    memset(s, 0, sizeof(*s));

    s->kind = kind;
    s->decl = d;
    s->init_expr = init_expr;
    s->expr = e;
    s->next_expr = next_expr;
    s->body = body;
    s->else_body = else_body;
    s->next = NULL;
    return s;
}

struct stmt *stmt_list_prepend(struct stmt *first, struct stmt *rest) {
    first->next = rest;
    return first;
}

void stmt_print(struct stmt *s, int indent) {

}
