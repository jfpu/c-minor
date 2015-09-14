/* Manifest definition */
enum {
    ARRAY = 258,    BOOLEAN = 259,  CHAR = 260,         ELSE = 261,
    FALSE = 262,    FOR = 263,      FUNCTION = 264,     IF = 265,
    INTEGER = 266,  PRINT = 267,    RETURN = 268,       STRING = 269,
    TRUE = 270,     VOID = 271,     WHILE = 272,
    INTEGER_LITERAL = 273,          BOOLEAN_LITERAL = 274,
    CHAR_LITERAL = 275,             STRING_LITERAL = 276,
    IDENTIFIER = 277,
    LPAREN = 278,   RPAREN = 279,   LBRACKET = 280,     RBRACKET = 281,
    OP_INC = 282,   OP_DEC = 283,   OP_MINUS = 284,    OP_EXP = 285,
    OP_MULT = 286,  OP_DIV = 287,   OP_MOD = 288,       OP_PLUS = 289,
    OP_LT = 290,    OP_LE = 291,    OP_GT = 292,        OP_GE = 293,
    OP_EQ = 294,    OP_NE = 295,    OP_LAND = 296,      OP_LOR = 297,
    OP_ASSIGN = 298,
    LCBRACK = 299,  RCBRACK = 300,  SEMICOLON = 301,    COLON = 302,
    COMMA = 303,
};

const char *token_manifest_to_string(int token) {
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
        case BOOLEAN_LITERAL:
            return "BOOLEAN_LITERAL";
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
