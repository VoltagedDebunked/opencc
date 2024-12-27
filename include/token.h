#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    // Keywords
    TOKEN_INT,
    TOKEN_RETURN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_VOID,
    
    // Identifiers and literals
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_CHAR,
    
    // Operators
    TOKEN_PLUS,      // +
    TOKEN_MINUS,     // -
    TOKEN_MULTIPLY,  // *
    TOKEN_DIVIDE,    // /
    TOKEN_MODULO,    // %
    TOKEN_ASSIGN,    // =
    TOKEN_EQ,        // ==
    TOKEN_NEQ,       // !=
    TOKEN_LT,        // <
    TOKEN_GT,        // >
    TOKEN_LEQ,       // <=
    TOKEN_GEQ,       // >=
    TOKEN_AND,       // &&
    TOKEN_OR,        // ||
    TOKEN_NOT,       // !
    
    // Punctuation
    TOKEN_LPAREN,    // (
    TOKEN_RPAREN,    // )
    TOKEN_LBRACE,    // {
    TOKEN_RBRACE,    // }
    TOKEN_LBRACKET,  // [
    TOKEN_RBRACKET,  // ]
    TOKEN_SEMICOLON, // ;
    TOKEN_COMMA,     // ,
    TOKEN_DOT,       // .
    
    // Special tokens
    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    char *value;     // Lexeme or string value
    int line;        // Line number in source
    int column;      // Column number in source
} Token;

// Token management functions
Token *token_create(TokenType type, const char *value, int line, int column);
void token_free(Token *token);
const char *token_type_to_string(TokenType type);

#endif // TOKEN_H