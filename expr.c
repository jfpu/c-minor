#include <stdlib.h> // malloc
#include <string.h> // memset
#include "expr.h"

#define PRINT_WITH_PRECEDENCE(expr, base)                       \
    if (expr_precedence((expr)) < expr_precedence((base))) {    \
        printf("("); expr_print((expr)); printf(")");           \
    } else {                                                    \
        expr_print((expr));                                     \
    }

int expr_precedence(struct expr *e);

struct expr *expr_create(expr_t kind, struct expr *left, struct expr *right) {
    struct expr *e = (struct expr *)malloc(sizeof(*e));
    memset(e, 0, sizeof(*e));

    e->kind = kind;
    e->left = left;
    e->right = right;
    return e;
}

struct expr *expr_list_prepend(struct expr *first, struct expr *rest) {
    first->next = rest;
    return first;
}

struct expr *expr_create_name(const char *n) {
    struct expr *e = expr_create(EXPR_NAME, NULL, NULL);
    e->name = n;
    return e;
}

struct expr *expr_create_boolean_literal(int c) {
    struct expr *e = expr_create(EXPR_BOOLEAN, NULL, NULL);
    e->literal_value = c;
    return e;
}

struct expr *expr_create_integer_literal(int c) {
    struct expr *e = expr_create(EXPR_INTEGER, NULL, NULL);
    e->literal_value = c;
    return e;
}

struct expr *expr_create_character_literal(int c) {
    struct expr *e = expr_create(EXPR_CHARACTER, NULL, NULL);
    e->literal_value = c;
    return e;
}

struct expr *expr_create_string_literal(const char *str) {
    struct expr *e = expr_create(EXPR_STRING, NULL, NULL);
    e->string_literal = str;
    return e;
}

int expr_precedence(struct expr *e) {
    switch (e->kind) {
        case EXPR_NAME:
        case EXPR_BOOLEAN:
        case EXPR_INTEGER:
        case EXPR_CHARACTER:
        case EXPR_STRING:
        case EXPR_ARRAY_DEREF:
        case EXPR_FCALL:
            return 10;

        case EXPR_INC:
        case EXPR_DEC:
            return 9;

        case EXPR_NEG:
        case EXPR_LNOT:
            return 8;

        case EXPR_EXP:
            return 7;

        case EXPR_MUL:
        case EXPR_DIV:
        case EXPR_MOD:
            return 6;

        case EXPR_ADD:
        case EXPR_SUB:
            return 5;

        case EXPR_LT:
        case EXPR_LE:
        case EXPR_GT:
        case EXPR_GE:
        case EXPR_EQ:
        case EXPR_NE:
            return 4;

        case EXPR_LAND:
            return 3;

        case EXPR_LOR:
            return 2;

        case EXPR_ASSIGN:
            return 1;
    }
    return -1;
}

void expr_print(struct expr *e) {
    if (!e) return;

    int i = 0;
    switch (e->kind) {
        case EXPR_NAME:
            printf("%s", e->name);
            break;

        case EXPR_BOOLEAN:
            if (e->literal_value) {
                printf("true");
            } else {
                printf("false");
            }
            break;

        case EXPR_INTEGER:
            printf("%d", e->literal_value);
            break;

        case EXPR_CHARACTER:
            if (e->literal_value == '\0') {
                printf("'\\0'");
            } else if (e->literal_value == '\n') {
                printf("'\\n'");
            } else {
                printf("'%c'", e->literal_value);
            }
            break;

        case EXPR_STRING:
            // We need to unescape each individual character
            i = 0;
            printf("\"");
            while (e->string_literal[i]) {
                if (e->string_literal[i] == '\n') {
                    printf("\\n");
                } else {
                    printf("%c", e->string_literal[i]);
                }
                ++i;
            }
            printf("\"");
            break;

        case EXPR_ASSIGN:
            expr_print(e->left);
            printf("=");
            expr_print(e->right);
            break;

        case EXPR_FCALL:
            expr_print(e->left);
            printf("("); expr_print(e->right); printf(")");
            break;

        case EXPR_ARRAY_DEREF:
            expr_print(e->left);
            printf("["); expr_print(e->right); printf("]");
            break;

        case EXPR_ADD:
            PRINT_WITH_PRECEDENCE(e->left, e);
            printf("+");
            PRINT_WITH_PRECEDENCE(e->right, e);
            break;

        case EXPR_SUB:
            PRINT_WITH_PRECEDENCE(e->left, e);
            printf("-");
            PRINT_WITH_PRECEDENCE(e->right, e);
            break;

        case EXPR_MUL:
            PRINT_WITH_PRECEDENCE(e->left, e);
            printf("*");
            PRINT_WITH_PRECEDENCE(e->right, e);
            break;

        case EXPR_DIV:
            PRINT_WITH_PRECEDENCE(e->left, e);
            printf("/");
            PRINT_WITH_PRECEDENCE(e->right, e);
            break;

        case EXPR_EXP:
            PRINT_WITH_PRECEDENCE(e->left, e);
            printf("^");
            PRINT_WITH_PRECEDENCE(e->right, e);
            break;

        case EXPR_MOD:
            PRINT_WITH_PRECEDENCE(e->left, e);
            printf("%%");
            PRINT_WITH_PRECEDENCE(e->right, e);
            break;

        case EXPR_INC:
            PRINT_WITH_PRECEDENCE(e->right, e);
            printf("++");
            break;

        case EXPR_DEC:
            PRINT_WITH_PRECEDENCE(e->right, e);
            printf("--");
            break;

        case EXPR_NEG:
            printf("-");
            PRINT_WITH_PRECEDENCE(e->right, e);
            break;

        case EXPR_LAND:
            PRINT_WITH_PRECEDENCE(e->left, e);
            printf("&&");
            PRINT_WITH_PRECEDENCE(e->right, e);
            break;

        case EXPR_LOR:
            PRINT_WITH_PRECEDENCE(e->left, e);
            printf("||");
            PRINT_WITH_PRECEDENCE(e->right, e);
            break;

        case EXPR_LNOT:
            printf("!");
            PRINT_WITH_PRECEDENCE(e->right, e);
            break;

        case EXPR_LT:
            PRINT_WITH_PRECEDENCE(e->left, e);
            printf("<");
            PRINT_WITH_PRECEDENCE(e->right, e);
            break;

        case EXPR_LE:
            PRINT_WITH_PRECEDENCE(e->left, e);
            printf("<=");
            PRINT_WITH_PRECEDENCE(e->right, e);
            break;

        case EXPR_GT:
            PRINT_WITH_PRECEDENCE(e->left, e);
            printf(">");
            PRINT_WITH_PRECEDENCE(e->right, e);
            break;

        case EXPR_GE:
            PRINT_WITH_PRECEDENCE(e->left, e);
            printf(">=");
            PRINT_WITH_PRECEDENCE(e->right, e);
            break;

        case EXPR_EQ:
            PRINT_WITH_PRECEDENCE(e->left, e);
            printf("==");
            PRINT_WITH_PRECEDENCE(e->right, e);
            break;

        case EXPR_NE:
            PRINT_WITH_PRECEDENCE(e->left, e);
            printf("!=");
            PRINT_WITH_PRECEDENCE(e->right, e);
            break;

        default:
            printf("Expression");
            break;
    }

    if (e->next) {
        printf(", ");
        expr_print(e->next);
    }
}

// for type checking
unsigned int expr_list_length(struct expr *e) {
    struct expr *e_ptr = e;
    unsigned int length = 0;

    while (e_ptr) {
        ++length;
        e_ptr = e_ptr->next;
    }

    return length;
}

int expr_is_constant(struct expr *e) {
    if (!e) return 0;

    // we don't allow folding for now
    return (e->kind == EXPR_BOOLEAN
        || e->kind == EXPR_INTEGER
        || e->kind == EXPR_CHARACTER
        || e->kind == EXPR_STRING);
}
