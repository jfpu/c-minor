#include "utility.h"

const char *token_to_string(enum yytokentype token) {
    switch (token) {
        case ARRAY:
            return "ARRAY";
        case BOOLEAN:
            return "BOOLEAN";
        case CHAR:
            return "CHAR";
        case ELSE:
            return "ELSE";
        case FALSE:
            return "FALSE";
        case FOR:
            return "FOR";
        case FUNCTION:
            return "FUNCTION";
        case IF:
            return "IF";
        case INTEGER:
            return "INTEGER";
        case PRINT:
            return "PRINT";
        case RETURN:
            return "RETURN";
        case STRING:
            return "STRING";
        case TRUE:
            return "TRUE";
        case VOID:
            return "VOID";
        case WHILE:
            return "WHILE";
        case INTEGER_LITERAL:
            return "INTEGER_LITERAL";
        case CHAR_LITERAL:
            return "CHAR_LITERAL";
        case STRING_LITERAL:
            return "STRING_LITERAL";
        case IDENTIFIER:
            return "IDENTIFIER";
        case LPAREN:
            return "LPAREN";
        case RPAREN:
            return "RPAREN";
        case LBRACKET:
            return "LBRACKET";
        case RBRACKET:
            return "RBRACKET";
        case OP_INC:
            return "INC";
        case OP_DEC:
            return "DEC";
        case OP_MINUS:
            return "MINUS";
        case OP_EXP:
            return "EXP";
        case OP_MULT:
            return "MULT";
        case OP_DIV:
            return "DIV";
        case OP_MOD:
            return "MOD";
        case OP_PLUS:
            return "PLUS";
        case OP_LT:
            return "LT";
        case OP_LE:
            return "LE";
        case OP_GT:
            return "GT";
        case OP_GE:
            return "GE";
        case OP_EQ:
            return "EQ";
        case OP_NE:
            return "NE";
        case OP_LAND:
            return "LAND";
        case OP_LOR:
            return "LOR";
        case OP_LNOT:
            return "LNOT";
        case OP_ASSIGN:
            return "ASSIGN";
        case LCBRACK:
            return "LEFTCURLYBRACKET";
        case RCBRACK:
            return "RIGHTCURLYBRACKET";
        case SEMICOLON:
            return "SEMICOLON";
        case COLON:
            return "COLON";
        case COMMA:
            return "COMMA";
        default:
            // This should never happen
            return "ERROR_INVALID_CHARACTER";
    }
}

void _print_indent(int indent) {
    int i;
    for (i = 0; i < indent; ++i) { printf("\t"); }
}
