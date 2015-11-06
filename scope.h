#ifndef SCOPE_H
#define SCOPE_H

#include "hash_table.h"     // we're maintaining a stack of hash tables
#include "symbol.h"

struct table_node {
    struct hash_table *table;
    struct table_node *next;
};
struct table_node *scope_table_list = NULL;
struct table_node *table_node_push(struct table_node *list);
struct table_node *table_node_pop(struct table_node *list);

void scope_enter();
void scope_exit();
void scope_bind(const char *name, struct symbol *s);
struct symbol *scope_lookup(const char *name);

#endif
