/* Parser for c-minor */

%defines

/* Token types taken from previous token manifest */
%token ARRAY
%token BOOLEAN
%token CHAR
%token ELSE
%token FALSE
%token FOR
%token FUNCTION
%token IF
%token INTEGER
%token PRINT
%token RETURN
%token STRING
%token TRUE
%token VOID
%token WHILE
%token INTEGER_LITERAL
%token BOOLEAN_LITERAL
%token CHAR_LITERAL
%token STRING_LITERAL
%token IDENTIFIER
%token LPAREN
%token RPAREN
%token LBRACKET
%token RBRACKET
%token OP_INC
%token OP_DEC
%token OP_MINUS
%token OP_EXP
%token OP_MULT
%token OP_DIV
%token OP_MOD
%token OP_PLUS
%token OP_LT
%token OP_LE
%token OP_GT
%token OP_GE
%token OP_EQ
%token OP_NE
%token OP_LAND
%token OP_LOR
%token OP_ASSIGN
%token LCBRACK
%token RCBRACK
%token SEMICOLON
%token COLON
%token COMMA

%union {
    long long int int_value;
    char char_value;
    char *identifier_symbol;
    unsigned int string_buffer_index;
}

%{
#include <stdio.h>
extern char *yytext;
extern int yylex();
void yyerror(char const *str);
%}

%%

prog: VOID
    ;

%%

void yyerror(char const *str) {
    fprintf(stderr, "PARSE ERROR: %s\n", str);
}
