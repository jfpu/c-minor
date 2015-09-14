#include <stdio.h>
#include <stdlib.h>
#include "lex.yy.c"

extern const char *token_manifest_to_string(int token);

int main(int argc, char* argv[]) {
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
    return 0;
}
