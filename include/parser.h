#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <lexer.h>
#include <ast.h>

typedef struct {
    Lexer *lexer;
    Token *current_token;
    Token *peek_token;
} Parser;

// Parser management functions
Parser *parser_create(Lexer *lexer);
void parser_free(Parser *parser);

// Core parsing functions
void parser_advance(Parser *parser);
bool parser_expect(Parser *parser, TokenType type);
void parser_error(Parser *parser, const char *message);

// Production rules
ASTNode *parser_parse_program(Parser *parser);
ASTNode *parser_parse_function(Parser *parser);
ASTNode *parser_parse_block(Parser *parser);
ASTNode *parser_parse_statement(Parser *parser);
ASTNode *parser_parse_expression(Parser *parser);
ASTNode *parser_parse_arithmetic(Parser *parser);
ASTNode *parser_parse_term(Parser *parser);
ASTNode *parser_parse_factor(Parser *parser);

// Statement parsing functions
ASTNode *parser_parse_return_statement(Parser *parser);
ASTNode *parser_parse_if_statement(Parser *parser);
ASTNode *parser_parse_while_statement(Parser *parser);
ASTNode *parser_parse_variable_declaration(Parser *parser);
ASTNode *parser_parse_assignment(Parser *parser);

#endif // PARSER_H