#include <stdlib.h>
#include <string.h>
#include <token.h>

Token *token_create(TokenType type, const char *value, int line, int column) {
    Token *token = malloc(sizeof(Token));
    if (!token) return NULL;
    
    token->type = type;
    token->line = line;
    token->column = column;
    
    if (value) {
        token->value = strdup(value);
        if (!token->value) {
            free(token);
            return NULL;
        }
    } else {
        token->value = NULL;
    }
    
    return token;
}

void token_free(Token *token) {
    if (token) {
        free(token->value);
        free(token);
    }
}

const char *token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_INT: return "INT";
        case TOKEN_RETURN: return "RETURN";
        case TOKEN_IF: return "IF";
        case TOKEN_ELSE: return "ELSE";
        case TOKEN_WHILE: return "WHILE";
        case TOKEN_FOR: return "FOR";
        case TOKEN_VOID: return "VOID";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_STRING: return "STRING";
        case TOKEN_CHAR: return "CHAR";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_MULTIPLY: return "MULTIPLY";
        case TOKEN_DIVIDE: return "DIVIDE";
        case TOKEN_MODULO: return "MODULO";
        case TOKEN_ASSIGN: return "ASSIGN";
        case TOKEN_EQ: return "EQUALS";
        case TOKEN_NEQ: return "NOT_EQUALS";
        case TOKEN_LT: return "LESS_THAN";
        case TOKEN_GT: return "GREATER_THAN";
        case TOKEN_LEQ: return "LESS_EQUAL";
        case TOKEN_GEQ: return "GREATER_EQUAL";
        case TOKEN_AND: return "AND";
        case TOKEN_OR: return "OR";
        case TOKEN_NOT: return "NOT";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_LBRACE: return "LBRACE";
        case TOKEN_RBRACE: return "RBRACE";
        case TOKEN_LBRACKET: return "LBRACKET";
        case TOKEN_RBRACKET: return "RBRACKET";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_DOT: return "DOT";
        case TOKEN_EOF: return "EOF";
        case TOKEN_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}