/* Parser for c-minor */

%defines
%debug

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
%token TOKEN_EOF

%{
#include <stdio.h>
#include <string.h> // strdup
#include "utility.h"

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
    char *name;
}

%type <decl> prog decl_list decl
%type <stmt> stmt_list stmt stmt_matched
%type <expr> expr_list expr_list_opt expr expr_opt arithmetic_expr arithmetic_expr_no_assign arithmetic_term arithmetic_factor_with_exponentiation arithmetic_factor_with_negation arithmetic_factor func_call boolean_expr boolean_factor
%type <formal> formal_list nonempty_formal_list formal
%type <type> type non_array_type array_type ret_type func_type
%type <name> identifier

%%

prog
:   TOKEN_EOF
    { program = NULL; return 0; }
|   decl_list TOKEN_EOF
    { program = $1; return 0; }
;

decl_list
:   decl decl_list
    { $$ = decl_list_prepend($1, $2); }
|   decl
    { $$ = $1; }
;

decl
:   identifier COLON type SEMICOLON
    { $$ = decl_create($1, $3, NULL, NULL, NULL); }
|   identifier COLON non_array_type OP_ASSIGN expr SEMICOLON
    { $$ = decl_create($1, $3, $5, NULL, NULL); }
|   identifier COLON array_type OP_ASSIGN LCBRACK expr_list RCBRACK SEMICOLON
    { $$ = decl_create($1, $3, $6, NULL, NULL); }
|   identifier COLON func_type OP_ASSIGN LCBRACK stmt_list RCBRACK
    { $$ = decl_create($1, $3, NULL, $6, NULL); }
;

stmt_list
:   stmt stmt_list
    { $$ = stmt_list_prepend($1, $2); }
|   stmt
    { $$ = $1; }
;

stmt
:   LCBRACK stmt_list RCBRACK
    { $$ = $2; }
|   decl
    { $$ = stmt_create(STMT_DECL, $1, NULL, NULL, NULL, NULL, NULL); }
|   RETURN expr_opt SEMICOLON
    { $$ = stmt_create(STMT_RETURN, NULL, NULL, $2, NULL, NULL, NULL); }
|   PRINT expr_list_opt SEMICOLON
    { $$ = stmt_create(STMT_PRINT, NULL, NULL, $2, NULL, NULL, NULL); }
|   expr SEMICOLON
    { $$ = stmt_create(STMT_EXPR, NULL, NULL, $1, NULL, NULL, NULL); }
|   IF LPAREN expr RPAREN stmt
    { $$ = stmt_create(STMT_IF_ELSE, NULL, NULL, $3, NULL, $5, NULL); }
|   IF LPAREN expr RPAREN stmt_matched ELSE stmt
    { $$ = stmt_create(STMT_IF_ELSE, NULL, NULL, $3, NULL, $5, $7); }
|   FOR LPAREN expr_opt SEMICOLON expr_opt SEMICOLON expr_opt RPAREN stmt
    { $$ = stmt_create(STMT_FOR, NULL, $3, $5, $7, $9, NULL); }
;

stmt_matched
:   IF LPAREN expr RPAREN stmt_matched ELSE stmt_matched
    { $$ = stmt_create(STMT_IF_ELSE, NULL, NULL, $3, NULL, $5, $7); }
|   FOR LPAREN expr_opt SEMICOLON expr_opt SEMICOLON expr_opt RPAREN stmt_matched
    { $$ = stmt_create(STMT_FOR, NULL, $3, $5, $7, $9, NULL); }
|   LCBRACK stmt_list RCBRACK
    { $$ = $2; }
|   decl
    { $$ = stmt_create(STMT_DECL, $1, NULL, NULL, NULL, NULL, NULL); }
|   RETURN expr SEMICOLON
    { $$ = stmt_create(STMT_RETURN, NULL, NULL, $2, NULL, NULL, NULL); }
|   PRINT expr_list_opt SEMICOLON
    { $$ = stmt_create(STMT_PRINT, NULL, NULL, $2, NULL, NULL, NULL); }
|   expr SEMICOLON
    { $$ = stmt_create(STMT_EXPR, NULL, NULL, $1, NULL, NULL, NULL); }
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
:   identifier COLON type
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
:   ARRAY LBRACKET expr_opt RBRACKET type
    { $$ = type_create_array($3, $5); }
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

expr_list_opt
:   expr_list
    { $$ = $1; }
|
    { $$ = NULL; }
;

expr
:   CHAR_LITERAL
    { $$ = expr_create_character_literal(lexer_val.char_value); }
|   STRING_LITERAL
    {   $$ = expr_create_string_literal(strdup(_global_string_buffer)); }
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
:   arithmetic_factor OP_ASSIGN arithmetic_expr
    { $$ = expr_create(EXPR_ASSIGN, $1, $3); }
|   arithmetic_expr_no_assign
    { $$ = $1; }
;

arithmetic_expr_no_assign
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
:   arithmetic_factor_with_negation OP_EXP arithmetic_factor_with_exponentiation
    { $$ = expr_create(EXPR_EXP, $1, $3); }
|   arithmetic_factor_with_negation
    { $$ = $1; }
;

arithmetic_factor_with_negation
:   OP_MINUS arithmetic_factor
    { $$ = expr_create(EXPR_SUB, NULL, $2); }
|   arithmetic_factor
    { $$ = $1; }
;

arithmetic_factor
:   INTEGER_LITERAL
    { $$ = expr_create_integer_literal(lexer_val.int_value); }
|   identifier
    { $$ = expr_create_name($1); }
|   identifier LBRACKET expr RBRACKET
    { $$ = expr_create_array_deref($1, $3); }
|   func_call
    { $$ = $1; }
|   LPAREN arithmetic_expr RPAREN
    { $$ = $2; }
|   identifier OP_INC
    { $$ = expr_create_incdec(EXPR_INC, $1); }
|   identifier OP_DEC
    { $$ = expr_create_incdec(EXPR_DEC, $1); }
;

func_call
    /* need to fix these: what's the right way to invoke a function? */
:   identifier LPAREN expr_list RPAREN
    { $$ = expr_create_function_call($1, $3); }
|   identifier LPAREN RPAREN
    { $$ = expr_create_function_call($1, NULL); }
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
    { $$ = expr_create(EXPR_LT, $1, $3); }
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

identifier
:   IDENTIFIER
    /* We're not creating a symbol here; instead we're returning the string value as name */
    { $$ = strdup(lexer_val.identifier_symbol); }
;

%%

void yyerror(char const *str) {
    fprintf(stderr, "PARSE ERROR: %s\n", str);
}
