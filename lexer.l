/* Scanner for C-Minor */

%{
#include <stdlib.h>         // strtoll
#include <string.h>         // strlen
#include <ctype.h>          // isascii
#include "utility.h"        // tokens
%}

%option noyywrap
%option yylineno
%option header-file="lex.yy.h"
%option bison-bridge

DIGIT       [0-9]
LETTER      [a-zA-Z]
ID          ({LETTER}|_)({LETTER}|{DIGIT}|_)*
WHITESPACE  [\t\ \n\r]

%x STR
%x CMT
%x LINECMT

%%

array               { return (ARRAY); }
boolean             { return (BOOLEAN); }
char                { return (CHAR); }
else                { return (ELSE); }
false               { return (FALSE); }
for                 { return (FOR); }
function            { return (FUNCTION); }
if                  { return (IF); }
integer             { return (INTEGER); }
print               { return (PRINT); }
return              { return (RETURN); }
string              { return (STRING); }
true                { return (TRUE); }
void                { return (VOID); }
while               { return (WHILE); }

\(                  { return (LPAREN); }
\)                  { return (RPAREN); }
\[                  { return (LBRACKET); }
\]                  { return (RBRACKET); }
\+\+                { return (OP_INC); }
--                  { return (OP_DEC); }
\^                  { return (OP_EXP); }
\*                  { return (OP_MULT); }
\/                  { return (OP_DIV); }
%                   { return (OP_MOD); }
\+                  { return (OP_PLUS); }
-                   { return (OP_MINUS); }

\<                  { return (OP_LT); }
\<=                 { return (OP_LE); }
\>                  { return (OP_GT); }
\>=                 { return (OP_GE); }
==                  { return (OP_EQ); }
!=                  { return (OP_NE); }
&&                  { return (OP_LAND); }
\|\|                { return (OP_LOR); }
!                   { return (OP_LNOT); }
=                   { return (OP_ASSIGN); }

\{                  { return (LCBRACK); }
\}                  { return (RCBRACK); }
\;                  { return (SEMICOLON); }
\:                  { return (COLON); }
,                   { return (COMMA); }

{WHITESPACE}+       {}   /* Eat whitespace */

{ID}                {
    if (strlen(yytext) > MAX_IDENTIFIER_LENGTH) {
        // Identifier is too long, throw error
        ERROR("Identifier is overflowing max length %d", MAX_IDENTIFIER_LENGTH);
    }
    lexer_val.identifier_symbol = yytext;
    return (IDENTIFIER);
}

{DIGIT}+            {
    lexer_val.int_value = strtoll(yytext, NULL, 10);
    return (INTEGER_LITERAL);
}

'.'                 {
    if (!isascii(yytext[1])) {
        // Illegal character
        ERROR_NOARGS("Illegal character literal");
    } else if (yytext[1] == '\\') {
        // Matched '\' which should be an error
        ERROR_NOARGS("Illegal character literal");
    }
    lexer_val.char_value = yytext[1];
    return (CHAR_LITERAL);
}
'\\.'               {
    // Handle escape sequences
    switch (yytext[2]) {
        case 'n': {   // '\n'
            lexer_val.char_value = 10;
            break;
        }
        case '0': {   // '\0'
            lexer_val.char_value = 0;
            break;
        }
        default: {    // anything else is the same character
            if (!isascii(yytext[2])) {
                // Illegal character
                ERROR_NOARGS("Illegal character literal");
            }
            lexer_val.char_value = yytext[2];
            break;
        }
    }
    return (CHAR_LITERAL);
}

<INITIAL>\"         {
    // Beginning of string
    lexer_val.string_buffer_index = 0;
    BEGIN(STR);
}
<STR>\"             {
    // End of string
    BEGIN(INITIAL);
    BUFFER_APPEND('\0');
    return (STRING_LITERAL);
}
<STR>\\.            {
    // Handle escape sequences
    if (lexer_val.string_buffer_index + 1 >= MAX_STRING_LENGTH) {
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
            if (!isascii(yytext[1])) {
                // Illegal character
                ERROR_NOARGS("Illegal character in string");
            }
            BUFFER_APPEND(yytext[1]);
            break;
        }
    }
}
<STR>\n             {
    // Strings with line breaks are invalid
    ERROR_NOARGS("String contains line breaks");
}
<STR>.              {
    if (lexer_val.string_buffer_index + 1 >= MAX_STRING_LENGTH) {
        // String is overflowing, throw error
        ERROR("String is overflowing max length %d", MAX_STRING_LENGTH);
    } else if (!isascii(yytext[0])) {
        // Illegal character
        ERROR_NOARGS("Illegal character in string");
    }
    BUFFER_APPEND(yytext[0]);
}

<INITIAL>\/\/       {
    BEGIN(LINECMT);
}
<LINECMT>\n         {
    BEGIN(INITIAL);
}
<LINECMT>.          {}

<INITIAL>\/\* {
    BEGIN(CMT);
}
<CMT>\*\/           {
    BEGIN(INITIAL);
}
<CMT>\n             {}
<CMT>.              {}

.                   {
    // Error case
    ERROR("Unknown character %s", yytext);
}

<INITIAL><<EOF>>    { return 0; }
<LINECMT><<EOF>>    { return 0; }
<STR><<EOF>>        { ERROR_NOARGS("Unexpected EOF: unmatched \""); }
<CMT><<EOF>>        { ERROR_NOARGS("Unexpected EOF: unmatched /*"); }
