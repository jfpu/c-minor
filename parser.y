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
%token OP_LNOT
%token OP_ASSIGN
%token LCBRACK
%token RCBRACK
%token SEMICOLON
%token COLON
%token COMMA

%{
#include <stdio.h>
extern char *yytext;
extern int yylex();
void yyerror(char const *str);
%}

%%

prog
:   /* empty */
|   decl_list
;

decl_list
:   decl decl_list
|   decl
;

decl
:   decl_non_func
|   decl_func
;

decl_non_func
:   IDENTIFIER COLON type SEMICOLON
|   IDENTIFIER COLON non_array_type OP_ASSIGN expr SEMICOLON
|   IDENTIFIER COLON array_type OP_ASSIGN LCBRACK expr_list RCBRACK SEMICOLON
;

decl_func
:   IDENTIFIER COLON FUNCTION ret_type LPAREN formal_list RPAREN SEMICOLON
|   IDENTIFIER COLON FUNCTION ret_type LPAREN formal_list RPAREN OP_ASSIGN block
;

stmt_list
:   stmt stmt_list
|   stmt
;

stmt
:   block
|   decl_non_func
|   IF LPAREN expr RPAREN block
|   IF LPAREN expr RPAREN block ELSE block
|   FOR LPAREN expr SEMICOLON expr SEMICOLON RPAREN block
|   RETURN expr SEMICOLON
|   PRINT expr_list SEMICOLON
|   expr SEMICOLON;
;

block
:   LCBRACK stmt_list RCBRACK
;

formal_list
:   formal COMMA formal_list
|   formal
|   /* empty */
;

formal
    /* parameter */
:   IDENTIFIER COLON type
;

type
:   non_array_type
|   array_type
;

non_array_type
:   BOOLEAN
|   INTEGER
|   CHAR
|   STRING
;

array_type
:   ARRAY LBRACKET RBRACKET non_array_type
;

ret_type
:   type
|   VOID
;

expr_list
:   expr COMMA expr_list
|   expr
;

expr
:   CHAR_LITERAL
|   STRING_LITERAL
|   arithmetic_expr
|   boolean_expr
;

/* arithmetic expression with proper precedence */
arithmetic_expr
:   arithmetic_term OP_PLUS arithmetic_expr
|   arithmetic_term OP_MINUS arithmetic_expr
|   arithmetic_term
;

arithmetic_term
:   arithmetic_factor OP_MULT arithmetic_term
|   arithmetic_factor OP_DIV arithmetic_term
|   arithmetic_factor
;

arithmetic_factor
:   INTEGER_LITERAL
|   IDENTIFIER
|   func_call
|   LPAREN arithmetic_expr RPAREN
|   OP_MINUS arithmetic_factor
;

func_call
:   IDENTIFIER LPAREN expr_list RPAREN
|   IDENTIFIER LPAREN RPAREN
;

/* boolean exprs */
boolean_expr
:   boolean_factor OP_LAND boolean_expr
|   boolean_factor OP_LOR boolean_expr
|   boolean_factor
;

boolean_factor
:   BOOLEAN_LITERAL
|   LPAREN boolean_expr RPAREN
|   arithmetic_expr OP_LT arithmetic_expr
|   arithmetic_expr OP_LE arithmetic_expr
|   arithmetic_expr OP_GT arithmetic_expr
|   arithmetic_expr OP_GE arithmetic_expr
|   arithmetic_expr OP_EQ arithmetic_expr
|   arithmetic_expr OP_NE arithmetic_expr
;

%%

void yyerror(char const *str) {
    fprintf(stderr, "PARSE ERROR: %s\n", str);
}
