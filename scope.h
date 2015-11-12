#ifndef SCOPE_H
#define SCOPE_H

#include "hash_table.h"     // we're maintaining a stack of hash tables
#include "decl.h"
#include "expr.h"
#include "param_list.h"
#include "stmt.h"
#include "type.h"
#include "symbol.h"

struct table_node {
    struct hash_table *table;
    symbol_t scope;
    struct table_node *next;
};
extern struct table_node *scope_table_list;
struct table_node *table_node_push(struct table_node *list, symbol_t scope);
struct table_node *table_node_pop(struct table_node *list);

// scope operation
void scope_enter();
void scope_exit();
void scope_bind(const char *name, struct symbol *s);
struct symbol *scope_lookup(const char *name);
struct symbol *scope_lookup_current(const char *name);

// name resolution
extern int __print_name_resolution_result;
extern unsigned int error_count_name;
void stmt_resolve(struct stmt *s, int which);
void expr_resolve(struct expr *e);
void function_param_resolve(struct type *t);

void print_name_resolution(struct symbol *s);

#endif
