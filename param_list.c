#include <stdlib.h> // malloc
#include <string.h> // memset
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
