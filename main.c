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
extern struct decl *program;

// Name resolution
#include "scope.h"
struct table_node *scope_table_list = NULL;

// Type check result
#include "type.h"
int __print_name_resolution_result = 0;
unsigned int error_count_name = 0;
unsigned int error_count_type = 0;

enum _cminor_options {
    LEX = 1,
    PARSE,
    RESOLVE,
    CHECK,
    COMPILE
};

// Codegen
int label_count = 0;

void _lex_manual();
void _parse();
void _resolve_name();
void _typecheck();
void _compile(FILE *outfile);

int main(int argc, char* argv[]) {

    // handle command line arguments
    int i = 0;
    int opt = -1;
    const char *optstring = "";

    // setup long arguments
    struct option options_spec[6];
    SETUP_OPT_STRUCT(options_spec, 0, "scan", LEX);
    SETUP_OPT_STRUCT(options_spec, 1, "print", PARSE);
    SETUP_OPT_STRUCT(options_spec, 2, "resolve", RESOLVE);
    SETUP_OPT_STRUCT(options_spec, 3, "typecheck", CHECK);
    SETUP_OPT_STRUCT(options_spec, 4, "compile", COMPILE);
    SETUP_OPT_STRUCT(options_spec, 5, 0, 0);

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
    if (optind >= argc) {
        fprintf(stderr, "cminor: no file given\n");
        exit(1);
    }

    // use file
    // first file is the infile, second (if given) is the outfile
    const char *infile = NULL, *outfile = NULL;
    if (opt == COMPILE) {
        infile = argv[optind];
        outfile = argv[optind + 1];
    } else {
        infile = argv[optind];
    }

    FILE *source_file = fopen(infile, "r");
    if (!source_file) {
        fprintf(stderr, "cminor: cannot open file %s\n", infile);
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
        case CHECK:
            _typecheck();
            break;
        case COMPILE: {
            FILE *assembly_file = fopen(outfile, "w");
            if (!assembly_file) {
                fprintf(stderr, "cminor: cannot create file %s\n", outfile);
                exit(1);
            }
            _compile(assembly_file);
            fclose(assembly_file);
            break;
        }
    }
    fclose(source_file);

    return 0;
}


void _lex_manual() {
    int token;
    YYSTYPE yylval;
    while ((token = yylex(&yylval)) != 0) {
        // Process token
        printf("%s", token_to_string((enum yytokentype)token));
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
    error_count_name = 0;

    // create global scope
    scope_table_list = table_node_push(scope_table_list, SYMBOL_GLOBAL);
    decl_resolve(program, NULL, -1);
    if (error_count_name > 0) {
        // we have name errors
        if (error_count_name == 1) printf("encountered 1 name error\n");
        else printf("encountered %u name errors\n", error_count_name);
        exit(1);
    }
}

void _typecheck() {
    _resolve_name();
    decl_typecheck(program);
    if (error_count_type > 0) {
        // we have type errors
        if (error_count_type == 1) printf("encountered 1 type error\n");
        else printf("encountered %u type errors\n", error_count_type);
        exit(1);
    }
}

void _compile(FILE *outfile) {
    label_count = 0;
    _typecheck();
    decl_codegen(program, outfile);
}
