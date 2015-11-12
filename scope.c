#include <stdlib.h> // malloc
#include "scope.h"

struct table_node *table_node_push(struct table_node *list, symbol_t scope) {
    struct table_node *n = (struct table_node *)malloc(sizeof(*n));
    n->table = hash_table_create(0, 0);
    n->scope = scope;
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

// scope operation
void scope_enter() {
    // enter a new scope
    // prepend a new hashtable to the beginning of scope list
    scope_table_list = table_node_push(scope_table_list, SYMBOL_LOCAL);
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

struct symbol *scope_lookup_current(const char *name) {
    // looks up a name only from the current scope
    return (struct symbol *)hash_table_lookup(scope_table_list->table, name);
}

// name resolution
void expr_resolve(struct expr *e) {
    if (!e) return;

    switch (e->kind) {
        case EXPR_BOOLEAN:
        case EXPR_INTEGER:
        case EXPR_CHARACTER:
        case EXPR_STRING:
            // we don't need to resolve literals
            break;

        case EXPR_NAME: {
            // name resolution
            struct symbol *resolved = scope_lookup(e->name);
            if (!resolved) {
                printf("name error: %s is not defined in the current scope\n", e->name);
                ++error_count_name;
            }
            if (__print_name_resolution_result) print_name_resolution(resolved);
            e->symbol = resolved;
            break;
        }

        default: {
            // otherwise we resolve both sides
            expr_resolve(e->left);
            expr_resolve(e->right);
            return;
        }
    }

    // if it's a list, we resolve the next one
    if (e->next) expr_resolve(e->next);
}

void function_param_resolve(struct type *t) {
    // only for functions
    if (!t) return;
    if (t->kind != TYPE_FUNCTION) return;

    // resolve parameters
    int param_count = 0;
    struct param_list *p_ptr = t->params;
    while (p_ptr) {
        if (scope_lookup_current(p_ptr->name)) {
            // if the name already exists in current scope, error
            ++error_count_type;
            printf("name error: duplicate parameter name %s\n", p_ptr->name);

            // move on to next parameter
            p_ptr = p_ptr->next;
            continue;
        }

        // create symbol
        struct symbol *p_sym = symbol_create(SYMBOL_PARAM, param_count, p_ptr->type, p_ptr->name);
        scope_bind(p_ptr->name, p_sym);
        p_ptr->symbol = p_sym;
        if (__print_name_resolution_result) {
            printf("create symbol: ");
            print_name_resolution(p_sym);
        }

        // next
        ++param_count;
        p_ptr = p_ptr->next;
    }
}

void print_name_resolution(struct symbol *s) {
    if (!s) return;
    printf("%s resolves to ", s->name);
    switch (s->kind) {
        case SYMBOL_LOCAL:
            printf("local %d\n", s->which);
            break;
        case SYMBOL_PARAM:
            printf("param %d\n", s->which);
            break;
        case SYMBOL_GLOBAL:
            printf("global %s\n", s->name);
            break;
        default:
            printf("error\n");
    }
}
