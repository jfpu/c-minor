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
#warning Unimplemented
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
                fprintf(stderr, "type error: cannot assign expression of type ");
                type_print(type_right);
                fprintf(stderr, " to expression of type ");
                type_print(type_left);
                fprintf(stderr, "\n");
            }
            TYPE_FREE(type_left);
            return type_right;
        }

        // +, -, *, /, ^, %, ++, -- only work on integers
        case EXPR_ADD: {
            type_left = expr_typecheck(e->left);
            type_right = expr_typecheck(e->right);
            if (type_left->kind != TYPE_INTEGER || type_right->kind != TYPE_INTEGER) {
                fprintf(stderr, "type error: cannot add expression of type ");
                type_print(type_left);
                fprintf(stderr, " to expression of type ");
                type_print(type_right);
                fprintf(stderr, "\n");
            }
            TYPE_FREE(type_left);
            TYPE_FREE(type_right);
            return type_create(TYPE_INTEGER, NULL, NULL);
        }
        case EXPR_SUB:
        case EXPR_MUL:
        case EXPR_DIV:
        case EXPR_EXP:
        case EXPR_MOD:
        case EXPR_INC:
        case EXPR_DEC:
        case EXPR_NEG: {
            type_right = expr_typecheck(e->right);
            if (type_right->kind != TYPE_INTEGER) {
                // error
            }
            TYPE_FREE(type_right);
            return type_create(TYPE_INTEGER, NULL, NULL);
        }

        // &&, ||, ! work on booleans
        case EXPR_LAND:
        case EXPR_LOR:
        case EXPR_LNOT:

        // <, <=, >, >= work on only integers
        case EXPR_LT:
        case EXPR_LE:
        case EXPR_GT:
        case EXPR_GE:

        // EQ and NE work on any type except arrays and functions
        case EXPR_EQ: {
            type_left = expr_typecheck(e->left);
            type_right = expr_typecheck(e->right);
            if (type_left->kind != TYPE_INTEGER || type_right->kind != TYPE_INTEGER) {
                fprintf(stderr, "type error: cannot add expression (");
                expr_print(e->left);
                fprintf(stderr, ") of type ");
                type_print(type_left);
                fprintf(stderr, " to expression (");
                expr_print(e->right);
                fprintf(stderr, ")of type ");
                type_print(type_right);
                fprintf(stderr, "\n");
            }
            TYPE_FREE(type_left);
            TYPE_FREE(type_right);
            return type_create(TYPE_INTEGER, NULL, NULL);
        }
        case EXPR_NE: {

        }

        // a[b]: a must be an array and b must be an integer
        case EXPR_ARRAY_DEREF: {

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

    if (/*d is global*/1 && !expr_is_constant(d->value)) {
        // error
    }

    if (d->code) {
        stmt_typecheck(d->code, d->type->subtype);
    }

    // clean up
    TYPE_FREE(value_type);
}

void param_list_typecheck(struct param_list *p, struct expr *e) {
#warning Unimplemented
}
