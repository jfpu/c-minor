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
