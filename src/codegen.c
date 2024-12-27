#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <codegen.h>

static const char *arg_registers[] = {
    "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"
};
static const int MAX_ARGS_IN_REGISTERS = 6;

CodeGenerator *codegen_create(const char *output_file) {
    CodeGenerator *gen = malloc(sizeof(CodeGenerator));
    if (!gen) return NULL;

    gen->output = fopen(output_file, "w");
    if (!gen->output) {
        free(gen);
        return NULL;
    }

    gen->label_count = 0;
    return gen;
}

void codegen_free(CodeGenerator *gen) {
    if (gen) {
        if (gen->output) fclose(gen->output);
        free(gen);
    }
}

void codegen_emit(CodeGenerator *gen, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(gen->output, format, args);
    va_end(args);
    fprintf(gen->output, "\n");
}

char *codegen_new_label(CodeGenerator *gen) {
    char label[32];
    snprintf(label, sizeof(label), ".L%d", gen->label_count++);
    return strdup(label);
}

void codegen_generate(CodeGenerator *gen, ASTNode *ast) {
    // Data section
    codegen_emit(gen, "\t.section .data");
    
    // BSS section
    codegen_emit(gen, "\t.section .bss");

    // Text section
    codegen_emit(gen, "\t.section .text");
    
    // Generate all function declarations first
    for (int i = 0; i < ast->data.program.function_count; i++) {
        codegen_emit(gen, "\t.global %s", 
            ast->data.program.functions[i]->data.function.name);
        codegen_emit(gen, "\t.type %s, @function", 
            ast->data.program.functions[i]->data.function.name);
    }

    // Generate the actual functions
    for (int i = 0; i < ast->data.program.function_count; i++) {
        codegen_function(gen, ast->data.program.functions[i]);
    }

    // Entry point last
    codegen_emit(gen, "\t.global _start");
    codegen_emit(gen, "\t.type _start, @function");
    codegen_emit(gen, "_start:");
    codegen_emit(gen, "\t# Set up stack frame");
    codegen_emit(gen, "\tmovq %%rsp, %%rbp");
    codegen_emit(gen, "\t# Align stack to 16 bytes");
    codegen_emit(gen, "\tandq $-16, %%rsp");
    
    // Call main
    codegen_emit(gen, "\tcall main");
    
    // Exit syscall
    codegen_emit(gen, "\t# Exit syscall");
    codegen_emit(gen, "\tmovq %%rax, %%rdi");   // Return value from main as exit status
    codegen_emit(gen, "\tmovq $60, %%rax");     // sys_exit
    codegen_emit(gen, "\tsyscall");
    
    // Size directive for _start
    codegen_emit(gen, "\t.size _start, .-_start");
}

void codegen_function(CodeGenerator *gen, ASTNode *node) {
    if (node->type != NODE_FUNCTION) return;

    // Function prologue
    codegen_emit(gen, "\t.align 16");
    codegen_emit(gen, "%s:", node->data.function.name);
    
    // System V AMD64 ABI stack frame setup
    codegen_emit(gen, "\tpushq %%rbp");               // Save old frame pointer
    codegen_emit(gen, "\tmovq %%rsp, %%rbp");        // Set up new frame pointer
    
    // Reserve stack space for local variables
    // Round up to maintain 16-byte stack alignment
    int stack_size = ((node->data.function.param_count * 8 + 15) & ~15);
    if (stack_size > 0) {
        codegen_emit(gen, "\tsubq $%d, %%rsp", stack_size);
    }

    // Save any used callee-saved registers
    codegen_emit(gen, "\tpushq %%rbx");
    codegen_emit(gen, "\tpushq %%r12");
    codegen_emit(gen, "\tpushq %%r13");
    codegen_emit(gen, "\tpushq %%r14");
    codegen_emit(gen, "\tpushq %%r15");

    // Handle parameters according to System V AMD64 ABI
    for (int i = 0; i < node->data.function.param_count && i < MAX_ARGS_IN_REGISTERS; i++) {
        codegen_emit(gen, "\tmovq %s, %d(%%rbp)", arg_registers[i], -(i+1)*8);
    }

    // Generate code for function body
    codegen_block(gen, node->data.function.body);

    // Function epilogue
    codegen_emit(gen, ".%s_return:", node->data.function.name);
    
    // Restore callee-saved registers
    codegen_emit(gen, "\tpopq %%r15");
    codegen_emit(gen, "\tpopq %%r14");
    codegen_emit(gen, "\tpopq %%r13");
    codegen_emit(gen, "\tpopq %%r12");
    codegen_emit(gen, "\tpopq %%rbx");
    
    codegen_emit(gen, "\tmovq %%rbp, %%rsp");
    codegen_emit(gen, "\tpopq %%rbp");
    codegen_emit(gen, "\tret");
    
    // Add size directive for debugging
    codegen_emit(gen, "\t.size %s, .-%s", 
                node->data.function.name, 
                node->data.function.name);
}

void codegen_block(CodeGenerator *gen, ASTNode *node) {
    if (node->type != NODE_BLOCK) return;

    for (int i = 0; i < node->data.block.statement_count; i++) {
        codegen_statement(gen, node->data.block.statements[i]);
    }
}

void codegen_statement(CodeGenerator *gen, ASTNode *node) {
    switch (node->type) {
        case NODE_RETURN:
            codegen_expression(gen, node->data.return_stmt.expression);
            codegen_emit(gen, "\tjmp .%s_return", "main");  // TODO: current function name
            break;

        case NODE_IF: {
            char *else_label = codegen_new_label(gen);
            char *end_label = codegen_new_label(gen);

            codegen_expression(gen, node->data.if_stmt.condition);
            codegen_emit(gen, "\tcmpq $0, %%rax");
            codegen_emit(gen, "\tje %s", else_label);

            codegen_block(gen, node->data.if_stmt.then_branch);
            codegen_emit(gen, "\tjmp %s", end_label);

            codegen_emit(gen, "%s:", else_label);
            if (node->data.if_stmt.else_branch) {
                codegen_block(gen, node->data.if_stmt.else_branch);
            }
            codegen_emit(gen, "%s:", end_label);

            free(else_label);
            free(end_label);
            break;
        }

        case NODE_WHILE: {
            char *start_label = codegen_new_label(gen);
            char *end_label = codegen_new_label(gen);

            codegen_emit(gen, "%s:", start_label);
            codegen_expression(gen, node->data.while_stmt.condition);
            codegen_emit(gen, "\tcmpq $0, %%rax");
            codegen_emit(gen, "\tje %s", end_label);

            codegen_block(gen, node->data.while_stmt.body);
            codegen_emit(gen, "\tjmp %s", start_label);
            codegen_emit(gen, "%s:", end_label);

            free(start_label);
            free(end_label);
            break;
        }

        default:
            codegen_expression(gen, node);
            break;
    }
}

void codegen_expression(CodeGenerator *gen, ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case NODE_NUMBER:
            codegen_emit(gen, "\tmovq $%d, %%rax", node->data.number.value);
            break;

        case NODE_BINARY_OP:
            // Generate right operand first
            codegen_expression(gen, node->data.binary_op.right);
            codegen_emit(gen, "\tpushq %%rax");
            
            // Generate left operand
            codegen_expression(gen, node->data.binary_op.left);
            codegen_emit(gen, "\tpopq %%rcx");

            // Perform operation
            switch (node->data.binary_op.operator) {
                case '+':
                    codegen_emit(gen, "\taddq %%rcx, %%rax");
                    break;
                case '-':
                    codegen_emit(gen, "\tsubq %%rcx, %%rax");
                    break;
                case '*':
                    codegen_emit(gen, "\timulq %%rcx, %%rax");
                    break;
                case '/':
                    codegen_emit(gen, "\tcqo");        // Sign extend rax into rdx
                    codegen_emit(gen, "\tidivq %%rcx");
                    break;
                case '>':
                    codegen_emit(gen, "\tcmpq %%rcx, %%rax");
                    codegen_emit(gen, "\tsetg %%al");
                    codegen_emit(gen, "\tmovzbq %%al, %%rax");
                    break;
                case '<':
                    codegen_emit(gen, "\tcmpq %%rcx, %%rax");
                    codegen_emit(gen, "\tsetl %%al");
                    codegen_emit(gen, "\tmovzbq %%al, %%rax");
                    break;
                case '=':
                    // Handle assignment
                    break;
            }
            break;

        case NODE_VARIABLE:
            // This would require symbol table lookup
            break;

        default:
            // Handle other expression types
            break;
    }
}