#include <stdlib.h> // malloc
#include <string.h> // memset
#include "type.h"

struct type *type_create(type_kind_t kind, struct param_list *params, struct type *subtype) {
    struct type *t = (struct type *)malloc(sizeof(*t));
    memset(t, 0, sizeof(*t));

    t->kind = kind;
    t->params = params;
    t->subtype = subtype;
    return t;
}

struct type *type_create_array(struct expr *size, struct type *subtype) {
    struct type *t = type_create(TYPE_ARRAY, NULL, subtype);
    t->size = size;
    return t;
}

void type_print(struct type *t) {
    if (!t) return;

    switch (t->kind) {
        case TYPE_BOOLEAN:
            printf("boolean");
            break;

        case TYPE_CHARACTER:
            printf("char");
            break;

        case TYPE_INTEGER:
            printf("integer");
            break;

        case TYPE_STRING:
            printf("string");
            break;

        case TYPE_ARRAY:
            printf("array [");
            expr_print(t->size);
            printf("] ");
            type_print(t->subtype);
            break;

        case TYPE_FUNCTION:
            printf("function ");
            type_print(t->subtype);
            printf(" (");
            if (t->params) {
                printf(" ");
                param_list_print(t->params);
                printf(" ");
            }
            printf(")");
            break;

        case TYPE_VOID:
            printf("void");
            break;
    }
}

// for type checking
struct type *type_copy(struct type *t) {
    if (!t) return NULL;
    struct type *new_type = type_create(t->kind, param_list_copy(t->params), type_copy(t->subtype));
    return new_type;
}

void type_delete(struct type *t) {
    if (!t) return;
    param_list_delete(t->params);
    type_delete(t->subtype);
    free(t);
}

int type_is_equal(struct type *a, struct type *b) {
    if (!a || !b) {
        // ideally this should never happen
        fprintf(stderr, "type_is_equal received NULL types\n");
        return 0;
    }

    if (a->kind != b->kind) return 0;
    if ((a->kind == TYPE_ARRAY || a->kind == TYPE_FUNCTION)
        && !type_is_equal(a->subtype, b->subtype)) return 0;

    return 1;
}

// actual type checking funcitons

void array_type_typecheck(struct type *t, const char * const name) {
    // array length must be present and constant and positive
    if (!t->size) {
        ++error_count_type;
        printf("type error: declaring array `%s` without size\n", name);
    } else if (!expr_is_constant(t->size)) {
        ++error_count_type;
        printf("type error: declaring array `%s` with non-constant size `", name);
        expr_print(t->size);
        printf("`\n");
    } else if (t->size->literal_value <= 0) {
        ++error_count_type;
        printf("type error: declaring array `%s` with non-positive size %d\n", name, t->size->literal_value);
    }

    // array subtype cannot be void or function
    if (t->subtype->kind == TYPE_VOID
        || t->subtype->kind == TYPE_FUNCTION) {
        ++error_count_type;
        printf("type error: declaring array `%s` of type ", name);
        type_print(t->subtype);
        printf("\n");
    } else if (t->subtype->kind == TYPE_ARRAY) {
        array_type_typecheck(t->subtype, name);
    }
}

void param_list_typecheck(struct param_list *p, struct expr *e, const char * const name) {
    // this is invoked for each function invocation
    // we compare each item in the param list with the given expression list

    struct param_list *p_ptr = p;
    struct expr *e_ptr = e;

    while (p_ptr && e_ptr) {
        // comparing each pair
        struct type *expected_type = p_ptr->type;
        struct type *received_type = expr_typecheck(e_ptr);
        if (!type_is_equal(expected_type, received_type)) {
            // error
            ++error_count_type;
            printf("type error: function `%s` parameter %d type mismatch; expected ",
                name,
                p_ptr->symbol->which);
            type_print(expected_type);
            printf(", received ");
            type_print(received_type);
            printf("\n");
        }
        TYPE_FREE(received_type);

        // move on
        p_ptr = p_ptr->next;
        e_ptr = e_ptr->next;
    }

    // ensure lengths are the same
    if (p_ptr != NULL || e_ptr != NULL) {
        ++error_count_type;
        printf("type error: function `%s` expected %u parameters, received %u arguments\n",
            name,
            param_list_length(p),
            expr_list_length(e));
    }
}
