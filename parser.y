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

#include "stmt.h"
#include "decl.h"
#include "expr.h"
#include "type.h"
#include "param_list.h"
%}

%union {
    /* for parser */
    struct stmt *stmt;
    struct decl *decl;
    struct expr *expr;
    struct param_list *formal;
    struct type *type;
    struct symbol *symbol;
}

%type <decl> prog decl_list decl
%type <stmt> stmt_list stmt block stmt_non_if_else stmt_if_else_matched stmt_if_else_open
%type <expr> expr_list expr expr_opt arithmetic_expr arithmetic_term arithmetic_factor_with_exponentiation arithmetic_factor func_call boolean_expr boolean_factor
%type <formal> formal_list nonempty_formal_list formal
%type <type> type non_array_type array_type ret_type func_type
%type <symbol> symbol

%%

prog
:   /* empty */
    { $$ = NULL; return 0; }
|   decl_list
    { $$ = $1; return 0; }
;

decl_list
:   decl decl_list
    { $$ = decl_list_prepend($1, $2); }
|   decl
    { $$ = $1; }
;

decl
:   symbol COLON type SEMICOLON
    { $$ = decl_create($1, $3, NULL, NULL, NULL); }
|   symbol COLON non_array_type OP_ASSIGN expr SEMICOLON
    { $$ = decl_create($1, $3, $5, NULL, NULL); }
|   symbol COLON array_type OP_ASSIGN LCBRACK expr_list RCBRACK SEMICOLON
    { $$ = decl_create($1, $3, $6, NULL, NULL); }
|   symbol COLON func_type OP_ASSIGN block
    { $$ = decl_create($1, $3, NULL, $5, NULL); }
;

stmt_list
:   stmt stmt_list
    { $$ = stmt_list_prepend($1, $2); }
|   stmt
    { $$ = $1; }
;

stmt
:   stmt_if_else_matched
    { $$ = $1; }
|   stmt_if_else_open
    { $$ = $1; }
;

stmt_if_else_matched
:   IF LPAREN expr RPAREN stmt_if_else_matched ELSE stmt_if_else_matched
    { $$ = stmt_create(STMT_IF_ELSE, NULL, NULL, $3, NULL, $5, $7); }
|   stmt_non_if_else
    { $$ = $1; }
;

stmt_if_else_open
:   IF LPAREN expr RPAREN stmt
    { $$ = stmt_create(STMT_IF_ELSE, NULL, NULL, $3, NULL, $5, NULL); }
|   IF LPAREN expr RPAREN stmt_if_else_matched ELSE stmt_if_else_open
    { $$ = stmt_create(STMT_IF_ELSE, NULL, NULL, $3, NULL, $5, $7); }
;

stmt_non_if_else
:   block
    { $$ = stmt_create(STMT_BLOCK, NULL, NULL, NULL, NULL, $1, NULL); }
|   decl
    { $$ = stmt_create(STMT_DECL, $1, NULL, NULL, NULL, NULL, NULL); }
|   FOR LPAREN expr_opt SEMICOLON expr_opt SEMICOLON expr_opt RPAREN stmt
    { $$ = stmt_create(STMT_FOR, NULL, $3, $5, $7, $9, NULL); }
|   RETURN expr SEMICOLON
    { $$ = stmt_create(STMT_RETURN, NULL, NULL, $2, NULL, NULL, NULL); }
|   PRINT expr_list SEMICOLON
    { $$ = stmt_create(STMT_PRINT, NULL, NULL, $2, NULL, NULL, NULL); }
|   expr SEMICOLON
    { $$ = stmt_create(STMT_EXPR, NULL, NULL, $1, NULL, NULL, NULL); }
;

block
:   LCBRACK stmt_list RCBRACK
    { $$ = $2; }
;

formal_list
:   /* empty */
    { $$ = NULL; }
|   nonempty_formal_list
    { $$ = $1; }

nonempty_formal_list
:   formal COMMA nonempty_formal_list
    { $$ = param_list_prepend($1, $3); }
|   formal
    { $$ = $1; }
;

formal
    /* parameter */
:   symbol COLON type
    { $$ = param_list_create($1, $3, NULL); }
;

type
:   non_array_type
    { $$ = $1; }
|   array_type
    { $$ = $1; }
|   func_type
    { $$ = $1; }
;

non_array_type
:   BOOLEAN
    { $$ = type_create(TYPE_BOOLEAN, NULL, NULL); }
|   INTEGER
    { $$ = type_create(TYPE_INTEGER, NULL, NULL); }
|   CHAR
    { $$ = type_create(TYPE_CHARACTER, NULL, NULL); }
|   STRING
    { $$ = type_create(TYPE_STRING, NULL, NULL); }
;

array_type
    /* QUESTION: do we support nested arrays? */
:   ARRAY LBRACKET expr_opt RBRACKET type
    { $$ = type_create(TYPE_ARRAY, NULL, $5); }
;

func_type
:   FUNCTION ret_type LPAREN formal_list RPAREN
    { $$ = type_create(TYPE_FUNCTION, $4, $2); }
;

ret_type
:   type
    { $$ = $1; }
|   VOID
    { $$ = type_create(TYPE_VOID, NULL, NULL); }
;

expr_list
:   expr COMMA expr_list
    { $$ = expr_list_prepend($1, $3); }
|   expr
    { $$ = $1; }
;

expr
:   CHAR_LITERAL
    { $$ = expr_create_character_literal(lexer_val.char_value); }
|   STRING_LITERAL
    { $$ = expr_create_string_literal(_global_string_buffer); }
|   arithmetic_expr
    { $$ = $1; }
|   boolean_expr
    { $$ = $1; }
;

expr_opt
:   expr
    { $$ = $1; }
|   /* empty */
    { $$ = NULL; }
;

/* arithmetic expression with proper precedence */
arithmetic_expr
:   arithmetic_term OP_PLUS arithmetic_expr
    { $$ = expr_create(EXPR_ADD, $1, $3); }
|   arithmetic_term OP_MINUS arithmetic_expr
    { $$ = expr_create(EXPR_SUB, $1, $3); }
|   arithmetic_term
    { $$ = $1; }
;

arithmetic_term
:   arithmetic_factor_with_exponentiation OP_MULT arithmetic_term
    { $$ = expr_create(EXPR_MUL, $1, $3); }
|   arithmetic_factor_with_exponentiation OP_DIV arithmetic_term
    { $$ = expr_create(EXPR_DIV, $1, $3); }
|   arithmetic_factor_with_exponentiation OP_MOD arithmetic_term
    { $$ = expr_create(EXPR_MOD, $1, $3); }
|   arithmetic_factor_with_exponentiation
    { $$ = $1; }
;

arithmetic_factor_with_exponentiation
:   arithmetic_factor OP_EXP arithmetic_factor_with_exponentiation
    { $$ = expr_create(EXPR_EXP, $1, $3); }
|   arithmetic_factor
    { $$ = $1; }
;

arithmetic_factor
:   INTEGER_LITERAL
    { $$ = expr_create_integer_literal(lexer_val.int_value); }
|   symbol
    { $$ = expr_create_symbol($1); }
|   func_call
    { $$ = $1; }
|   LPAREN arithmetic_expr RPAREN
    { $$ = $2; }
|   OP_MINUS arithmetic_factor
    { $$ = expr_create(EXPR_SUB, NULL, $2); }
|   symbol OP_INC
    { $$ = expr_create(EXPR_INC, NULL, $1); }
|   symbol OP_DEC
    { $$ = expr_create(EXPR_DEC, NULL, $1); }
;

func_call
    /* need to fix all of these */
:   symbol LPAREN expr_list RPAREN
    { $$ = NULL; }
|   symbol LPAREN RPAREN
    { $$ = NULL; }
;

/* boolean exprs */
boolean_expr
:   boolean_factor OP_LAND boolean_expr
    { $$ = expr_create(EXPR_LAND, $1, $3); }
|   boolean_factor OP_LOR boolean_expr
    { $$ = expr_create(EXPR_LOR, $1, $3); }
|   boolean_factor
    { $$ = $1; }
;

boolean_factor
:   TRUE
    { $$ = expr_create_boolean_literal(1); }
|   FALSE
    { $$ = expr_create_boolean_literal(0); }
|   OP_LNOT boolean_factor
    { $$ = expr_create(EXPR_LNOT, NULL, $2); }
|   LPAREN boolean_expr RPAREN
    { $$ = $2; }
|   arithmetic_expr OP_LT arithmetic_expr
    { $$ = expr_create(EXPR_LT $1, $3); }
|   arithmetic_expr OP_LE arithmetic_expr
    { $$ = expr_create(EXPR_LE, $1, $3); }
|   arithmetic_expr OP_GT arithmetic_expr
    { $$ = expr_create(EXPR_GT, $1, $3); }
|   arithmetic_expr OP_GE arithmetic_expr
    { $$ = expr_create(EXPR_GE, $1, $3); }
|   arithmetic_expr OP_EQ arithmetic_expr
    { $$ = expr_create(EXPR_EQ, $1, $3); }
|   arithmetic_expr OP_NE arithmetic_expr
    { $$ = expr_create(EXPR_NE, $1, $3); }
;

symbol
:   IDENTIFIER
    /* need to fix the creation (scope, type, name) */
    { $$ = symbol_create(SYMBOL_GLOBAL, NULL, lexer_val.identifier_symbol); }
;

%%

void yyerror(char const *str) {
    fprintf(stderr, "PARSE ERROR: %s\n", str);
}
