#include <stdio.h>      // printf, fopen, fclose
#include <stdlib.h>     // exit
#include <unistd.h>     // getopt
#include <getopt.h>     // getopt
#include "utility.h"    // token to string
#include "lex.yy.h"     // yylex
#include "parser.tab.h" // yyparse

// Macro to setup options for getopt
#define SETUP_OPT_STRUCT(__struct_name, __idx, __name, __val)   \
    (__struct_name)[(__idx)].name = (__name);                   \
    (__struct_name)[(__idx)].has_arg = 0;                       \
    (__struct_name)[(__idx)].flag = NULL;                       \
    (__struct_name)[(__idx)].val = (__val);

// Redirect lex input to a file
extern FILE *yyin;

// Parse procedure
extern int yyparse();
int yydebug = 0;

// Parse result
#include "decl.h"

// Name resolution
#include "scope.h"
struct table_node *scope_table_list = NULL;

// Type check result
#include "type.h"
int __print_name_resolution_result = 0;
unsigned int type_error_count = 0;

enum _cminor_options {
    LEX = 1,
    PARSE = 2,
    RESOLVE = 3,
};

void _lex_manual();
void _parse();
void _resolve_name();

int main(int argc, char* argv[]) {

    // handle command line arguments
    int i = 0;
    int opt = -1;
    const char *optstring = "";

    // setup long arguments
    struct option options_spec[3];
    SETUP_OPT_STRUCT(options_spec, 0, "scan", LEX);
    SETUP_OPT_STRUCT(options_spec, 1, "print", PARSE);
    SETUP_OPT_STRUCT(options_spec, 2, "resolve", RESOLVE);

    // process flags
    while ((i = getopt_long_only(argc, argv, optstring, options_spec, NULL)) != -1) {
        if (opt != -1) {
            fprintf(stderr, "cminor: received multiple flags\n");
            exit(1);
        }
        if (i == '?') exit(1);
        opt = i;
    }
    if (opt == -1) {
        fprintf(stderr, "cminor: must pass in at least one flag\n");
        exit(1);
    }

    // consider only the first file requested
    if (optind >= argc) {
        fprintf(stderr, "cminor: no file given\n");
        exit(1);
    } else if (optind != argc - 1) {
        fprintf(stderr, "cminor: multiple files given, processing only the first file\n");
    }

    // use file
    const char *filename = argv[optind];
    FILE *source_file = fopen(filename, "r");
    if (!source_file) {
        fprintf(stderr, "cminor: cannot open file %s\n", filename);
        exit(1);
    }
    yyin = source_file;

    // perform action
    switch (opt) {
        case LEX:
            _lex_manual();
            break;
        case PARSE:
            _parse();
            decl_print(program, 0);
            break;
        case RESOLVE:
            __print_name_resolution_result = 1;
            _resolve_name();
            break;
    }

    fclose(source_file);

    return 0;
}


void _lex_manual() {
    int token;
    YYSTYPE yylval;
    while ((token = yylex(&yylval)) != 0) {
        // Process token
        printf("%s", token_to_string(token));
        if (token == INTEGER_LITERAL) {
            printf(" %lld", lexer_val.int_value);
        } else if (token == CHAR_LITERAL) {
            printf(" %c", lexer_val.char_value);
        } else if (token == STRING_LITERAL) {
            printf(" %s", _global_string_buffer);
        }

        printf("\n");
    }
}

void _parse() {
    program = NULL;
    if (yyparse() != 0) exit(1);
}

void _resolve_name() {
    _parse();
    type_error_count = 0;

    // create global scope
    scope_table_list = table_node_push(scope_table_list, SYMBOL_GLOBAL);

    decl_resolve(program, -1);
    if (type_error_count > 0) {
        // we have type errors
        if (type_error_count == 1) fprintf(stderr, "encountered 1 type error\n");
        else fprintf(stderr, "encountered %u type errors\n", type_error_count);

        exit(1);
    }
}
