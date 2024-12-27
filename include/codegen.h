#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include <stdio.h>

typedef struct {
    FILE *output;
    int label_count;
    // Symbol table could be added here
} CodeGenerator;

// Code generator management functions
CodeGenerator *codegen_create(const char *output_file);
void codegen_free(CodeGenerator *gen);

// Code generation functions
void codegen_generate(CodeGenerator *gen, ASTNode *ast);
void codegen_program(CodeGenerator *gen, ASTNode *node);
void codegen_function(CodeGenerator *gen, ASTNode *node);
void codegen_block(CodeGenerator *gen, ASTNode *node);
void codegen_statement(CodeGenerator *gen, ASTNode *node);
void codegen_expression(CodeGenerator *gen, ASTNode *node);

// Helper functions
void codegen_emit(CodeGenerator *gen, const char *format, ...);
char *codegen_new_label(CodeGenerator *gen);

#endif // CODEGEN_H