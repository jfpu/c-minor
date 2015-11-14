#include <stdlib.h> // malloc
#include <string.h> // memset
#include "type.h"
#include "scope.h"

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

// name resolution
void function_param_resolve(struct type *t, const char * const name) {
    // only for functions
    if (!t || t->kind != TYPE_FUNCTION) return;

    // resolve parameters
    int param_count = 0;
    struct param_list *p_ptr = t->params;
    while (p_ptr) {
        if (scope_lookup_current(p_ptr->name)) {
            // if the name already exists in current scope, error
            ++error_count_name;
            printf("name error: duplicate parameter name %s in function `%s`\n", p_ptr->name, name);

            // move on to next parameter
            p_ptr = p_ptr->next;
            continue;
        }

        // create symbol
        struct symbol *p_sym = symbol_create(SYMBOL_PARAM, param_count, p_ptr->type, p_ptr->name);
        scope_bind(p_ptr->name, p_sym);
        p_ptr->symbol = p_sym;
        if (__print_name_resolution_result) { print_name_resolution(p_sym); }

        // next
        ++param_count;
        p_ptr = p_ptr->next;
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

// actual type checking functions
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
