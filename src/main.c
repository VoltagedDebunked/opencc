#include <codegen.h>
#include <lexer.h>
#include <parser.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Read entire file into memory
char *read_file(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    perror("Error opening file");
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *buffer = malloc(size + 1);
  if (!buffer) {
    fclose(file);
    return NULL;
  }

  size_t read_size = fread(buffer, 1, size, file);
  buffer[read_size] = '\0';
  fclose(file);
  return buffer;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <input.c> <output.s>\n", argv[0]);
    return 1;
  }

  // Read input file
  char *source = read_file(argv[1]);
  if (!source) {
    fprintf(stderr, "Failed to read input file\n");
    return 1;
  }

  // Create lexer
  Lexer *lexer = lexer_create(source);
  if (!lexer) {
    fprintf(stderr, "Failed to create lexer\n");
    free(source);
    return 1;
  }

  // Create parser
  Parser *parser = parser_create(lexer);
  if (!parser) {
    fprintf(stderr, "Failed to create parser\n");
    lexer_free(lexer);
    free(source);
    return 1;
  }

  // Parse the program
  ASTNode *ast = parser_parse_program(parser);
  if (!ast) {
    fprintf(stderr, "Failed to parse program\n");
    parser_free(parser);
    lexer_free(lexer);
    free(source);
    return 1;
  }

  // Create code generator
  CodeGenerator *codegen = codegen_create(argv[2]);
  if (!codegen) {
    fprintf(stderr, "Failed to create code generator\n");
    ast_free(ast);
    parser_free(parser);
    lexer_free(lexer);
    free(source);
    return 1;
  }

  // Generate code
  codegen_generate(codegen, ast);

  // Clean up
  codegen_free(codegen);
  ast_free(ast);
  parser_free(parser);
  lexer_free(lexer);
  free(source);

  printf("Compilation successful: output written to %s\n", argv[2]);
  return 0;
}