#ifndef EXPR_H
#define EXPR_H

#include "symbol.h"

typedef enum {
    EXPR_NAME,
    EXPR_BOOLEAN,
    EXPR_INTEGER,
    EXPR_CHARACTER,
    EXPR_STRING,
    EXPR_ASSIGN,
    EXPR_ADD,
    EXPR_SUB,
    EXPR_MUL,
    EXPR_DIV,
    EXPR_EXP,
    EXPR_MOD,
    EXPR_INC,
    EXPR_DEC,
    EXPR_NEG,
    EXPR_LAND,
    EXPR_LOR,
    EXPR_LNOT,
    EXPR_LT,
    EXPR_LE,
    EXPR_GT,
    EXPR_GE,
    EXPR_EQ,
    EXPR_NE,
    EXPR_FCALL,
    EXPR_ARRAY_DEREF
} expr_t;

struct expr {
    /* used by all expr types */
    expr_t kind;
    struct expr *left;
    struct expr *right;

    /* for expression lists */
    struct expr *next;

    /* used by leaf expr types */
    const char *name;
    struct symbol *symbol;
    int literal_value;
    const char *string_literal;
    struct expr *params;
};

struct expr *expr_create(expr_t kind, struct expr *left, struct expr *right);
struct expr *expr_list_prepend(struct expr *first, struct expr *rest);

struct expr *expr_create_name(const char *n);
struct expr *expr_create_boolean_literal(int c);
struct expr *expr_create_integer_literal(int c);
struct expr *expr_create_character_literal(int c);
struct expr *expr_create_string_literal(const char *str);
struct expr *expr_create_function_call(const char *name, struct expr *params);
struct expr *expr_create_incdec(expr_t kind, const char *name);

void expr_print(struct expr *e);

#endif
