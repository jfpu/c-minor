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
    if (!s) return;

    struct stmt *s_ptr = s;

    while (s_ptr) {
        switch (s_ptr->kind) {
            case STMT_DECL:
                decl_print(s_ptr->decl, indent);
                break;

            case STMT_EXPR:
                _print_indent(indent);
                expr_print(s_ptr->expr);
                printf(";\n");
                break;

            case STMT_IF_ELSE:
                _print_indent(indent);
                printf("if (");
                expr_print(s_ptr->expr);
                printf(")\n");
                if (s_ptr->body && s_ptr->body->kind == STMT_BLOCK) {
                    stmt_print(s_ptr->body, indent);
                } else if (s_ptr->body) {
                    stmt_print(s_ptr->body, indent + 1);
                }
                if (s_ptr->else_body) {
                    _print_indent(indent);
                    printf("else\n");
                    if (s_ptr->else_body->kind == STMT_BLOCK) {
                        stmt_print(s_ptr->else_body, indent);
                    } else {
                        stmt_print(s_ptr->else_body, indent + 1);
                    }
                }
                break;

            case STMT_FOR:
                _print_indent(indent);
                printf("for (");
                if (s_ptr->init_expr) expr_print(s_ptr->init_expr);
                printf("; ");
                if (s_ptr->expr) expr_print(s_ptr->expr);
                printf("; ");
                if (s_ptr->next_expr) expr_print(s_ptr->next_expr);
                printf(")\n");
                if (s_ptr->body && s_ptr->body->kind == STMT_BLOCK) {
                    stmt_print(s_ptr->body, indent);
                } else if (s_ptr->body) {
                    stmt_print(s_ptr->body, indent + 1);
                }
                break;

            case STMT_PRINT:
                _print_indent(indent);
                printf("print");
                if (s_ptr->expr) {
                    printf(" ");
                    expr_print(s_ptr->expr);
                }
                printf(";\n");
                break;

            case STMT_RETURN:
                _print_indent(indent);
                printf("return");
                if (s_ptr->expr) {
                    printf(" ");
                    expr_print(s_ptr->expr);
                }
                printf(";\n");
                break;

            case STMT_BLOCK:
                _print_indent(indent);
                printf("{\n");
                stmt_print(s_ptr->body, indent + 1);
                _print_indent(indent);
                printf("}\n");
                break;

            case STMT_EMPTY:
                break;

            default:
                _print_indent(indent);
                printf("Statement!\n");
                break;
        }

        s_ptr = s_ptr->next;
    }
}

void stmt_typecheck(struct stmt *s, const char *name, struct type *expected) {
    if (!s) return;

    struct stmt *s_ptr = s;
    while (s_ptr) {
        switch (s_ptr->kind) {
            case STMT_DECL: {
                decl_typecheck(s_ptr->decl);
                break;
            }
            case STMT_EXPR: {
                expr_typecheck(s_ptr->expr);
                break;
            }
            case STMT_IF_ELSE: {
                struct type *type_expr = expr_typecheck(s_ptr->expr);
                if (type_expr->kind != TYPE_BOOLEAN) {
                    ++error_count_type;
                    printf("type error: if statement received expression of type ");
                    type_print(type_expr);
                    printf(", expected boolean\n");
                }
                stmt_typecheck(s_ptr->body, name, expected);
                stmt_typecheck(s_ptr->else_body, name, expected);
                TYPE_FREE(type_expr);
                break;
            }
            case STMT_FOR: {
                struct type *type_expr = NULL;

                // type check init and next
                type_expr = expr_typecheck(s_ptr->init_expr);
                TYPE_FREE(type_expr);
                type_expr = expr_typecheck(s_ptr->next_expr);
                TYPE_FREE(type_expr);

                // type check current body
                type_expr = expr_typecheck(s_ptr->expr);
                if (type_expr->kind != TYPE_BOOLEAN) {
                    ++error_count_type;
                    printf("type error: for statement received expression of type ");
                    type_print(type_expr);
                    printf(", expected boolean\n");
                }
                stmt_typecheck(s_ptr->body, name, expected);
                TYPE_FREE(type_expr);
                break;
            }
            case STMT_PRINT: {
                // type check each item in expr list
                expr_list_typecheck(s_ptr->expr, NULL);
                break;
            }
            case STMT_RETURN: {
                // function must return the expected type
                struct type *type_expr = expr_typecheck(s_ptr->expr);
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
                stmt_typecheck(s_ptr->body, name, expected);
                break;
            }
            case STMT_EMPTY:
                // dummy node, ignore
                break;
        }
        s_ptr = s_ptr->next;
    }
}
