#include <stdlib.h> // malloc
#include <string.h> // memset, strdup
#include "param_list.h"
#include "type.h"

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
