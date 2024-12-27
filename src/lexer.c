#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <lexer.h>

Lexer *lexer_create(const char *source) {
    Lexer *lexer = malloc(sizeof(Lexer));
    if (!lexer) return NULL;
    
    lexer->source = strdup(source);
    if (!lexer->source) {
        free(lexer);
        return NULL;
    }
    
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->current_char = lexer->source[0];
    
    return lexer;
}

void lexer_free(Lexer *lexer) {
    if (lexer) {
        free(lexer->source);
        free(lexer);
    }
}

void lexer_advance(Lexer *lexer) {
    if (lexer->current_char == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }
    
    lexer->position++;
    lexer->current_char = lexer->source[lexer->position];
}

char lexer_peek(Lexer *lexer) {
    return lexer->source[lexer->position + 1];
}

void lexer_skip_whitespace(Lexer *lexer) {
    while (lexer->current_char && isspace(lexer->current_char)) {
        lexer_advance(lexer);
    }
}

void lexer_skip_comment(Lexer *lexer) {
    // Skip single-line comment
    if (lexer->current_char == '/' && lexer_peek(lexer) == '/') {
        while (lexer->current_char && lexer->current_char != '\n') {
            lexer_advance(lexer);
        }
    }
    // Skip multi-line comment
    else if (lexer->current_char == '/' && lexer_peek(lexer) == '*') {
        lexer_advance(lexer); // Skip /
        lexer_advance(lexer); // Skip *
        while (lexer->current_char) {
            if (lexer->current_char == '*' && lexer_peek(lexer) == '/') {
                lexer_advance(lexer); // Skip *
                lexer_advance(lexer); // Skip /
                break;
            }
            lexer_advance(lexer);
        }
    }
}

Token *lexer_make_number(Lexer *lexer) {
    char buffer[256] = {0};
    int i = 0;
    int line = lexer->line;
    int column = lexer->column;
    
    while (lexer->current_char && isdigit(lexer->current_char)) {
        if (i < 255) {
            buffer[i++] = lexer->current_char;
        }
        lexer_advance(lexer);
    }
    
    return token_create(TOKEN_NUMBER, buffer, line, column);
}

Token *lexer_make_identifier(Lexer *lexer) {
    char buffer[256] = {0};
    int i = 0;
    int line = lexer->line;
    int column = lexer->column;
    
    while (lexer->current_char && (isalnum(lexer->current_char) || lexer->current_char == '_')) {
        if (i < 255) {
            buffer[i++] = lexer->current_char;
        }
        lexer_advance(lexer);
    }

    // Check for keywords
    if (strcmp(buffer, "int") == 0) return token_create(TOKEN_INT, buffer, line, column);
    if (strcmp(buffer, "return") == 0) return token_create(TOKEN_RETURN, buffer, line, column);
    if (strcmp(buffer, "if") == 0) return token_create(TOKEN_IF, buffer, line, column);
    if (strcmp(buffer, "else") == 0) return token_create(TOKEN_ELSE, buffer, line, column);
    if (strcmp(buffer, "while") == 0) return token_create(TOKEN_WHILE, buffer, line, column);
    if (strcmp(buffer, "for") == 0) return token_create(TOKEN_FOR, buffer, line, column);
    if (strcmp(buffer, "void") == 0) return token_create(TOKEN_VOID, buffer, line, column);
    
    return token_create(TOKEN_IDENTIFIER, buffer, line, column);
}

Token *lexer_next_token(Lexer *lexer) {
    while (lexer->current_char) {
        // Skip whitespace and comments
        if (isspace(lexer->current_char)) {
            lexer_skip_whitespace(lexer);
            continue;
        }
        if (lexer->current_char == '/' && (lexer_peek(lexer) == '/' || lexer_peek(lexer) == '*')) {
            lexer_skip_comment(lexer);
            continue;
        }

        // Start token recognition
        if (isdigit(lexer->current_char)) {
            return lexer_make_number(lexer);
        }

        if (isalpha(lexer->current_char) || lexer->current_char == '_') {
            return lexer_make_identifier(lexer);
        }

        // Handle operators and punctuation
        int line = lexer->line;
        int column = lexer->column;
        char current = lexer->current_char;
        lexer_advance(lexer);

        switch (current) {
            case '+': return token_create(TOKEN_PLUS, "+", line, column);
            case '-': return token_create(TOKEN_MINUS, "-", line, column);
            case '*': return token_create(TOKEN_MULTIPLY, "*", line, column);
            case '/': return token_create(TOKEN_DIVIDE, "/", line, column);
            case '%': return token_create(TOKEN_MODULO, "%", line, column);
            case '(': return token_create(TOKEN_LPAREN, "(", line, column);
            case ')': return token_create(TOKEN_RPAREN, ")", line, column);
            case '{': return token_create(TOKEN_LBRACE, "{", line, column);
            case '}': return token_create(TOKEN_RBRACE, "}", line, column);
            case '[': return token_create(TOKEN_LBRACKET, "[", line, column);
            case ']': return token_create(TOKEN_RBRACKET, "]", line, column);
            case ';': return token_create(TOKEN_SEMICOLON, ";", line, column);
            case ',': return token_create(TOKEN_COMMA, ",", line, column);
            case '.': return token_create(TOKEN_DOT, ".", line, column);
            
            // Two-character operators
            case '=':
                if (lexer->current_char == '=') {
                    lexer_advance(lexer);
                    return token_create(TOKEN_EQ, "==", line, column);
                }
                return token_create(TOKEN_ASSIGN, "=", line, column);
            
            case '!':
                if (lexer->current_char == '=') {
                    lexer_advance(lexer);
                    return token_create(TOKEN_NEQ, "!=", line, column);
                }
                return token_create(TOKEN_NOT, "!", line, column);
            
            case '<':
                if (lexer->current_char == '=') {
                    lexer_advance(lexer);
                    return token_create(TOKEN_LEQ, "<=", line, column);
                }
                return token_create(TOKEN_LT, "<", line, column);
            
            case '>':
                if (lexer->current_char == '=') {
                    lexer_advance(lexer);
                    return token_create(TOKEN_GEQ, ">=", line, column);
                }
                return token_create(TOKEN_GT, ">", line, column);
            
            case '&':
                if (lexer->current_char == '&') {
                    lexer_advance(lexer);
                    return token_create(TOKEN_AND, "&&", line, column);
                }
                break;
            
            case '|':
                if (lexer->current_char == '|') {
                    lexer_advance(lexer);
                    return token_create(TOKEN_OR, "||", line, column);
                }
                break;
        }

        // Handle invalid characters
        char error_msg[2] = {current, '\0'};
        return token_create(TOKEN_ERROR, error_msg, line, column);
    }

    // End of file
    return token_create(TOKEN_EOF, NULL, lexer->line, lexer->column);
}