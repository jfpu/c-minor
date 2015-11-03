#include <stdio.h>

/* Token to string utility */
#include "parser.tab.h"

const char *token_to_string(enum yytokentype token);

// Type definition for lexer
typedef union _lexer_value_t {
    long long int int_value;
    char char_value;
    char *identifier_symbol;
    unsigned int string_buffer_index;
} lexer_value_t;
lexer_value_t lexer_val;

// Shared character buffer for strings
#define MAX_STRING_LENGTH 256
char _global_string_buffer[MAX_STRING_LENGTH];

// String routines
#define BUFFER_APPEND(c) _global_string_buffer[(lexer_val.string_buffer_index++)] = (c)

// Make sure matched identifiers are not too long
#define MAX_IDENTIFIER_LENGTH 256

// Error handling routine
#define ERROR(f_, ...) {                            \
    fprintf(stderr, "SCAN ERROR (%d) ", yylineno);  \
    fprintf(stderr, (f_), __VA_ARGS__);             \
    fprintf(stderr, "\n");                          \
    exit(1);                                        \
}
#define ERROR_NOARGS(f_) {                          \
    fprintf(stderr, "SCAN ERROR (%d) ", yylineno);  \
    fprintf(stderr, (f_));                          \
    fprintf(stderr, "\n");                          \
    exit(1);                                        \
}

/* Print indentation */
void _print_indent(int indent);
