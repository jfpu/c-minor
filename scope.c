#include <stdlib.h> // malloc
#include "scope.h"

struct table_node *table_node_push(struct table_node *list) {
    struct table_node *n = (struct table_node *)malloc(sizeof(*n));
    n->table = hash_table_create(0, 0);
    n->next = list;
    return n;
}

struct table_node *table_node_pop(struct table_node *list) {
    if (!list) {
        // ideally this should never happen
        fprintf(stderr, "attempting to pop when there is no scope in stack\n");
        exit(1);
    }

    struct table_node *n = list->next;

    hash_table_delete(list->table);
    list->next = NULL;
    free(list);

    return n;
}

void scope_enter() {
    // enter a new scope
    // prepend a new hashtable to the beginning of scope list
    scope_table_list = table_node_push(scope_table_list);
}

void scope_exit() {
    // exit from current scope
    // pop the front table
    scope_table_list = table_node_pop(scope_table_list);
}

void scope_bind(const char *name, struct symbol *s) {
    // bind a symbol to a name in the current scope
    // if the name exists in the current scope, this will silently overwrite the existing binding
    if (!scope_table_list) {
        // this should never happen
        fprintf(stderr, "no existing scope\n");
    }

    struct hash_table *currnet_scope = scope_table_list->table;
    if (!hash_table_insert(currnet_scope, name, s)) {
        // this ideally should never happen
        fprintf(stderr, "cannot insert into hash table\n");
    }
}

struct symbol *scope_lookup(const char *name) {
    // looks up a name from the list of scopes, returning the corresponding symbol
    struct table_node *current_scope_node = scope_table_list;
    struct symbol *sym = NULL;

    while (current_scope_node) {
        sym = (struct symbol *)hash_table_lookup(current_scope_node->table, name);
        if (sym) return sym;

        current_scope_node = current_scope_node->next;
    }

    return NULL;
}
