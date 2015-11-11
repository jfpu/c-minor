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
            // if the function name isn't correctly resolved or if the name isn't a function, move on
            if (!e->left->symbol
                || e->left->symbol->type->kind != TYPE_FUNCTION) {
                ++error_count_type;
                printf("type error: expression `");
                expr_print(e->left);
                printf("` is not callable\n");
                return type_create(TYPE_VOID, NULL, NULL);
            }

            // otherwise, type check the formal parameter list
            param_list_typecheck(e->left->symbol->type->params, e->right, e->left->name);
            return type_copy(e->left->symbol->type->subtype);
        }

        // = works on any type except arrays
        case EXPR_ASSIGN: {
            type_left = expr_typecheck(e->left);

            // we can only assign to an lvalue
            if (!expr_is_lvalue_type(e->left)) {
                ++error_count_type;
                printf("type error: expression `");
                expr_print(e->left);
                printf("` of type ");
                type_print(type_left);
                printf(" is not an lvalue\n");
            }

            type_right = expr_typecheck(e->right);
            if (!type_is_equal(type_left, type_right)) {
                ++error_count_type;
                printf("type error: cannot assign expression `");
                expr_print(e->right);
                printf("` of type ");
                type_print(type_right);
                printf(" to expression `");
                expr_print(e->left);
                printf("` of type ");
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

                // prematurely return an appropriate type to avoid comparing null types
                TYPE_FREE(type_left);
                return type_right;
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

void stmt_typecheck(struct stmt *s, const char *name, struct type *expected) {
    if (!s) return;

    switch (s->kind) {
        case STMT_DECL: {
            decl_typecheck(s->decl);
            break;
        }
        case STMT_EXPR: {
            expr_typecheck(s->expr);
            break;
        }
        case STMT_IF_ELSE: {
            struct type *type_expr = expr_typecheck(s->expr);
            if (type_expr->kind != TYPE_BOOLEAN) {
                ++error_count_type;
                printf("type error: if statement received expression of type ");
                type_print(type_expr);
                printf(", expected boolean\n");
            }
            stmt_typecheck(s->body, name, expected);
            stmt_typecheck(s->else_body, name, expected);
            TYPE_FREE(type_expr);
            break;
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
                ++error_count_type;
                printf("type error: for statement received expression of type ");
                type_print(type_expr);
                printf(", expected boolean\n");
            }
            stmt_typecheck(s->body, name, expected);
            TYPE_FREE(type_expr);
            break;
        }
        case STMT_PRINT: {
            // type check each item in expr list
            struct type *type_expr = NULL;
            struct expr *e_ptr = s->expr;
            while (e_ptr) {
                type_expr = expr_typecheck(s->expr);
                TYPE_FREE(type_expr);
            }
            break;
        }
        case STMT_RETURN: {
            // function must return the expected type
            struct type *type_expr = expr_typecheck(s->expr);
            if (!type_is_equal(type_expr, expected)) {
                ++error_count_type;
                printf("type error: function `%s` with return type ", name);
                type_print(expected);
                printf(" returns expression of type ");
                type_print(type_expr);
                printf("\n");
            }
            TYPE_FREE(type_expr);
            break;
        }
        case STMT_BLOCK: {
            stmt_typecheck(s->body, name, expected);
            break;
        }
        case STMT_EMPTY:
            // dummy node, ignore
            break;
    }

    stmt_typecheck(s->next, name, expected);
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
