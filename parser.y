/* Parser for c-minor */

%{
    extern char *yytext;
    extern int yylex();
    extern int yyerror(char const *);
%}

%token SAMPLE

%%

prog: SAMPLE
    ;
