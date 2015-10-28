#include <stdlib.h> // malloc
#include <string.h> // memset
#include "utility.h"
#include "stmt.h"

struct stmt *stmt_create(stmt_kind_t kind, struct decl *d, struct expr *init_expr, struct expr *e, struct expr *next_expr, struct stmt *body, struct stmt *else_body) {
    struct stmt *s = (struct stmt *)malloc(sizeof(*s));
    memset(s, 0, sizeof(*s));

    s->kind = kind;
    s->decl = d;
    s->init_expr = init_expr;
    s->expr = e;
    s->next_expr = next_expr;
    s->body = body;
    s->else_body = else_body;
    s->next = NULL;
    return s;
}

struct stmt *stmt_list_prepend(struct stmt *first, struct stmt *rest) {
    first->next = rest;
    return first;
}

void stmt_print(struct stmt *s, int indent) {
    switch (s->kind) {
        case STMT_DECL:
            decl_print(s->decl, indent);
            break;

        case STMT_EXPR:
            _print_indent(indent);
            expr_print(s->expr);
            printf(";\n");
            break;

        case STMT_IF_ELSE:
            _print_indent(indent);
            printf("if (");
            expr_print(s->expr);
            printf(") {\n");
            stmt_print(s->body, indent + 1);
            if (s->else_body) {
                printf("} else {\n");
                stmt_print(s->else_body, indent + 1);
            }
            _print_indent(indent);
            printf("}\n");
            break;

        case STMT_FOR:
            _print_indent(indent);
            printf("for (");
            if (s->init_expr) expr_print(s->init_expr);
            printf("; ");
            if (s->expr) expr_print(s->expr);
            printf("; ");
            if (s->next_expr) expr_print(s->next_expr);
            printf(") {\n");
            stmt_print(s->body, indent + 1);
            _print_indent(indent);
            printf("}\n");
            break;

        case STMT_PRINT:
            _print_indent(indent);
            printf("print ");
            expr_print(s->expr);
            printf(";\n");
            break;

        case STMT_RETURN:
            _print_indent(indent);
            printf("return ");
            expr_print(s->expr);
            printf(";\n");
            break;

        default:
            _print_indent(indent);
            printf("Statement!\n");
            break;
    }

    if (s->next) stmt_print(s->next, indent);
}
