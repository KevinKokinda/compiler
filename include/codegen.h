#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include "symbol_table.h"

typedef enum
{
    ASM_NASM,
    ASM_GAS
} AssemblerType;

typedef struct
{
    AssemblerType assembler;
    int optimize_registers;
    int generate_comments;
} CodeGenOptions;

typedef struct
{
    FILE *output_file;
    SymbolTable *symbol_table;
    CodeGenOptions options;
    int label_counter;
    int stack_offset;
    char **used_registers;
    int register_count;
} CodeGenerator;

CodeGenerator *codegen_create(FILE *output_file, SymbolTable *symbol_table);
void codegen_destroy(CodeGenerator *generator);

void codegen_set_options(CodeGenerator *generator, CodeGenOptions options);
int codegen_generate(CodeGenerator *generator, ASTNode *ast);

void codegen_emit_prologue(CodeGenerator *generator);
void codegen_emit_epilogue(CodeGenerator *generator);

void codegen_emit_expression(CodeGenerator *generator, ASTNode *node);
void codegen_emit_statement(CodeGenerator *generator, ASTNode *node);
void codegen_emit_block(CodeGenerator *generator, ASTNode *node);

int codegen_allocate_register(CodeGenerator *generator);
void codegen_free_register(CodeGenerator *generator, int reg);

void codegen_emit_binary_op(CodeGenerator *generator, ASTNode *node);
void codegen_emit_assignment(CodeGenerator *generator, ASTNode *node);
void codegen_emit_if(CodeGenerator *generator, ASTNode *node);
void codegen_emit_while(CodeGenerator *generator, ASTNode *node);

char *codegen_new_label(CodeGenerator *generator);
int codegen_get_variable_offset(CodeGenerator *generator, const char *name);

#endif