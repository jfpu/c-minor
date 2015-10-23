#include <stdlib.h> // malloc
#include <string.h> // memset
#include "expr.h"

struct expr *expr_create(expr_t kind, struct expr *left, struct expr *right) {
    struct expr *e = (struct expr *)malloc(sizeof(*e));
    memset(e, 0, sizeof(*e));

    e->kind = kind;
    e->left = left;
    e->right = right;
    return e;
}

struct expr *expr_list_prepend(struct expr *first, struct expr *rest) {
    first->next = rest;
    return first;
}

struct expr *expr_create_name(const char *n) {
    struct expr *e = expr_create(EXPR_NAME, NULL, NULL);
    e->name = n;
    return e;
}

struct expr *expr_create_boolean_literal(int c) {
    struct expr *e = expr_create(EXPR_BOOLEAN, NULL, NULL);
    e->literal_value = c;
    return e;
}

struct expr *expr_create_integer_literal(int c) {
    struct expr *e = expr_create(EXPR_INTEGER, NULL, NULL);
    e->literal_value = c;
    return e;
}

struct expr *expr_create_character_literal(int c) {
    struct expr *e = expr_create(EXPR_CHARACTER, NULL, NULL);
    e->literal_value = c;
    return e;
}

struct expr *expr_create_string_literal(const char *str) {
    struct expr *e = expr_create(EXPR_STRING, NULL, NULL);
    e->string_literal = str;
    return e;
}

void expr_print(struct expr *e) {

}
