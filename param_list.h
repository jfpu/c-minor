#ifndef PARAM_LIST_H
#define PARAM_LIST_H

#include "type.h"
#include <stdio.h>

struct expr;

struct param_list {
    char *name;
    struct type *type;
    struct symbol *symbol;
    struct param_list *next;
};

struct param_list *param_list_create(char *name, struct type *type, struct param_list *next);
struct param_list *param_list_prepend(struct param_list *first, struct param_list *rest);
void param_list_print(struct param_list *a);

// for type checking
unsigned int param_list_length(struct param_list *p);
struct param_list *param_list_copy(struct param_list *p);
void param_list_delete(struct param_list *p);

#endif
