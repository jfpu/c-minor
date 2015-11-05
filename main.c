#include <stdio.h>      // printf, fopen, fclose
#include <stdlib.h>     // exit
#include <unistd.h>     // getopt
#include <getopt.h>     // getopt
#include "utility.h"    // token to string
#include "lex.yy.h"     // yylex
#include "parser.tab.h" // yyparse

// Redirect lex input to a file
extern FILE *yyin;

// Parse procedure
extern int yyparse();
int yydebug = 0;

// Parse result
#include "decl.h"

enum _cminor_options {
    LEX = 1,
    PARSE = 2,
};

void _lex();
void _parse();

int main(int argc, char* argv[]) {

    // handle command line arguments
    int opt;
    const char *optstring = "";

    // setup long arguments
    struct option options_spec[2];
    options_spec[0].name = "scan";
    options_spec[0].has_arg = 1;
    options_spec[0].flag = NULL;
    options_spec[0].val = LEX;

    options_spec[1].name = "parse";
    options_spec[1].has_arg = 1;
    options_spec[1].flag = NULL;
    options_spec[1].val = PARSE;

    while ((opt = getopt_long_only(argc, argv, optstring, options_spec, NULL)) != -1) {
        if (opt == LEX || opt == PARSE) {
            // Use file
            FILE *source_file = fopen(optarg, "r");
            if (!source_file) {
                fprintf(stderr, "Cannot open file %s\n", optarg);
                exit(1);
            }
            yyin = source_file;

            if (opt == LEX) _lex();
            else if (opt == PARSE) _parse();

            fclose(source_file);
        }
    }

    return 0;
}


void _lex() {
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
    decl_print(program, 0);
}
