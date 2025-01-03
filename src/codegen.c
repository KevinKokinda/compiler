#include "codegen.h"

const char *registers[] = {"rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"};
const int NUM_REGISTERS = 10;

CodeGenerator *codegen_create(FILE *output_file, SymbolTable *symbol_table)
{
    CodeGenerator *generator = (CodeGenerator *)malloc(sizeof(CodeGenerator));
    generator->output_file = output_file;
    generator->symbol_table = symbol_table;
    generator->label_counter = 0;
    generator->stack_offset = 0;
    generator->used_registers = (char **)calloc(NUM_REGISTERS, sizeof(char *));
    generator->register_count = 0;
    generator->options.assembler = ASM_NASM;
    generator->options.optimize_registers = 1;
    generator->options.generate_comments = 1;
    return generator;
}

void codegen_destroy(CodeGenerator *generator)
{
    free(generator->used_registers);
    free(generator);
}

void codegen_emit_prologue(CodeGenerator *generator)
{
    fprintf(generator->output_file, "section .text\n");
    fprintf(generator->output_file, "global main\n");
    fprintf(generator->output_file, "main:\n");
    fprintf(generator->output_file, "    push rbp\n");
    fprintf(generator->output_file, "    mov rbp, rsp\n");
}

void codegen_emit_epilogue(CodeGenerator *generator)
{
    fprintf(generator->output_file, "    mov rsp, rbp\n");
    fprintf(generator->output_file, "    pop rbp\n");
    fprintf(generator->output_file, "    xor eax, eax\n");
    fprintf(generator->output_file, "    ret\n");
}

char *codegen_new_label(CodeGenerator *generator)
{
    char *label = (char *)malloc(16);
    sprintf(label, ".L%d", generator->label_counter++);
    return label;
}

int codegen_allocate_register(CodeGenerator *generator)
{
    for (int i = 0; i < NUM_REGISTERS; i++)
    {
        if (!generator->used_registers[i])
        {
            generator->used_registers[i] = (char *)1;
            generator->register_count++;
            return i;
        }
    }
    return -1;
}

void codegen_free_register(CodeGenerator *generator, int reg)
{
    if (reg >= 0 && reg < NUM_REGISTERS)
    {
        generator->used_registers[reg] = NULL;
        generator->register_count--;
    }
}

void codegen_emit_binary_op(CodeGenerator *generator, ASTNode *node)
{
    codegen_emit_expression(generator, node->data.binary_op.left);
    int left_reg = codegen_allocate_register(generator);
    fprintf(generator->output_file, "    mov %s, rax\n", registers[left_reg]);

    codegen_emit_expression(generator, node->data.binary_op.right);
    int right_reg = codegen_allocate_register(generator);
    fprintf(generator->output_file, "    mov %s, rax\n", registers[right_reg]);

    switch (node->data.binary_op.operator)
    {
    case TOKEN_PLUS:
        fprintf(generator->output_file, "    add %s, %s\n", registers[left_reg], registers[right_reg]);
        fprintf(generator->output_file, "    mov rax, %s\n", registers[left_reg]);
        break;
    case TOKEN_MINUS:
        fprintf(generator->output_file, "    sub %s, %s\n", registers[left_reg], registers[right_reg]);
        fprintf(generator->output_file, "    mov rax, %s\n", registers[left_reg]);
        break;
    case TOKEN_MULTIPLY:
        fprintf(generator->output_file, "    imul %s, %s\n", registers[left_reg], registers[right_reg]);
        fprintf(generator->output_file, "    mov rax, %s\n", registers[left_reg]);
        break;
    case TOKEN_DIVIDE:
        fprintf(generator->output_file, "    mov rax, %s\n", registers[left_reg]);
        fprintf(generator->output_file, "    cqo\n");
        fprintf(generator->output_file, "    idiv %s\n", registers[right_reg]);
        break;
    case TOKEN_SHIFT_LEFT:
        fprintf(generator->output_file, "    mov rax, %s\n", registers[left_reg]);
        fprintf(generator->output_file, "    mov rcx, %s\n", registers[right_reg]);
        fprintf(generator->output_file, "    shl rax, cl\n");
        break;
    case TOKEN_LESS:
        fprintf(generator->output_file, "    cmp %s, %s\n", registers[left_reg], registers[right_reg]);
        fprintf(generator->output_file, "    setl al\n");
        fprintf(generator->output_file, "    movzx rax, al\n");
        break;
    case TOKEN_GREATER:
        fprintf(generator->output_file, "    cmp %s, %s\n", registers[left_reg], registers[right_reg]);
        fprintf(generator->output_file, "    setg al\n");
        fprintf(generator->output_file, "    movzx rax, al\n");
        break;
    case TOKEN_EQUAL:
        fprintf(generator->output_file, "    cmp %s, %s\n", registers[left_reg], registers[right_reg]);
        fprintf(generator->output_file, "    sete al\n");
        fprintf(generator->output_file, "    movzx rax, al\n");
        break;
    default:
        break;
    }

    codegen_free_register(generator, right_reg);
    codegen_free_register(generator, left_reg);
}

void codegen_emit_expression(CodeGenerator *generator, ASTNode *node)
{
    if (!node)
        return;

    switch (node->type)
    {
    case NODE_INTEGER:
        fprintf(generator->output_file, "    mov rax, %d\n", node->data.integer.value);
        break;
    case NODE_IDENTIFIER:
        fprintf(generator->output_file, "    mov rax, [rbp-%d]\n",
                codegen_get_variable_offset(generator, node->data.identifier.name));
        break;
    case NODE_BINARY_OP:
        codegen_emit_binary_op(generator, node);
        break;
    default:
        break;
    }
}

void codegen_emit_statement(CodeGenerator *generator, ASTNode *node)
{
    if (!node)
        return;

    switch (node->type)
    {
    case NODE_ASSIGNMENT:
    {
        codegen_emit_expression(generator, node->data.assignment.value);
        int offset = codegen_get_variable_offset(generator, node->data.assignment.name);
        generator->stack_offset = offset;
        fprintf(generator->output_file, "    mov [rbp-%d], rax\n", offset);
        break;
    }
    case NODE_IF:
    {
        char *else_label = codegen_new_label(generator);
        char *end_label = codegen_new_label(generator);

        codegen_emit_expression(generator, node->data.if_stmt.condition);
        fprintf(generator->output_file, "    cmp rax, 0\n");
        fprintf(generator->output_file, "    je %s\n", else_label);

        codegen_emit_statement(generator, node->data.if_stmt.if_body);
        fprintf(generator->output_file, "    jmp %s\n", end_label);

        fprintf(generator->output_file, "%s:\n", else_label);
        if (node->data.if_stmt.else_body)
        {
            codegen_emit_statement(generator, node->data.if_stmt.else_body);
        }

        fprintf(generator->output_file, "%s:\n", end_label);
        free(else_label);
        free(end_label);
        break;
    }
    case NODE_WHILE:
    {
        char *start_label = codegen_new_label(generator);
        char *end_label = codegen_new_label(generator);

        fprintf(generator->output_file, "%s:\n", start_label);
        codegen_emit_expression(generator, node->data.while_loop.condition);
        fprintf(generator->output_file, "    cmp rax, 0\n");
        fprintf(generator->output_file, "    je %s\n", end_label);

        codegen_emit_statement(generator, node->data.while_loop.body);
        fprintf(generator->output_file, "    jmp %s\n", start_label);
        fprintf(generator->output_file, "%s:\n", end_label);

        free(start_label);
        free(end_label);
        break;
    }
    case NODE_BLOCK:
        for (size_t i = 0; i < node->data.block.statement_count; i++)
        {
            codegen_emit_statement(generator, node->data.block.statements[i]);
        }
        break;
    default:
        break;
    }
}

int codegen_get_variable_offset(CodeGenerator *generator, const char *name)
{
    Symbol *symbol = symbol_table_lookup(generator->symbol_table, name);
    if (!symbol)
    {
        symbol = symbol_table_add(generator->symbol_table, name, SYMBOL_INTEGER);
        generator->stack_offset += 8;
        fprintf(generator->output_file, "    sub rsp, 8\n");
    }
    return generator->stack_offset;
}

int codegen_generate(CodeGenerator *generator, ASTNode *ast)
{
    codegen_emit_prologue(generator);
    codegen_emit_statement(generator, ast);
    codegen_emit_epilogue(generator);
    return 1;
}