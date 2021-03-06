#ifndef EXPR_H
#define EXPR_H

#include "type.h"
#include <stdio.h>

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

    /* for codegen */
    int reg;
};

struct expr *expr_create(expr_t kind, struct expr *left, struct expr *right);
struct expr *expr_list_prepend(struct expr *first, struct expr *rest);

struct expr *expr_create_name(const char *n);
struct expr *expr_create_boolean_literal(int c);
struct expr *expr_create_integer_literal(int c);
struct expr *expr_create_character_literal(int c);
struct expr *expr_create_string_literal(const char *str);

void expr_print(struct expr *e);
void expr_print_individual(struct expr *e);

// name resolution
void expr_resolve(struct expr *e);

// for type checking
unsigned int expr_list_length(struct expr *e);
int expr_is_constant(struct expr *e);
int expr_is_lvalue_type(struct expr *e);
struct type *expr_typecheck(struct expr *e);
void expr_list_typecheck(struct expr *e, struct type *expected);

// for codegen
extern int label_count;
void expr_codegen(struct expr *e, FILE *file);

void expr_string_print(const char * const str, FILE *file);

#endif
