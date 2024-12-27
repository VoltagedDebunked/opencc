#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <parser.h>

Parser *parser_create(Lexer *lexer) {
    Parser *parser = malloc(sizeof(Parser));
    if (!parser) return NULL;

    parser->lexer = lexer;
    parser->current_token = lexer_next_token(lexer);
    parser->peek_token = lexer_next_token(lexer);
    return parser;
}

void parser_free(Parser *parser) {
    if (parser) {
        token_free(parser->current_token);
        token_free(parser->peek_token);
        free(parser);
    }
}

void parser_advance(Parser *parser) {
    token_free(parser->current_token);
    parser->current_token = parser->peek_token;
    parser->peek_token = lexer_next_token(parser->lexer);
}

bool parser_expect(Parser *parser, TokenType type) {
    if (parser->current_token->type == type) {
        parser_advance(parser);
        return true;
    }
    return false;
}

void parser_error(Parser *parser, const char *message) {
    fprintf(stderr, "Error at line %d, column %d: %s\n", 
            parser->current_token->line,
            parser->current_token->column,
            message);
    exit(1);
}

// Grammar rules implementation
ASTNode *parser_parse_program(Parser *parser) {
    ASTNode *program = ast_create_program();
    if (!program) return NULL;

    // Add functions to program
    while (parser->current_token->type != TOKEN_EOF) {
        ASTNode *function = parser_parse_function(parser);
        if (!function) {
            ast_free(program);
            return NULL;
        }

        // Reallocate function array
        void *temp = realloc(program->data.program.functions,
                           sizeof(ASTNode*) * (program->data.program.function_count + 1));
        if (!temp) {
            ast_free(function);
            ast_free(program);
            return NULL;
        }

        program->data.program.functions = temp;
        program->data.program.functions[program->data.program.function_count++] = function;
    }

    return program;
}

ASTNode *parser_parse_function(Parser *parser) {
    // Parse return type (for now, only 'int' or 'void')
    if (!parser_expect(parser, TOKEN_INT) && !parser_expect(parser, TOKEN_VOID)) {
        parser_error(parser, "Expected function return type");
        return NULL;
    }

    // Parse function name
    if (parser->current_token->type != TOKEN_IDENTIFIER) {
        parser_error(parser, "Expected function name");
        return NULL;
    }
    char *name = strdup(parser->current_token->value);
    parser_advance(parser);

    // Parse parameters
    if (!parser_expect(parser, TOKEN_LPAREN)) {
        free(name);
        parser_error(parser, "Expected '(' after function name");
        return NULL;
    }

    char **params = NULL;
    int param_count = 0;

    // Parse parameter list
    while (parser->current_token->type != TOKEN_RPAREN) {
        if (param_count > 0) {
            if (!parser_expect(parser, TOKEN_COMMA)) {
                free(name);
                // Free previously allocated parameters
                for (int i = 0; i < param_count; i++) {
                    free(params[i]);
                }
                free(params);
                parser_error(parser, "Expected ',' between parameters");
                return NULL;
            }
        }

        // Parse parameter type
        if (!parser_expect(parser, TOKEN_INT)) {
            free(name);
            for (int i = 0; i < param_count; i++) {
                free(params[i]);
            }
            free(params);
            parser_error(parser, "Expected parameter type");
            return NULL;
        }

        // Parse parameter name
        if (parser->current_token->type != TOKEN_IDENTIFIER) {
            free(name);
            for (int i = 0; i < param_count; i++) {
                free(params[i]);
            }
            free(params);
            parser_error(parser, "Expected parameter name");
            return NULL;
        }

        // Add parameter to list
        void *temp = realloc(params, sizeof(char*) * (param_count + 1));
        if (!temp) {
            free(name);
            for (int i = 0; i < param_count; i++) {
                free(params[i]);
            }
            free(params);
            return NULL;
        }
        params = temp;
        params[param_count] = strdup(parser->current_token->value);
        param_count++;
        parser_advance(parser);
    }

    parser_expect(parser, TOKEN_RPAREN);

    // Parse function body
    ASTNode *body = parser_parse_block(parser);
    if (!body) {
        free(name);
        for (int i = 0; i < param_count; i++) {
            free(params[i]);
        }
        free(params);
        return NULL;
    }

    return ast_create_function(name, params, param_count, body);
}

ASTNode *parser_parse_block(Parser *parser) {
    if (!parser_expect(parser, TOKEN_LBRACE)) {
        parser_error(parser, "Expected '{' at start of block");
        return NULL;
    }

    ASTNode *block = ast_create_block();
    if (!block) return NULL;

    while (parser->current_token->type != TOKEN_RBRACE) {
        ASTNode *statement = parser_parse_statement(parser);
        if (!statement) {
            ast_free(block);
            return NULL;
        }

        // Add statement to block
        void *temp = realloc(block->data.block.statements,
                          sizeof(ASTNode*) * (block->data.block.statement_count + 1));
        if (!temp) {
            ast_free(statement);
            ast_free(block);
            return NULL;
        }

        block->data.block.statements = temp;
        block->data.block.statements[block->data.block.statement_count++] = statement;
    }

    parser_expect(parser, TOKEN_RBRACE);
    return block;
}

ASTNode *parser_parse_statement(Parser *parser) {
    switch (parser->current_token->type) {
        case TOKEN_RETURN:
            return parser_parse_return_statement(parser);
        case TOKEN_IF:
            return parser_parse_if_statement(parser);
        case TOKEN_WHILE:
            return parser_parse_while_statement(parser);
        case TOKEN_INT:
            return parser_parse_variable_declaration(parser);
        case TOKEN_IDENTIFIER:
            return parser_parse_assignment(parser);
        default:
            parser_error(parser, "Expected statement");
            return NULL;
    }
}

ASTNode *parser_parse_expression(Parser *parser) {
    ASTNode *left = parser_parse_arithmetic(parser);
    if (!left) return NULL;

    // Handle comparison operators
    if (parser->current_token->type == TOKEN_GT ||
        parser->current_token->type == TOKEN_LT ||
        parser->current_token->type == TOKEN_GEQ ||
        parser->current_token->type == TOKEN_LEQ ||
        parser->current_token->type == TOKEN_EQ ||
        parser->current_token->type == TOKEN_NEQ) {

        TokenType op_type = parser->current_token->type;
        parser_advance(parser);

        ASTNode *right = parser_parse_arithmetic(parser);
        if (!right) {
            ast_free(left);
            return NULL;
        }

        char op;
        switch (op_type) {
            case TOKEN_GT:  op = '>'; break;
            case TOKEN_LT:  op = '<'; break;
            case TOKEN_GEQ: op = 'G'; break;
            case TOKEN_LEQ: op = 'L'; break;
            case TOKEN_EQ:  op = 'E'; break;
            case TOKEN_NEQ: op = 'N'; break;
            default: op = '?'; break;
        }

        ASTNode *new_node = ast_create_binary_op(op, left, right);
        if (!new_node) {
            ast_free(left);
            ast_free(right);
            return NULL;
        }
        return new_node;
    }

    return left;
}

ASTNode *parser_parse_arithmetic(Parser *parser) {
    ASTNode *left = parser_parse_term(parser);
    if (!left) return NULL;

    while (parser->current_token->type == TOKEN_PLUS || 
           parser->current_token->type == TOKEN_MINUS) {
        char op = parser->current_token->type == TOKEN_PLUS ? '+' : '-';
        parser_advance(parser);

        ASTNode *right = parser_parse_term(parser);
        if (!right) {
            ast_free(left);
            return NULL;
        }

        ASTNode *new_node = ast_create_binary_op(op, left, right);
        if (!new_node) {
            ast_free(left);
            ast_free(right);
            return NULL;
        }
        left = new_node;
    }

    return left;
}

ASTNode *parser_parse_term(Parser *parser) {
    ASTNode *left = parser_parse_factor(parser);
    if (!left) return NULL;

    while (parser->current_token->type == TOKEN_MULTIPLY || 
           parser->current_token->type == TOKEN_DIVIDE) {
        char op = parser->current_token->type == TOKEN_MULTIPLY ? '*' : '/';
        parser_advance(parser);

        ASTNode *right = parser_parse_factor(parser);
        if (!right) {
            ast_free(left);
            return NULL;
        }

        ASTNode *new_node = ast_create_binary_op(op, left, right);
        if (!new_node) {
            ast_free(left);
            ast_free(right);
            return NULL;
        }
        left = new_node;
    }

    return left;
}

ASTNode *parser_parse_factor(Parser *parser) {
    switch (parser->current_token->type) {
        case TOKEN_NUMBER: {
            int value = atoi(parser->current_token->value);
            parser_advance(parser);
            return ast_create_number(value);
        }
        case TOKEN_IDENTIFIER: {
            char *name = strdup(parser->current_token->value);
            parser_advance(parser);
            return ast_create_variable(name);
        }
        case TOKEN_LPAREN: {
            parser_advance(parser);
            ASTNode *expr = parser_parse_expression(parser);
            if (!expr) return NULL;
            
            if (!parser_expect(parser, TOKEN_RPAREN)) {
                ast_free(expr);
                parser_error(parser, "Expected ')'");
                return NULL;
            }
            return expr;
        }
        case TOKEN_MINUS: {
            parser_advance(parser);
            ASTNode *factor = parser_parse_factor(parser);
            if (!factor) return NULL;
            
            // Create a binary operation with 0 - factor
            ASTNode *zero = ast_create_number(0);
            if (!zero) {
                ast_free(factor);
                return NULL;
            }
            
            return ast_create_binary_op('-', zero, factor);
        }
        default:
            parser_error(parser, "Expected number, identifier, or '('");
            return NULL;
    }
}

ASTNode *parser_parse_return_statement(Parser *parser) {
    if (!parser_expect(parser, TOKEN_RETURN)) {
        parser_error(parser, "Expected 'return'");
        return NULL;
    }

    ASTNode *expr = parser_parse_expression(parser);
    if (!expr) return NULL;

    if (!parser_expect(parser, TOKEN_SEMICOLON)) {
        ast_free(expr);
        parser_error(parser, "Expected ';' after return statement");
        return NULL;
    }

    return ast_create_return(expr);
}

ASTNode *parser_parse_variable_declaration(Parser *parser) {
    // Skip 'int' keyword
    parser_advance(parser);

    // Get variable name
    if (parser->current_token->type != TOKEN_IDENTIFIER) {
        parser_error(parser, "Expected variable name");
        return NULL;
    }
    
    char *name = strdup(parser->current_token->value);
    parser_advance(parser);

    // Handle initialization if present
    ASTNode *init_expr = NULL;
    if (parser->current_token->type == TOKEN_ASSIGN) {
        parser_advance(parser);
        init_expr = parser_parse_expression(parser);
        if (!init_expr) {
            free(name);
            return NULL;
        }
    }

    if (!parser_expect(parser, TOKEN_SEMICOLON)) {
        free(name);
        if (init_expr) ast_free(init_expr);
        parser_error(parser, "Expected ';' after variable declaration");
        return NULL;
    }

    // Create variable declaration node
    ASTNode *var_node = ast_create_variable(name);
    free(name);
    
    if (!var_node) {
        if (init_expr) ast_free(init_expr);
        return NULL;
    }

    if (init_expr) {
        // Create assignment node if there was initialization
        return ast_create_binary_op('=', var_node, init_expr);
    }
    
    return var_node;
}

ASTNode *parser_parse_if_statement(Parser *parser) {
    if (!parser_expect(parser, TOKEN_IF)) {
        parser_error(parser, "Expected 'if'");
        return NULL;
    }

    if (!parser_expect(parser, TOKEN_LPAREN)) {
        parser_error(parser, "Expected '(' after 'if'");
        return NULL;
    }

    ASTNode *condition = parser_parse_expression(parser);
    if (!condition) return NULL;

    if (!parser_expect(parser, TOKEN_RPAREN)) {
        ast_free(condition);
        parser_error(parser, "Expected ')' after if condition");
        return NULL;
    }

    ASTNode *then_branch = parser_parse_block(parser);
    if (!then_branch) {
        ast_free(condition);
        return NULL;
    }

    ASTNode *else_branch = NULL;
    if (parser->current_token->type == TOKEN_ELSE) {
        parser_advance(parser);
        else_branch = parser_parse_block(parser);
        if (!else_branch) {
            ast_free(condition);
            ast_free(then_branch);
            return NULL;
        }
    }

    return ast_create_if(condition, then_branch, else_branch);
}

ASTNode *parser_parse_while_statement(Parser *parser) {
    if (!parser_expect(parser, TOKEN_WHILE)) {
        parser_error(parser, "Expected 'while'");
        return NULL;
    }

    if (!parser_expect(parser, TOKEN_LPAREN)) {
        parser_error(parser, "Expected '(' after 'while'");
        return NULL;
    }

    ASTNode *condition = parser_parse_expression(parser);
    if (!condition) return NULL;

    if (!parser_expect(parser, TOKEN_RPAREN)) {
        ast_free(condition);
        parser_error(parser, "Expected ')' after while condition");
        return NULL;
    }

    ASTNode *body = parser_parse_block(parser);
    if (!body) {
        ast_free(condition);
        return NULL;
    }

    // While loops can be represented as if statements that repeat
    return ast_create_while(condition, body);
}

ASTNode *parser_parse_assignment(Parser *parser) {
    if (parser->current_token->type != TOKEN_IDENTIFIER) {
        parser_error(parser, "Expected identifier");
        return NULL;
    }

    // Save the variable name
    char *name = strdup(parser->current_token->value);
    if (!name) return NULL;

    parser_advance(parser);

    // Check for assignment operator
    if (!parser_expect(parser, TOKEN_ASSIGN)) {
        free(name);
        parser_error(parser, "Expected '=' after identifier");
        return NULL;
    }

    // Parse the expression being assigned
    ASTNode *expr = parser_parse_expression(parser);
    if (!expr) {
        free(name);
        return NULL;
    }

    if (!parser_expect(parser, TOKEN_SEMICOLON)) {
        free(name);
        ast_free(expr);
        parser_error(parser, "Expected ';' after assignment");
        return NULL;
    }

    // Create variable node
    ASTNode *var = ast_create_variable(name);
    free(name);
    if (!var) {
        ast_free(expr);
        return NULL;
    }

    // Create assignment node (using binary op node with '=' operator)
    return ast_create_binary_op('=', var, expr);
}