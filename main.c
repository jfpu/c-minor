#include <stdio.h>      // printf, fopen, fclose
#include <stdlib.h>     // exit
#include <unistd.h>     // getopt
#include <getopt.h>     // getopt
#include "utility.h"
#include "lex.yy.h"     // yylex
#include "parser.tab.h" // yyparse

// Redirect lex input to a file
extern FILE *yyin;

enum _cminor_options {
    LEX = 1,
};

void _lex();

int main(int argc, char* argv[]) {

    // handle command line arguments
    int opt;
    const char *optstring = "";

    // setup long arguments
    struct option options_spec[1];
    options_spec[0].name = "scan";
    options_spec[0].has_arg = 1;
    options_spec[0].flag = NULL;
    options_spec[0].val = LEX;

    while ((opt = getopt_long_only(argc, argv, optstring, options_spec, NULL)) != -1) {
        if (opt == LEX) {
            // Use file
            FILE *source_file = fopen(optarg, "r");
            if (!source_file) {
                fprintf(stderr, "Cannot open file %s\n", optarg);
                exit(1);
            }

            yyin = source_file;
            _lex();
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
        printf("%s", token_manifest_to_string(token));
        if (token == INTEGER_LITERAL) {
            printf(" %lld", yylval.int_value);
        } else if (token == CHAR_LITERAL) {
            printf(" %c", yylval.char_value);
        } else if (token == STRING_LITERAL) {
            printf(" %s", _global_string_buffer);
        }

        printf("\n");
    }
}
