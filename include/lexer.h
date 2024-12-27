#ifndef LEXER_H
#define LEXER_H

#include <token.h>

typedef struct {
    char *source;        // Source code
    int position;        // Current position in source
    int line;           // Current line number
    int column;         // Current column number
    char current_char;  // Current character
} Lexer;

// Lexer management functions
Lexer *lexer_create(const char *source);
void lexer_free(Lexer *lexer);

// Lexical analysis functions
Token *lexer_next_token(Lexer *lexer);
void lexer_advance(Lexer *lexer);
char lexer_peek(Lexer *lexer);
void lexer_skip_whitespace(Lexer *lexer);
void lexer_skip_comment(Lexer *lexer);

// Token recognition functions
Token *lexer_make_number(Lexer *lexer);
Token *lexer_make_identifier(Lexer *lexer);
Token *lexer_make_string(Lexer *lexer);
Token *lexer_make_char(Lexer *lexer);

#endif // LEXER_H