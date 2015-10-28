#include <stdlib.h> // malloc
#include <string.h> // memset
#include "expr.h"

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

struct expr *expr_create_function_call(const char *name, struct expr *params) {
    struct expr *e = expr_create(EXPR_FCALL, NULL, NULL);
    e->name = name;
    e->params = params;
    return e;
}

struct expr *expr_create_array_deref(const char *name, struct expr *index) {
    struct expr *e = expr_create(EXPR_ARRAY_DEREF, NULL, NULL);
    e->name = name;
    e->params = index;
    return e;
}

struct expr *expr_create_incdec(expr_t kind, const char *name) {
    struct expr *e = expr_create(kind, NULL, NULL);
    e->name = name;
    return e;
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
                printf("\\0");
            } else if (e->literal_value == '\n') {
                printf("\\n");
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

        case EXPR_ADD:
            expr_print(e->left);
            printf("+");
            expr_print(e->right);
            break;

        case EXPR_SUB:
            expr_print(e->left);
            printf("-");
            expr_print(e->right);
            break;

        case EXPR_MUL:
            printf("(");
            expr_print(e->left);
            printf(")*(");
            expr_print(e->right);
            printf(")");
            break;

        case EXPR_DIV:
            printf("(");
            expr_print(e->left);
            printf(")/(");
            expr_print(e->right);
            printf(")");
            break;

        case EXPR_EXP:
            expr_print(e->left);
            printf("^");
            expr_print(e->right);
            break;

        case EXPR_MOD:
            printf("(");
            expr_print(e->left);
            printf(")%%(");
            expr_print(e->right);
            printf(")");
            break;

        case EXPR_INC:
            printf("%s++", e->name);
            break;

        case EXPR_DEC:
            printf("%s--", e->name);
            break;

        case EXPR_LAND:
            printf("(");
            expr_print(e->left);
            printf(")&&(");
            expr_print(e->right);
            printf(")");
            break;

        case EXPR_LOR:
            printf("(");
            expr_print(e->left);
            printf(")||(");
            expr_print(e->right);
            printf(")");
            break;

        case EXPR_LNOT:
            printf("!(");
            expr_print(e->right);
            printf(")");
            break;

        case EXPR_LT:
            expr_print(e->left);
            printf("<");
            expr_print(e->right);
            break;

        case EXPR_LE:
            expr_print(e->left);
            printf("<=");
            expr_print(e->right);
            break;

        case EXPR_GT:
            expr_print(e->left);
            printf(">");
            expr_print(e->right);
            break;

        case EXPR_GE:
            expr_print(e->left);
            printf(">=");
            expr_print(e->right);
            break;

        case EXPR_EQ:
            expr_print(e->left);
            printf("==");
            expr_print(e->right);
            break;

        case EXPR_NE:
            expr_print(e->left);
            printf("!=");
            expr_print(e->right);
            break;

        case EXPR_FCALL:
            printf("%s(", e->name);
            expr_print(e->params);
            printf(")");
            break;

        case EXPR_ARRAY_DEREF:
            printf("%s[", e->name);
            expr_print(e->params);
            printf("]");
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
