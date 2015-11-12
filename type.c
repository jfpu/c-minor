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


void decl_typecheck(struct decl *d) {
    if (!d) return;

    // declaration
    if (d->type->kind == TYPE_VOID) {
        // declared type cannot be void
        ++error_count_type;
        printf("type error: declaring variable `%s` with type ", d->name);
        type_print(d->type);
        printf("\n");

    } else if (d->type->kind == TYPE_ARRAY) {
        array_type_typecheck(d->type, d->name);

    } else if (d->type->kind == TYPE_FUNCTION) {
        // function cannot return arrays or functions
        // array subtype cannot be void or function
        if (d->type->subtype->kind == TYPE_ARRAY
            || d->type->subtype->kind == TYPE_FUNCTION) {
            ++error_count_type;
            printf("type error: declaring function `%s` with return type ", d->name);
            type_print(d->type->subtype);
            printf("\n");
        }
    }

    // initialization
    if (d->value) {
        // value type must match declared type
        struct type *value_type = expr_typecheck(d->value);
        if (d->type->kind != TYPE_ARRAY
            && !type_is_equal(d->type, value_type)) {
            ++error_count_type;
            printf("type error: initializing variable `%s` with type ", d->name);
            type_print(value_type);
            printf(", expecting ");
            type_print(d->type);
            printf("\n");
        }
        TYPE_FREE(value_type);

        // array type must match both length and value
        if (d->type->kind == TYPE_ARRAY) {
            struct type *expected_type = d->type->subtype;
            int init_list_length = 0;
            struct expr *e_ptr = d->value;

            // check type
            while (e_ptr) {
                struct type *init_list_item_type = expr_typecheck(e_ptr);
                if (!type_is_equal(expected_type, init_list_item_type)) {
                    ++error_count_type;
                    printf("type error: array `%s` initialization list received type ", d->name);
                    type_print(init_list_item_type);
                    printf(" at index %d, expecting ", init_list_length);
                    type_print(expected_type);
                    printf("\n");
                }
                TYPE_FREE(init_list_item_type);

                ++init_list_length;
                e_ptr = e_ptr->next;
            }

            // check length
            if (d->type->size
                && d->type->size->kind == EXPR_INTEGER
                && d->type->size->literal_value != init_list_length) {
                ++error_count_type;
                printf("type error: array `%s` initialization list has length %d, expecting %d\n", d->name, init_list_length, d->type->size->literal_value);
            }
        }

        // global initialization must use constant value
        if (d->symbol->kind == SYMBOL_GLOBAL
            && !expr_is_constant(d->value)) {
            ++error_count_type;
            printf("type error: initializing variable `%s` with non-constant expression `", d->name);
            expr_print(d->value);
            printf("`\n");
        }

    }

    // function: check body
    if (d->code) {
        stmt_typecheck(d->code, d->name, d->type->subtype);
    }

    // clean up

    // check next
    decl_typecheck(d->next);
}

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
