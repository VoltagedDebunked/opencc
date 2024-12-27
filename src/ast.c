#include <stdlib.h>
#include <string.h>
#include <ast.h>

ASTNode *ast_create_node(NodeType type) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type = type;
    memset(&node->data, 0, sizeof(node->data));
    return node;
}

void ast_free(ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case NODE_PROGRAM:
            for (int i = 0; i < node->data.program.function_count; i++) {
                ast_free(node->data.program.functions[i]);
            }
            free(node->data.program.functions);
            break;

        case NODE_FUNCTION:
            for (int i = 0; i < node->data.function.param_count; i++) {
                free(node->data.function.params[i]);
            }
            free(node->data.function.params);
            free(node->data.function.name);
            ast_free(node->data.function.body);
            break;

        case NODE_BLOCK:
            for (int i = 0; i < node->data.block.statement_count; i++) {
                ast_free(node->data.block.statements[i]);
            }
            free(node->data.block.statements);
            break;

        case NODE_RETURN:
            ast_free(node->data.return_stmt.expression);
            break;

        case NODE_IF:
            ast_free(node->data.if_stmt.condition);
            ast_free(node->data.if_stmt.then_branch);
            ast_free(node->data.if_stmt.else_branch);
            break;

        case NODE_WHILE:
            ast_free(node->data.while_stmt.condition);
            ast_free(node->data.while_stmt.body);
            break;

        case NODE_FOR:
            // TODO: Implement for loop cleanup
            break;

        case NODE_BINARY_OP:
            ast_free(node->data.binary_op.left);
            ast_free(node->data.binary_op.right);
            break;

        case NODE_UNARY_OP:
            ast_free(node->data.unary_op.operand);
            break;

        case NODE_VARIABLE:
            free(node->data.variable.name);
            break;

        case NODE_NUMBER:
            // Nothing to free for number literals
            break;

        case NODE_STRING:
            free(node->data.string.value);
            break;

        case NODE_CHAR:
            // Nothing to free for char literals
            break;

        case NODE_CALL:
            free(node->data.call.name);
            for (int i = 0; i < node->data.call.arg_count; i++) {
                ast_free(node->data.call.args[i]);
            }
            free(node->data.call.args);
            break;

        case NODE_ASSIGNMENT:
            // TODO: Implement assignment cleanup
            break;
    }

    free(node);
}

ASTNode *ast_create_program(void) {
    ASTNode *node = ast_create_node(NODE_PROGRAM);
    if (!node) return NULL;

    node->data.program.functions = NULL;
    node->data.program.function_count = 0;
    return node;
}

ASTNode *ast_create_function(const char *name, char **params, int param_count, ASTNode *body) {
    ASTNode *node = ast_create_node(NODE_FUNCTION);
    if (!node) return NULL;

    node->data.function.name = strdup(name);
    if (!node->data.function.name) {
        ast_free(node);
        return NULL;
    }

    node->data.function.params = malloc(sizeof(char *) * param_count);
    if (!node->data.function.params) {
        ast_free(node);
        return NULL;
    }

    for (int i = 0; i < param_count; i++) {
        node->data.function.params[i] = strdup(params[i]);
        if (!node->data.function.params[i]) {
            ast_free(node);
            return NULL;
        }
    }

    node->data.function.param_count = param_count;
    node->data.function.body = body;
    return node;
}

ASTNode *ast_create_block(void) {
    ASTNode *node = ast_create_node(NODE_BLOCK);
    if (!node) return NULL;

    node->data.block.statements = NULL;
    node->data.block.statement_count = 0;
    return node;
}

ASTNode *ast_create_return(ASTNode *expression) {
    ASTNode *node = ast_create_node(NODE_RETURN);
    if (!node) return NULL;

    node->data.return_stmt.expression = expression;
    return node;
}

ASTNode *ast_create_if(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch) {
    ASTNode *node = ast_create_node(NODE_IF);
    if (!node) return NULL;

    node->data.if_stmt.condition = condition;
    node->data.if_stmt.then_branch = then_branch;
    node->data.if_stmt.else_branch = else_branch;
    return node;
}

ASTNode *ast_create_while(ASTNode *condition, ASTNode *body) {
    ASTNode *node = ast_create_node(NODE_WHILE);
    if (!node) return NULL;

    node->data.while_stmt.condition = condition;
    node->data.while_stmt.body = body;
    return node;
}

ASTNode *ast_create_binary_op(char operator, ASTNode *left, ASTNode *right) {
    ASTNode *node = ast_create_node(NODE_BINARY_OP);
    if (!node) return NULL;

    node->data.binary_op.operator = operator;
    node->data.binary_op.left = left;
    node->data.binary_op.right = right;
    return node;
}

ASTNode *ast_create_unary_op(char operator, ASTNode *operand) {
    ASTNode *node = ast_create_node(NODE_UNARY_OP);
    if (!node) return NULL;

    node->data.unary_op.operator = operator;
    node->data.unary_op.operand = operand;
    return node;
}

ASTNode *ast_create_number(int value) {
    ASTNode *node = ast_create_node(NODE_NUMBER);
    if (!node) return NULL;

    node->data.number.value = value;
    return node;
}

ASTNode *ast_create_variable(const char *name) {
    ASTNode *node = ast_create_node(NODE_VARIABLE);
    if (!node) return NULL;

    node->data.variable.name = strdup(name);
    if (!node->data.variable.name) {
        ast_free(node);
        return NULL;
    }
    return node;
}

ASTNode *ast_create_string(const char *value) {
    ASTNode *node = ast_create_node(NODE_STRING);
    if (!node) return NULL;

    node->data.string.value = strdup(value);
    if (!node->data.string.value) {
        ast_free(node);
        return NULL;
    }
    return node;
}

ASTNode *ast_create_char(char value) {
    ASTNode *node = ast_create_node(NODE_CHAR);
    if (!node) return NULL;

    node->data.char_literal.value = value;
    return node;
}

ASTNode *ast_create_call(const char *name, ASTNode **args, int arg_count) {
    ASTNode *node = ast_create_node(NODE_CALL);
    if (!node) return NULL;

    node->data.call.name = strdup(name);
    if (!node->data.call.name) {
        ast_free(node);
        return NULL;
    }

    if (arg_count > 0) {
        node->data.call.args = malloc(sizeof(ASTNode*) * arg_count);
        if (!node->data.call.args) {
            ast_free(node);
            return NULL;
        }
        memcpy(node->data.call.args, args, sizeof(ASTNode*) * arg_count);
    } else {
        node->data.call.args = NULL;
    }
    
    node->data.call.arg_count = arg_count;
    return node;
}