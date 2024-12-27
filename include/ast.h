#ifndef AST_H
#define AST_H

#include <stdbool.h>

typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION,
    NODE_EXTERN_FUNCTION,
    NODE_BLOCK,
    NODE_RETURN,
    NODE_IF,
    NODE_WHILE,
    NODE_FOR,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_VARIABLE,
    NODE_NUMBER,
    NODE_STRING,
    NODE_CHAR,
    NODE_CALL,
    NODE_ASSIGNMENT
} NodeType;

typedef struct ASTNode {
    NodeType type;
    union {
        // Program node
        struct {
            struct ASTNode **functions;  // Will now include both function and extern function nodes
            int function_count;
        } program;
        
        // Function node (both regular and extern functions)
        struct {
            char *name;
            char **params;
            int param_count;
            struct ASTNode *body;  // NULL for extern functions
        } function;
        
        // Block node
        struct {
            struct ASTNode **statements;
            int statement_count;
        } block;
        
        // Return node
        struct {
            struct ASTNode *expression;
        } return_stmt;
        
        // If node
        struct {
            struct ASTNode *condition;
            struct ASTNode *then_branch;
            struct ASTNode *else_branch;
        } if_stmt;

        // While node
        struct {
            struct ASTNode *condition;
            struct ASTNode *body;
        } while_stmt;
        
        // Binary operation node
        struct {
            char operator;
            struct ASTNode *left;
            struct ASTNode *right;
        } binary_op;

        // Unary operation node
        struct {
            char operator;
            struct ASTNode *operand;
        } unary_op;
        
        // Variable/identifier node
        struct {
            char *name;
        } variable;
        
        // Number literal node
        struct {
            int value;
        } number;

        // String literal node
        struct {
            char *value;
        } string;

        // Character literal node
        struct {
            char value;
        } char_literal;

        // Function call node
        struct {
            char *name;
            struct ASTNode **args;
            int arg_count;
        } call;
    } data;
} ASTNode;

// AST management functions
ASTNode *ast_create_node(NodeType type);
void ast_free(ASTNode *node);

// Node creation helper functions
ASTNode *ast_create_program(void);
ASTNode *ast_create_function(const char *name, char **params, int param_count, ASTNode *body);
ASTNode *ast_create_extern_function(const char *name, char **params, int param_count);
ASTNode *ast_create_block(void);
ASTNode *ast_create_return(ASTNode *expression);
ASTNode *ast_create_if(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch);
ASTNode *ast_create_while(ASTNode *condition, ASTNode *body);
ASTNode *ast_create_binary_op(char operator, ASTNode *left, ASTNode *right);
ASTNode *ast_create_unary_op(char operator, ASTNode *operand);
ASTNode *ast_create_number(int value);
ASTNode *ast_create_variable(const char *name);
ASTNode *ast_create_string(const char *value);
ASTNode *ast_create_char(char value);
ASTNode *ast_create_call(const char *name, ASTNode **args, int arg_count);

#endif // AST_H