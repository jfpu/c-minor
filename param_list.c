#include <stdlib.h> // malloc
#include <string.h> // memset, strdup
#include "param_list.h"
#include "type.h"
#include "symbol.h"

struct param_list *param_list_create(char *name, struct type *type, struct param_list *next) {
    struct param_list *p = (struct param_list *)malloc(sizeof(*p));
    memset(p, 0, sizeof(*p));

    p->name = name;
    p->type = type;
    p->next = next;
    return p;
}

struct param_list *param_list_prepend(struct param_list *first, struct param_list *rest) {
    first->next = rest;
    return first;
}

void param_list_print(struct param_list *a) {
    if (!a) return;
    printf("%s: ", a->name);
    type_print(a->type);
    if (a->next) {
        printf(", ");
        param_list_print(a->next);
    }
}

// for type checking
unsigned int param_list_length(struct param_list *p) {
    struct param_list *p_ptr = p;
    unsigned int length = 0;

    while (p_ptr) {
        ++length;
        p_ptr = p_ptr->next;
    }

    return length;
}

struct param_list *param_list_copy(struct param_list *p) {
    if (!p) return NULL;
    struct param_list *new_param_list = param_list_create(strdup(p->name), type_copy(p->type), param_list_copy(p->next));
    return new_param_list;
}

void param_list_delete(struct param_list *p) {
    if (!p) return;

    // delete from end of list
    param_list_delete(p->next);

    // free node
    free(p->name);
    type_delete(p->type);
    free(p);
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
