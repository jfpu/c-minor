#include <stdlib.h> // malloc
#include <string.h> // memset
#include "utility.h"
#include "decl.h"
#include "expr.h"
#include "stmt.h"
#include "scope.h"
#include "type.h"
#include "register.h"

#ifdef __linux__
#define FN_MANGLE_PREFIX ""
#else
#define FN_MANGLE_PREFIX "_"
#endif

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

void decl_resolve(struct decl *d, int *which, int param_count) {
    // `which` indicates the `which` value for local declarations, and is NULL for global
    if (!d) return;
    struct decl *d_ptr = d;

    while (d_ptr) {

        struct symbol *looked_up = scope_lookup_current(d_ptr->name);
        if (looked_up && !(looked_up->type->kind == TYPE_FUNCTION && looked_up->is_prototype_only)) {
            // if the name already exists in current scope, and it's not a funciton prototype, error
            ++error_count_name;
            printf("name error: duplicate declaration for name `%s` with type ", d_ptr->name);
            type_print(d_ptr->type);
            printf(" (previously declared as ");
            type_print(looked_up->type);
            printf(")\n");

        } else if (looked_up && looked_up->type->kind == TYPE_FUNCTION && looked_up->is_prototype_only) {
            // we're defining a previously declared function
            struct symbol *s = looked_up;
            if (__print_name_resolution_result) { print_name_resolution(s); }

            // we could re-resolve the parameters without issues
            scope_enter();
            function_param_resolve(d_ptr->type, d_ptr->name);
            s->param_count = param_list_length(d_ptr->type->params);

            if (d_ptr->code) {
                // if code is supplied, resolve code
                int new_function_scope_which = 0;
                stmt_resolve(d_ptr->code, &new_function_scope_which, s->param_count);

                // new_function_scope_which comes back as the total number of locals
                s->local_count = new_function_scope_which;

                // function is not prototype only, set accordingly
                s->is_prototype_only = 0;
            }
            scope_exit();

        } else {
            // all good: create a new symbol and bind to current scope
            // we don't copy d_ptr->type here, because we need to resolve parameters / size later
            struct symbol *s = NULL;
            if (which) {
                s = symbol_create(scope_table_list->scope, *which, d_ptr->type, d_ptr->name);
                ++(*which);
            } else {
                s = symbol_create(scope_table_list->scope, -1, d_ptr->type, d_ptr->name);
            }
            s->param_count = param_count;

            scope_bind(d_ptr->name, s);
            d_ptr->symbol = s;
            if (__print_name_resolution_result) { print_name_resolution(s); }

            // ensure parameter names in function prototypes are properly resolved
            // enter new scope for function and resolve parameters
            scope_enter();
            function_param_resolve(d_ptr->type, d_ptr->name);

            // keep track of count of params
            s->param_count = param_list_length(d_ptr->type->params);

            if (d_ptr->code) {
                // if declaration is a function, resolve funciton body with new scope
                int new_function_scope_which = 0;
                stmt_resolve(d_ptr->code, &new_function_scope_which, s->param_count);

                // new_function_scope_which comes back as the total number of locals
                s->local_count = new_function_scope_which;

                // function is not prototype only, set accordingly
                s->is_prototype_only = 0;
            }
            scope_exit();
        }

        if (d_ptr->value) {
            // if there's initialization, type check initialization
            expr_resolve(d_ptr->value);
        }

        d_ptr = d_ptr->next;
    }
}

void decl_typecheck(struct decl *d) {
    struct decl *d_ptr = d;
    while (d_ptr) {
        decl_typecheck_individual(d_ptr);
        d_ptr = d_ptr->next;
    }
}

void decl_typecheck_individual(struct decl *d) {
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
}

// codegen
void decl_codegen(struct decl *d, FILE *file) {
    struct decl *d_ptr = d;
    while (d_ptr) {
        decl_codegen_individual(d_ptr, file);
        d_ptr = d_ptr->next;
    }
}

void decl_codegen_individual(struct decl *d, FILE *file) {
    if (!d) return;

    // arrays are not supported
    if (d->symbol->type->kind == TYPE_ARRAY) {
        printf("error: arrays are not supported\n");
        exit(1);
    }

    if (d->symbol->kind == SYMBOL_GLOBAL && d->symbol->type->kind != TYPE_FUNCTION) {
        // global data: emit into data section
        fprintf(file, ".data\n");

        if (d->symbol->type->kind == TYPE_STRING) {
            // if it's a string, first emit a string literal
            int string_label = label_count++;

            // switch into data section, create the string, and switch back and use it
            fprintf(file, ".data\n");
            fprintf(file, "str%d:\n", string_label);
            fprintf(file, ".asciz ");
            if (d->value) {
                expr_string_print(d->value->string_literal, file);
            } else {
                fprintf(file, "\"\"");
            }
            fprintf(file, "\n");

            // then emit a quad word
            fprintf(file, "%s:\n", d->symbol->name);
            fprintf(file, ".quad str%d\n", string_label);

        } else {
            // otherwise emit a quad word
            fprintf(file, "%s:\n", d->symbol->name);
            fprintf(file, ".quad ");
            if (d->value) {
                fprintf(file, "%d\n", d->value->literal_value);
            } else {
                fprintf(file, "0\n");
            }
        }
        fprintf(file, "\n");

    } else if (d->symbol->kind == SYMBOL_GLOBAL && d->symbol->type->kind == TYPE_FUNCTION) {
        // global function: emit into text section
        // if this is only a prototype, do nothing!
        if (!d->code) return;

        fprintf(file, ".text\n");
        fprintf(file, ".global %s%s\n", FN_MANGLE_PREFIX, d->symbol->name);
        fprintf(file, "%s%s:\n", FN_MANGLE_PREFIX, d->symbol->name);

        // set up call stack
        fprintf(file, "push %%rbp\n");
        fprintf(file, "mov %%rsp, %%rbp\n");

        // for each parameter, push it on the stack
        if (d->symbol->param_count > 6) {
            printf("error: functions with over 6 arguments are not supported\n");
            exit(1);
        }
        struct param_list *p_ptr = d->type->params;
        while (p_ptr) {
            fprintf(file, "push %s\n", param_register_name(p_ptr->symbol->which));
            p_ptr = p_ptr->next;
        }

        // for the total number of local variables, make room in the stack
        // OS X requires 16-bit stack alignment
        int rsp_move_amount;
        if (d->symbol->local_count % 2 == 1) {
            rsp_move_amount = 8 * (d->symbol->local_count + 1);
        } else {
            rsp_move_amount = 8 * d->symbol->local_count;
        }
        fprintf(file, "sub $%d, %%rsp\n", rsp_move_amount);

        // then save callee-save registers
        fprintf(file, "push $0\n");
        fprintf(file, "push %%rbx\n");
        fprintf(file, "push %%r12\n");
        fprintf(file, "push %%r13\n");
        fprintf(file, "push %%r14\n");
        fprintf(file, "push %%r15\n");

        // then generate code
        stmt_codegen(d->code, file);

        // then unwind stack
        fprintf(file, "pop %%r15\n");
        fprintf(file, "pop %%r14\n");
        fprintf(file, "pop %%r13\n");
        fprintf(file, "pop %%r12\n");
        fprintf(file, "pop %%rbx\n");
        fprintf(file, "mov %%rbp, %%rsp\n");
        fprintf(file, "pop %%rbp\n");
        fprintf(file, "ret\n");

    } else if (d->symbol->kind == SYMBOL_LOCAL) {
        // local data
        // if there is initialization, set the value
        if (d->value) {
            // compute value
            expr_codegen(d->value, file);

            // load value
            fprintf(file, "mov %s, %s\n", register_name(d->value->reg), symbol_code(d->symbol));

            // reclaim register
            register_free(d->value->reg);
            d->value->reg = -1;
        }
        // otherwise, do nothing!

    } else {
        // this shouldn't happen
        printf("fatal error: unexpected declaration\n");
        decl_print(d, 0);
        exit(1);
    }
}

#undef FN_MANGLE_PREFIX
