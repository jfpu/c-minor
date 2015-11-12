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

// name resolution
void function_param_resolve(struct type *t);

// for type checking
struct type *type_copy(struct type *t);
void type_delete(struct type *t);
int type_is_equal(struct type *a, struct type *b);

#define TYPE_FREE(_type_obj)    \
    type_delete((_type_obj));   \
    (_type_obj) = NULL

// actual type checking functions
extern unsigned int error_count_type;
void array_type_typecheck(struct type *t, const char * const name);

#endif
