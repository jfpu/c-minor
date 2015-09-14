/* Scanner for C-Minor */

%{
#include <stdlib.h>       // strtoll
#include <string.h>       // strlen
#include "token_manifest.h"

// Fake YYSTYPE definition
typedef union _yystype_fake {
    long long int int_value;
    char char_value;
    char *identifier_symbol;
    unsigned int string_buffer_index;
} YYSTYPE;

// Shared character buffer for identifier and strings
#define MAX_STRING_LENGTH 256
#define MAX_STRING_LENGTH_BUFFERED 257
char _global_string_buffer[MAX_STRING_LENGTH_BUFFERED];

// String appending routine
#define BUFFER_APPEND(c) _global_string_buffer[(yylval->string_buffer_index++)] = (c)

// Make sure matched identifier are not too long
#define MAX_IDENTIFIER_LENGTH 256

// Error handling routine
#define ERROR(f_, ...) {                  \
  fprintf(stderr, "SCAN ERROR: ");        \
  fprintf(stderr, (f_), __VA_ARGS__);     \
  fprintf(stderr, "\n");                  \
  exit(1);                                \
}
#define ERROR_NOARGS(f_) {                \
  fprintf(stderr, "SCAN ERROR: ");        \
  fprintf(stderr, (f_));                  \
  fprintf(stderr, "\n");                  \
  exit(1);                                \
}

%}

%option noyywrap
%option yylineno

/* Fake bison to access yylval */
%option bison-bridge

DIGIT       [0-9]
LETTER      [a-zA-Z]
ID          ({LETTER}|_)({LETTER}|{DIGIT}|_)*
WHITESPACE  [\t\ \n\r]

%x STR
%x CMT
%x LINECMT

%%

array       { return (ARRAY); }
boolean     { return (BOOLEAN); }
char        { return (CHAR); }
else        { return (ELSE); }
false       { return (FALSE); }
for         { return (FOR); }
function    { return (FUNCTION); }
if          { return (IF); }
integer     { return (INTEGER); }
print       { return (PRINT); }
return      { return (RETURN); }
string      { return (STRING); }
true        { return (TRUE); }
void        { return (VOID); }
while       { return (WHILE); }

\(          { return (LPAREN); }
\)          { return (RPAREN); }
\[          { return (LBRACKET); }
\]          { return (RBRACKET); }
\+\+        { return (OP_INC); }
--          { return (OP_DEC); }
\^          { return (OP_EXP); }
\*          { return (OP_MULT); }
\/          { return (OP_DIV); }
%           { return (OP_MOD); }
\+          { return (OP_PLUS); }
-           { return (OP_MINUS); }

\<          { return (OP_LT); }
\<=         { return (OP_LE); }
\>          { return (OP_GT); }
\>=         { return (OP_GE); }
==          { return (OP_EQ); }
!=          { return (OP_NE); }
&&          { return (OP_LAND); }
\|\|        { return (OP_LOR); }
=           { return (OP_ASSIGN); }

\{          { return (LCBRACK); }
\}          { return (RCBRACK); }
\;          { return (SEMICOLON); }
\:          { return (COLON); }
,           { return (COMMA); }

{WHITESPACE}+ ;   /* Eat whitespace */

{ID}        {
  if (strlen(yytext) > MAX_IDENTIFIER_LENGTH) {
    // Identifier is too long, throw error
    ERROR("Identifier is overflowing max length %d\n", MAX_IDENTIFIER_LENGTH);
  }
  yylval->identifier_symbol = yytext;
  return (IDENTIFIER);
}

{DIGIT}+    {
  yylval->int_value = strtoll(yytext, NULL, 10);
  return (INTEGER_LITERAL);
}
'.'         { yylval->char_value = yytext[1]; return (CHAR_LITERAL); }
'\\.'       {
  // Handle escape sequences
  switch (yytext[2]) {
    case 'n': {   // '\n'
      yylval->char_value = 10;
      break;
    }
    case '0': {   // '\0'
      yylval->char_value = 0;
      break;
    }
    default: {    // anything else is the same character
      yylval->char_value = yytext[2];
      break;
    }
  }
  return (CHAR_LITERAL);
}

<INITIAL>\" {     /* Beginning of string */
  yylval->string_buffer_index = 0;
  BEGIN(STR);
}
<STR>\"     {
  BEGIN(INITIAL);
  BUFFER_APPEND('\0');
  return (STRING_LITERAL);
}
<STR>\\.    {
  // Handle escape sequences
  if (yylval->string_buffer_index + 1 > MAX_STRING_LENGTH) {
    // String is overflowing
    ERROR("String is overflowing max length %d", MAX_STRING_LENGTH);
  }
  switch (yytext[1]) {
    case 'n': {   // '\n'
      BUFFER_APPEND(10);
      break;
    }
    case '0': {   // '\0'
      BUFFER_APPEND(0);
      break;
    }
    default: {    // anything else is the same character
      BUFFER_APPEND(yytext[1]);
      break;
    }
  }
}
<STR>\n     {
  // Strings with line breaks are invalid
  ERROR_NOARGS("String contains line breaks");
}
<STR>.      {
  if (yylval->string_buffer_index + 1 > MAX_STRING_LENGTH) {
    // String is overflowing, throw error
    ERROR("String length is longer than max length %d\n", MAX_STRING_LENGTH);
  }
  BUFFER_APPEND(yytext[0]);
}

<INITIAL>\/\/ {
  BEGIN(LINECMT);
}
<LINECMT>\n   {
  BEGIN(INITIAL);
}
<LINECMT>.  {}

<INITIAL>\/\* {
  BEGIN(CMT);
}
<CMT>\*\/   {
  BEGIN(INITIAL);
}
<CMT>\n     ;
<CMT>.      ;

.           {
  /* Error case */
  ERROR("Unknown character %s", yytext);
}

<<EOF>>     { return 0; }

