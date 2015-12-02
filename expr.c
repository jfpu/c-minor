#include <stdlib.h> // malloc
#include <string.h> // memset
#include "expr.h"
#include "scope.h"
#include "symbol.h"
#include "register.h"

#ifdef __linux__
#define FN_MANGLE_PREFIX ""
#else
#define FN_MANGLE_PREFIX "_"
#endif

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
    e->reg = -1;
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

    struct expr *e_ptr = e;

    while (e_ptr) {
        expr_print_individual(e_ptr);
        e_ptr = e_ptr->next;
        if (e_ptr) {
            printf(", ");
        }
    }
}

void expr_print_individual(struct expr *e) {
    if (!e) return;

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

        case EXPR_STRING: {
            expr_string_print(e->string_literal, stdout);
        }
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
        || e->kind == EXPR_STRING
        || (e->kind == EXPR_NEG && expr_is_constant(e->right)));
}

int expr_is_lvalue_type(struct expr *e) {
    if (!e) return 0;
    return (e->kind == EXPR_NAME
        || e->kind == EXPR_ARRAY_DEREF);
}

void expr_resolve(struct expr *e) {
    if (!e) return;

    struct expr *e_ptr = e;
    while (e_ptr) {
        switch (e_ptr->kind) {
            case EXPR_BOOLEAN:
            case EXPR_INTEGER:
            case EXPR_CHARACTER:
            case EXPR_STRING:
                // we don't need to resolve literals
                break;

            case EXPR_NAME: {
                // name resolution
                struct symbol *resolved = scope_lookup(e_ptr->name);
                if (!resolved) {
                    printf("name error: %s is not defined in the current scope\n", e_ptr->name);
                    ++error_count_name;
                }
                if (__print_name_resolution_result) { print_name_resolution(resolved); }
                e_ptr->symbol = resolved;
                break;
            }

            default: {
                // otherwise we resolve both sides
                expr_resolve(e_ptr->left);
                expr_resolve(e_ptr->right);
                break;
            }
        }
        e_ptr = e_ptr->next;
    }
}

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
                printf("` is not an lvalue\n");
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
        case EXPR_DEC: {
            type_right = expr_typecheck(e->right);

            // inc dec only work on lvalues
            if (!expr_is_lvalue_type(e->right)) {
                ++error_count_type;
                printf("type error: expression `");
                expr_print(e->right);
                printf("` is not an lvalue\n");
            }

            // also only work on integers
            if (type_right->kind != TYPE_INTEGER) {
                ++error_count_type;
                printf("type error: cannot increment or decrement expression of type ");
                type_print(type_right);
                printf("\n");
            }
            TYPE_FREE(type_right);
            return type_create(TYPE_INTEGER, NULL, NULL);
        }

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

void expr_list_typecheck(struct expr *e, struct type *expected) {
    // this is for homogeneous expr_list's
    // if expected is NULL (for print), we only typecheck each individual expr
    // otherwise we compare the types too
    struct expr *e_ptr = e;
    while (e_ptr) {
        struct type *actual = expr_typecheck(e_ptr);
        if (expected && !type_is_equal(actual, expected)) {
            ++error_count_type;
            printf("type error: expression list received expression `");
            expr_print_individual(e_ptr);
            printf("` of type ");
            type_print(actual);
            printf(", expecting ");
            type_print(expected);
            printf("\n");
        }
        TYPE_FREE(actual);
        e_ptr = e_ptr->next;
    }
}

// for codegen
void expr_codegen(struct expr *e, FILE *file) {
    switch (e->kind) {
        case EXPR_INTEGER:
        case EXPR_CHARACTER:
        case EXPR_BOOLEAN: {
            e->reg = register_alloc();
            fprintf(file, "mov $%d, %s\n", e->literal_value, register_name(e->reg));
            break;
        }
        case EXPR_NAME: {
            e->reg = register_alloc();
            fprintf(file, "mov %s, %s\n", symbol_code(e->symbol), register_name(e->reg));
            break;
        }
        case EXPR_STRING: {
            e->reg = register_alloc();
            int string_label = label_count++;

            // switch into data section, create the string, and switch back and use it
            fprintf(file, ".data\n");
            fprintf(file, "str%d:\n", string_label);
            fprintf(file, ".asciz ");
            expr_string_print(e->string_literal, file);
            fprintf(file, "\n");

            fprintf(file, ".text\n");
            fprintf(file, "lea str%d(%%rip), %s\n", string_label, register_name(e->reg));
            break;
        }
        case EXPR_ADD:
        case EXPR_SUB: {
            // post-order traversal: we need the left and right children ready first
            expr_codegen(e->left, file);
            expr_codegen(e->right, file);

            // add/sub left with right
            const char *action = (e->kind == EXPR_ADD) ? "add" : "sub";
            fprintf(file, "%s %s, %s\n", action, register_name(e->right->reg), register_name(e->left->reg));

            // destructive: the right register has the result
            e->reg = e->left->reg;
            e->left->reg = -1;
            register_free(e->right->reg);
            e->right->reg = -1;
            break;
        }
        case EXPR_NEG: {
            // we need the right children
            expr_codegen(e->right, file);
            // negate right
            fprintf(file, "neg %s\n", register_name(e->right->reg));

            // register maneuver
            e->reg = e->right->reg;
            e->right->reg = -1;
            break;
        }
        case EXPR_ASSIGN: {
            // evaluate right
            expr_codegen(e->right, file);
            // assign value to left
            fprintf(file, "mov %s, %s\n", register_name(e->right->reg), symbol_code(e->left->symbol));

            // expr evaluates to right
            e->reg = e->right->reg;
            e->right->reg = -1;
            break;
        }
        case EXPR_MUL:
        case EXPR_DIV:
        case EXPR_MOD: {
            expr_codegen(e->left, file);
            expr_codegen(e->right, file);

            // move left register into %rax
            fprintf(file, "mov %s, %%rax\n", register_name(e->left->reg));

            if (e->kind == EXPR_MUL) {
                // multiply with the right register
                fprintf(file, "imul %s\n", register_name(e->right->reg));
            } else {
                // sign extend %rax
                fprintf(file, "cqo\n");
                // divide by right register
                fprintf(file, "idiv %s\n", register_name(e->right->reg));
            }

            if (e->kind == EXPR_MOD) {
                // move rdx into result register
                fprintf(file, "mov %%rdx, %s\n", register_name(e->right->reg));
            } else {
                // move rax into result register
                fprintf(file, "mov %%rax, %s\n", register_name(e->right->reg));
            }

            // register maneuver
            e->reg = e->right->reg;
            e->right->reg = -1;
            register_free(e->left->reg);
            e->left->reg = -1;
            break;
        }
        case EXPR_EXP: {
            // we're not natively implementing exp
            // instead we're using the "c-minor standard library"
            expr_codegen(e->left, file);
            expr_codegen(e->right, file);

            // call integer_power
            fprintf(file, "mov %s, %s\n", register_name(e->left->reg), param_register_name(0));
            register_free(e->left->reg);
            fprintf(file, "mov %s, %s\n", register_name(e->right->reg), param_register_name(1));
            register_free(e->right->reg);

            fprintf(file, "push %%r10\n");
            fprintf(file, "push %%r11\n");
            fprintf(file, "call %sinteger_power\n", FN_MANGLE_PREFIX);
            fprintf(file, "pop %%r11\n");
            fprintf(file, "pop %%r10\n");

            // store result
            e->reg = register_alloc();
            fprintf(file, "mov %%rax, %s\n", register_name(e->reg));
            break;
        }
        case EXPR_INC:
        case EXPR_DEC: {
            const char *action = (e->kind == EXPR_INC) ? "inc" : "dec";
            // evaluate e->right (name)
            expr_codegen(e->right, file);

            // claim a new register for result
            e->reg = register_alloc();
            // move expression's value to result register
            fprintf(file, "mov %s, %s\n", register_name(e->right->reg), register_name(e->reg));
            // increment/decrement
            fprintf(file, "%s %s\n", action, register_name(e->right->reg));
            // store incremented value back to variable
            fprintf(file, "mov %s, %s\n", register_name(e->right->reg), symbol_code(e->right->symbol));
            // free temporary register
            register_free(e->right->reg);
            e->right->reg = -1;
            break;
        }
        case EXPR_LAND:{
            int false_label = label_count++;
            int end_label = label_count++;

            // evaluate left
            expr_codegen(e->left, file);

            // if left is false, jump to false label
            fprintf(file, "cmp $0, %s\n", register_name(e->left->reg));
            fprintf(file, "je .label%d\n", false_label);

            // otherwise evaluate right
            expr_codegen(e->right, file);

            // if right is false, jump to false label
            fprintf(file, "cmp $0, %s\n", register_name(e->right->reg));
            fprintf(file, "je .label%d\n", false_label);

            // now both sides are true, evaluates to true
            fprintf(file, "mov $1, %s\n", register_name(e->left->reg));
            fprintf(file, "jmp .label%d\n", end_label);

            // false label: evaluates to false
            fprintf(file, ".label%d:\n", false_label);
            fprintf(file, "mov $0, %s\n", register_name(e->left->reg));

            // end label
            fprintf(file, ".label%d:\n", end_label);

            // reclaim registers
            e->reg = e->left->reg;
            e->left->reg = -1;
            register_free(e->right->reg);
            e->right->reg = -1;
            break;
        }
        case EXPR_LOR: {
            int true_label = label_count++;
            int end_label = label_count++;

            // evaluate left
            expr_codegen(e->left, file);

            // if left is true, jump to true label
            fprintf(file, "cmp $1, %s\n", register_name(e->left->reg));
            fprintf(file, "je .label%d\n", true_label);

            // otherwise evaluate right
            expr_codegen(e->right, file);

            // if right is true, jump to true label
            fprintf(file, "cmp $1, %s\n", register_name(e->right->reg));
            fprintf(file, "je .label%d\n", true_label);

            // now both sides are false, evaluates to false
            fprintf(file, "mov $0, %s\n", register_name(e->left->reg));
            fprintf(file, "jmp .label%d\n", end_label);

            // true label: evaluates to true
            fprintf(file, ".label%d:\n", true_label);
            fprintf(file, "mov $1, %s\n", register_name(e->left->reg));

            // end label
            fprintf(file, ".label%d:\n", end_label);

            // reclaim registers
            e->reg = e->left->reg;
            e->left->reg = -1;
            register_free(e->right->reg);
            e->right->reg = -1;
            break;
        }
        case EXPR_LNOT: {
            // evaluate right
            expr_codegen(e->right, file);

            // flip right->reg
            fprintf(file, "sub $1, %s\n", register_name(e->right->reg));
            fprintf(file, "sbb %s, %s\n", register_name(e->right->reg), register_name(e->right->reg));
            fprintf(file, "and $1, %s\n", register_name(e->right->reg));

            // reclaim registers
            e->reg = e->right->reg;
            e->right->reg = -1;
            break;
        }
        case EXPR_LT:
        case EXPR_LE:
        case EXPR_GT:
        case EXPR_GE:{
            int true_label = label_count++;
            int end_label = label_count++;

            // evaluate both sides
            expr_codegen(e->left, file);
            expr_codegen(e->right, file);

            const char *jump_action;
            if (e->kind == EXPR_LT) {
                jump_action = "jl";
            } else if (e->kind == EXPR_LE) {
                jump_action = "jle";
            } else if (e->kind == EXPR_GT) {
                jump_action = "jg";
            } else {
                jump_action = "jge";
            }

            e->reg = e->right->reg;
            fprintf(file, "cmp %s, %s\n", register_name(e->right->reg), register_name(e->left->reg));
            fprintf(file, "%s label%d\n", jump_action, true_label);
            fprintf(file, "mov $0, %s\n", register_name(e->reg));
            fprintf(file, "jmp label%d\n", end_label);
            fprintf(file, "label%d:\n", true_label);
            fprintf(file, "mov $1, %s\n", register_name(e->reg));
            fprintf(file, "label%d:\n", end_label);

            // reclaim registers
            e->right->reg = -1;
            register_free(e->left->reg);
            e->left->reg = -1;
            break;
        }
        case EXPR_EQ:
        case EXPR_NE: {
            expr_codegen(e->left, file);
            expr_codegen(e->right, file);

            struct type *t = expr_typecheck(e->left);
            if (t->kind == TYPE_STRING) {
                // Call runtime string comparison function
                fprintf(file, "mov %s, %s\n", register_name(e->left->reg), param_register_name(0));
                fprintf(file, "mov %s, %s\n", register_name(e->right->reg), param_register_name(1));

                fprintf(file, "push %%r10\n");
                fprintf(file, "push %%r11\n");
                fprintf(file, "call %sstring_cmp\n", FN_MANGLE_PREFIX);
                fprintf(file, "pop %%r11\n");
                fprintf(file, "pop %%r10\n");

                // store result
                if (e->kind == EXPR_EQ) {
                    e->reg = e->right->reg;
                    fprintf(file, "mov %%rax, %s\n", register_name(e->reg));
                } else {
                    e->reg = e->right->reg;
                    // flip %rax
                    fprintf(file, "sub $1, %%rax\n");
                    fprintf(file, "sbb %%rax, %%rax\n");
                    fprintf(file, "and $1, %%rax\n");
                    fprintf(file, "mov %%rax, %s\n", register_name(e->reg));
                }
            } else {
                // Compare values directly
                int true_label = label_count++;
                int end_label = label_count++;

                const char *jump_action;
                if (e->kind == EXPR_EQ) {
                    jump_action = "je";
                } else {
                    jump_action = "jne";
                }

                e->reg = e->right->reg;
                fprintf(file, "cmp %s, %s\n", register_name(e->right->reg), register_name(e->left->reg));
                fprintf(file, "%s label%d\n", jump_action, true_label);
                fprintf(file, "mov $0, %s\n", register_name(e->reg));
                fprintf(file, "jmp label%d\n", end_label);
                fprintf(file, "label%d:\n", true_label);
                fprintf(file, "mov $1, %s\n", register_name(e->reg));
                fprintf(file, "label%d:\n", end_label);
            }
            TYPE_FREE(t);

            // reclaim registers
            e->right->reg = -1;
            register_free(e->left->reg);
            e->left->reg = -1;
            break;
        }
        case EXPR_FCALL: {
            // e_ptr is the list of arguments
            struct expr *e_ptr = e->right;
            int arg_count = 0;
            while (e_ptr) {
                if (arg_count >= 6) {
                    printf("error: functions with over 6 arguments are not supported\n");
                    exit(1);
                }

                // for each argument, codegen
                expr_codegen(e_ptr, file);
                // store
                fprintf(file, "mov %s, %s\n", register_name(e_ptr->reg), param_register_name(arg_count));
                // clean up temporary register
                register_free(e_ptr->reg);
                e_ptr->reg = -1;

                // move on
                e_ptr = e_ptr->next;
                ++arg_count;
            }

            // push caller save registers (r10, r11)
            fprintf(file, "push %%r10\n");
            fprintf(file, "push %%r11\n");

            // call function
            fprintf(file, "call %s%s\n", e->left->name, FN_MANGLE_PREFIX);

            // pop caller save registers
            fprintf(file, "pop %%r11\n");
            fprintf(file, "pop %%r10\n");

            // store result
            e->reg = register_alloc();
            fprintf(file, "mov %%rax, %s\n", register_name(e->reg));
            break;
        }
        case EXPR_ARRAY_DEREF: {
            // don't need to worry about arrays!
            printf("error: arrays are not supported\n");
            exit(1);
        }
        default:
            break;
    }
}

void expr_string_print(const char * const str, FILE *file) {
    int i = 0;
    fprintf(file, "\"");
    while (str[i]) {
        if (str[i] == '\n') {
            fprintf(file, "\\n");
        } else {
            fprintf(file, "%c", str[i]);
        }
        ++i;
    }
    fprintf(file, "\"");
}

#undef FN_MANGLE_PREFIX
