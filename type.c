#include <stdlib.h> // malloc
#include <string.h> // memset
#include "stmt.h"

struct type *type_create(type_kind_t kind, struct param_list *params, struct type *subtype) {
    struct type *t = (struct type *)malloc(sizeof(*t));
    memset(t, 0, sizeof(*t));

    t->kind = kind;
    t->params = params;
    t->subtype = subtype;
    return t;
}

void type_print(struct type *t) {

}
