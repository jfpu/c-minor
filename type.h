#ifndef TYPE_H
#define TYPE_H

#include "param_list.h"
#include "expr.h"
#include "stmt.h"
#include "decl.h"
#include "param_list.h"
#include "type.h"

typedef enum {
    TYPE_BOOLEAN,
    TYPE_CHARACTER,
    TYPE_INTEGER,
    TYPE_STRING,
    TYPE_ARRAY,
    TYPE_FUNCTION,
    TYPE_VOID
} type_kind_t;

struct type {
    type_kind_t kind;
    struct param_list *params;
    struct type *subtype;
    struct expr *size;
};

struct type *type_create(type_kind_t kind, struct param_list *params, struct type *subtype);
struct type *type_create_array(struct expr *size, struct type *subtype);
void type_print(struct type *t);

// for type checking
struct type *type_copy(struct type *t);
void type_delete(struct type *t);
int type_is_equal(struct type *a, struct type *b);

// actual type checking functions
extern unsigned int error_count_type;

struct type *expr_typecheck(struct expr *e);
void stmt_typecheck(struct stmt *s, struct type *expected);
void decl_typecheck(struct decl *d);
void param_list_typecheck(struct param_list *p, struct expr *e);

#endif
