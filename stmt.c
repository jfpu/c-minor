#include <stdlib.h> // malloc
#include <string.h> // memset
#include "utility.h"
#include "stmt.h"
#include "scope.h"
#include "register.h"

#ifdef __linux__
#define FN_MANGLE_PREFIX ""
#else
#define FN_MANGLE_PREFIX "_"
#endif

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

void stmt_resolve(struct stmt *s, int *which, int param_count) {
    if (!s) return;
    // which is guaranteed to have a value

    struct stmt *s_ptr = s;
    while (s_ptr) {
        switch (s_ptr->kind) {
            case STMT_DECL:
                decl_resolve(s_ptr->decl, which, param_count);
                break;

            case STMT_EXPR:
                expr_resolve(s_ptr->expr);
                break;

            case STMT_IF_ELSE:
                expr_resolve(s_ptr->expr);
                stmt_resolve(s_ptr->body, which, param_count);
                stmt_resolve(s_ptr->else_body, which, param_count);
                break;

            case STMT_FOR:
                expr_resolve(s_ptr->init_expr);
                expr_resolve(s_ptr->expr);
                expr_resolve(s_ptr->next_expr);
                stmt_resolve(s_ptr->body, which, param_count);
                break;

            case STMT_PRINT:
            case STMT_RETURN:
                expr_resolve(s_ptr->expr);
                break;

            case STMT_BLOCK:
                // enter new scope and resolve body in new scope
                scope_enter();
                stmt_resolve(s_ptr->body, which, param_count);
                scope_exit();
                break;

            case STMT_EMPTY:
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
                if (s_ptr->expr && type_expr->kind != TYPE_BOOLEAN) {
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

#define UNWIND_STACK(__file) {              \
    fprintf((__file), "pop %%r15\n");       \
    fprintf((__file), "pop %%r14\n");       \
    fprintf((__file), "pop %%r13\n");       \
    fprintf((__file), "pop %%r12\n");       \
    fprintf((__file), "pop %%rbx\n");       \
    fprintf((__file), "mov %%rbp, %%rsp\n");\
    fprintf((__file), "pop %%rbp\n");       \
    fprintf((__file), "ret\n");             \
}

void stmt_codegen(struct stmt *s, FILE *file) {
    if (!s) return;

    struct stmt *s_ptr = s;
    while (s_ptr) {
        switch (s_ptr->kind) {
            case STMT_DECL: {
                decl_codegen(s_ptr->decl, file);
                break;
            }
            case STMT_EXPR: {
                expr_codegen(s_ptr->expr, file);
                register_free(s_ptr->expr->reg);
                break;
            }
            case STMT_IF_ELSE: {
                int false_label = label_count++;
                int end_label = label_count++;

                expr_codegen(s_ptr->expr, file);
                fprintf(file, "cmp $0, %s\n", register_name(s_ptr->expr->reg));
                fprintf(file, "je .label%d\n", false_label);
                stmt_codegen(s_ptr->body, file);
                fprintf(file, "jmp .label%d\n", end_label);
                fprintf(file, ".label%d:\n", false_label);
                stmt_codegen(s_ptr->else_body, file);
                fprintf(file, ".label%d:\n", end_label);
                break;
            }
            case STMT_FOR: {
                int loop_begin_label = label_count++;
                int loop_end_label = label_count++;

                // init
                if (s_ptr->init_expr) {
                    expr_codegen(s_ptr->init_expr, file);
                    register_free(s_ptr->init_expr->reg);
                }

                // loop body
                fprintf(file, ".label%d:\n", loop_begin_label);

                if (s_ptr->expr) {
                    expr_codegen(s_ptr->expr, file);
                    fprintf(file, "cmp $0, %s\n", register_name(s_ptr->expr->reg));
                    register_free(s_ptr->expr->reg);
                    fprintf(file, "je .label%d\n", loop_end_label);
                }
                stmt_codegen(s_ptr->body, file);

                // next
                if (s_ptr->next_expr) {
                    expr_codegen(s_ptr->next_expr, file);
                    register_free(s_ptr->next_expr->reg);
                }

                fprintf(file, "jmp .label%d\n", loop_begin_label);
                fprintf(file, ".label%d:\n", loop_end_label);
                break;
            }
            case STMT_PRINT: {
                struct expr *e_ptr = s_ptr->expr;
                while (e_ptr) {
                    expr_codegen(e_ptr, file);

                    // push caller save registers (r10, r11)
                    fprintf(file, "push %%r10\n");
                    fprintf(file, "push %%r11\n");

                    struct type *t = expr_typecheck(e_ptr);
                    switch (t->kind) {
                        case TYPE_BOOLEAN: {
                            fprintf(file, "mov %s, %%rdi\n", register_name(e_ptr->reg));
                            fprintf(file, "call %sprint_boolean\n", FN_MANGLE_PREFIX);
                            break;
                        }
                        case TYPE_CHARACTER: {
                            fprintf(file, "mov %s, %%rdi\n", register_name(e_ptr->reg));
                            fprintf(file, "call %sprint_character\n", FN_MANGLE_PREFIX);
                            break;
                        }
                        case TYPE_INTEGER: {
                            fprintf(file, "mov %s, %%rdi\n", register_name(e_ptr->reg));
                            fprintf(file, "call %sprint_integer\n", FN_MANGLE_PREFIX);
                            break;
                        }
                        case TYPE_STRING: {
                            fprintf(file, "mov %s, %%rdi\n", register_name(e_ptr->reg));
                            fprintf(file, "call %sprint_string\n", FN_MANGLE_PREFIX);
                            break;
                        }
                        default:
                            fprintf(stdout, "expr `");
                            expr_print(e_ptr);
                            fprintf(stdout, "` of unknown type passed to print\n");
                            exit(1);
                    }

                    // pop caller save registers
                    fprintf(file, "pop %%r11\n");
                    fprintf(file, "pop %%r10\n");

                    TYPE_FREE(t);

                    // reclaim register
                    register_free(e_ptr->reg);
                    e_ptr->reg = -1;

                    // move on
                    e_ptr = e_ptr->next;
                }
                break;
            }
            case STMT_RETURN: {
                // generate return value
                expr_codegen(s_ptr->expr, file);
                // move into %rax
                fprintf(file, "mov %s, %%rax\n", register_name(s_ptr->expr->reg));
                register_free(s_ptr->expr->reg);
                // unwind stack
                UNWIND_STACK(file);
                break;
            }
            case STMT_BLOCK: {
                stmt_codegen(s_ptr->body, file);
                break;
            }
            case STMT_EMPTY: {
                // do nothing!
                break;
            }
        }

        s_ptr = s_ptr->next;
    }
}

#undef UNWIND_STACK
#undef FN_MANGLE_PREFIX
