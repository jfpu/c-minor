#include <stdlib.h> // malloc
#include <string.h> // memset
#include "type.h"

#define TYPE_FREE(_type_obj)    \
    type_delete((_type_obj));   \
    (_type_obj) = NULL

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
struct type *expr_typecheck(struct expr *e) {
    if (!e) return type_create(TYPE_VOID, NULL, NULL);

    struct type *type_left = NULL;
    struct type *type_right = NULL;

    switch (e->kind) {
        case EXPR_NAME:
            // name resolution
            return type_copy(e->symbol->type);

        case EXPR_BOOLEAN:
            return type_create(TYPE_BOOLEAN, NULL, NULL);
        case EXPR_INTEGER:
            return type_create(TYPE_INTEGER, NULL, NULL);
        case EXPR_CHARACTER:
            return type_create(TYPE_CHARACTER, NULL, NULL);
        case EXPR_STRING:
            return type_create(TYPE_STRING, NULL, NULL);

        case EXPR_FCALL: {
            // name resolution
            // type check the formal parameter list
            param_list_typecheck(e->left->symbol->type->params, e->right);
            return type_copy(e->left->symbol->type->subtype);
        }

        // = works on any type except arrays
        case EXPR_ASSIGN: {
            type_left = expr_typecheck(e->left);
            type_right = expr_typecheck(e->right);
            if (!type_is_equal(type_left, type_right)) {
                printf("type error: cannot assign expression of type ");
                type_print(type_right);
                printf(" to expression of type ");
                type_print(type_left);
                printf("\n");
            }
            TYPE_FREE(type_left);
            return type_right;
        }

        // +, -, *, /, ^, %, ++, -- only work on integers
        case EXPR_ADD:
        case EXPR_SUB:
        case EXPR_MUL:
        case EXPR_DIV:
        case EXPR_EXP:
        case EXPR_MOD: {
            type_left = expr_typecheck(e->left);
            type_right = expr_typecheck(e->right);
            if (type_left->kind != TYPE_INTEGER
                || type_right->kind != TYPE_INTEGER) {
                // error
                ++error_count_type;
                printf("type error: cannot perform arithmetic operator on expression of type ");
                type_print(type_left);
                printf(" with expression of type ");
                type_print(type_right);
                printf("\n");
            }
            TYPE_FREE(type_left);
            TYPE_FREE(type_right);
            return type_create(TYPE_INTEGER, NULL, NULL);
        }

        case EXPR_INC:
        case EXPR_DEC:
        case EXPR_NEG: {
            type_right = expr_typecheck(e->right);
            if (type_right->kind != TYPE_INTEGER) {
                // error
                ++error_count_type;
                printf("type error: cannot perform arithmetic operator on expression of type ");
                type_print(type_right);
                printf("\n");
            }
            TYPE_FREE(type_right);
            return type_create(TYPE_INTEGER, NULL, NULL);
        }

        // &&, ||, ! work on booleans
        case EXPR_LAND:
        case EXPR_LOR:
        case EXPR_LNOT: {
            type_right = expr_typecheck(e->right);
            if (type_right->kind != TYPE_BOOLEAN) {
                // error
                ++error_count_type;
                printf("type error: cannot perform boolean operator on expression of type ");
                type_print(type_right);
                printf("\n");
            }
            TYPE_FREE(type_right);
            return type_create(TYPE_BOOLEAN, NULL, NULL);
        }

        // <, <=, >, >= work on only integers
        case EXPR_LT:
        case EXPR_LE:
        case EXPR_GT:
        case EXPR_GE: {
            type_left = expr_typecheck(e->left);
            type_right = expr_typecheck(e->right);
            if (type_left->kind != TYPE_INTEGER
                || type_right->kind != TYPE_INTEGER) {
                // error
                ++error_count_type;
                printf("type error: cannot perform comparison operator on expression of type ");
                type_print(type_left);
                printf(" with expression of type ");
                type_print(type_right);
                printf("\n");
            }
            TYPE_FREE(type_left);
            TYPE_FREE(type_right);
            return type_create(TYPE_BOOLEAN, NULL, NULL);
        }

        // EQ and NE work on any type except arrays and functions
        case EXPR_EQ:
        case EXPR_NE: {
            type_left = expr_typecheck(e->left);
            type_right = expr_typecheck(e->right);
            if (type_left->kind != type_right->kind) {
                ++error_count_type;
                printf("type error: cannot compare expressions of type ");
                type_print(type_left);
                printf(" and of type ");
                type_print(type_right);
                printf("\n");
            }
            TYPE_FREE(type_left);
            TYPE_FREE(type_right);
            return type_create(TYPE_BOOLEAN, NULL, NULL);
        }

        // a[b]: a must be an array and b must be an integer
        case EXPR_ARRAY_DEREF: {
            type_left = expr_typecheck(e->left);
            type_right = expr_typecheck(e->right);
            if (type_left->kind != TYPE_ARRAY) {
                ++error_count_type;
                printf("type error: cannot dereference an expression of type ");
                type_print(type_left);
                printf("\n");
            }
            if (type_right->kind != TYPE_INTEGER) {
                ++error_count_type;
                printf("type error: array subscript cannot be of type ");
                type_print(type_right);
                printf("\n");
            }

            // compute return type
            struct type *array_subtype = type_copy(type_left->subtype);
            TYPE_FREE(type_left);
            TYPE_FREE(type_right);

            return array_subtype;
        }

        default: {
            // this should never happen
            fprintf(stderr, "fatal error: unknown type\n");
            return type_create(TYPE_VOID, NULL, NULL);
        }
    }
}

void stmt_typecheck(struct stmt *s, struct type *expected) {
    if (!s) return;

    switch (s->kind) {
        case STMT_DECL:
        case STMT_EXPR:
        case STMT_IF_ELSE: {
            struct type *type_expr = expr_typecheck(s->expr);
            if (type_expr->kind != TYPE_BOOLEAN) {
                // error
            }

            stmt_typecheck(s->body, expected);
            stmt_typecheck(s->else_body, expected);
            TYPE_FREE(type_expr);
        }
        case STMT_FOR: {
            struct type *type_expr = NULL;

            // type check init and next
            type_expr = expr_typecheck(s->init_expr);
            TYPE_FREE(type_expr);
            type_expr = expr_typecheck(s->next_expr);
            TYPE_FREE(type_expr);

            // type check current body
            type_expr = expr_typecheck(s->expr);
            if (type_expr->kind != TYPE_BOOLEAN) {
                // error
            }
            stmt_typecheck(s->body, expected);
            TYPE_FREE(type_expr);
        }
        case STMT_PRINT:
        case STMT_RETURN: {
            struct type *type_expr = expr_typecheck(s->expr);
            if (!type_is_equal(type_expr, expected)) {
                // error
            }
            TYPE_FREE(type_expr);
        }
        case STMT_BLOCK:
        case STMT_EMPTY:
            // dummy node, ignore
            return;
    }
}

void decl_typecheck(struct decl *d) {
    if (!d) return;
    struct type *value_type = expr_typecheck(d->value);
    if (!type_is_equal(d->type, value_type)) {
        // error
    }

    if (/*d is global*/1 && d->value && !expr_is_constant(d->value)) {
        // error
    }

    if (d->code) {
        stmt_typecheck(d->code, d->type->subtype);
    }

    // clean up
    TYPE_FREE(value_type);
}

void param_list_typecheck(struct param_list *p, struct expr *e) {
    // this is invoked for each function invocation
    // we compare each item in the param list with the given expression list

    struct param_list *p_ptr = p;
    struct expr *e_ptr = e;

    while (p_ptr && e_ptr) {
        // comparing each pair
        struct type *expected_type = p->type;
        struct type *received_type = expr_typecheck(e);
        if (!type_is_equal(expected_type, received_type)) {
            // error
            ++error_count_type;
            fprintf(stderr, "type error: param list type mismatch; expected ");
            type_print(expected_type);
            fprintf(stderr, ", received ");
            type_print(received_type);
        }
        TYPE_FREE(received_type);

        // move on
        p_ptr = p_ptr->next;
        e_ptr = e_ptr->next;
    }

    // ensure lengths are the same
    if (p_ptr != NULL || e_ptr != NULL) {
        ++error_count_type;
        fprintf(stderr, "type error: param list length mismatch; expected %u parameters, received %u arguments",
            param_list_length(p),
            expr_list_length(e));
    }
}
