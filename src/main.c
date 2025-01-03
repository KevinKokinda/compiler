#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "symbol_table.h"
#include "optimizer.h"
#include "codegen.h"

char *read_file(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Error opening file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(length + 1);
    if (!buffer)
    {
        fclose(file);
        return NULL;
    }

    size_t read_length = fread(buffer, 1, length, file);
    buffer[read_length] = '\0';

    fclose(file);
    return buffer;
}

void print_tokens(const char *source)
{
    printf("\nSource contents:\n%s\n", source);
    printf("\nTokenizing source...\n");

    Lexer *debug_lexer = lexer_create((char *)source);
    Token *token;
    int token_count = 0;

    while (1)
    {
        token = lexer_next_token(debug_lexer);
        printf("Token %d: Type: %-15s Value: %-15s Line: %d Column: %d\n",
               ++token_count,
               token_type_to_string(token->type),
               token->value ? token->value : "NULL",
               token->line,
               token->column);

        if (token->type == TOKEN_EOF)
        {
            token_destroy(token);
            break;
        }
        token_destroy(token);
    }

    printf("\nFinished tokenizing.\n\n");
    lexer_destroy(debug_lexer);
}

int compile_file(const char *input_filename, const char *output_filename)
{
    char *source = read_file(input_filename);
    if (!source)
    {
        fprintf(stderr, "Failed to read input file: %s\n", input_filename);
        return 1;
    }

    // Debug: Print file contents and tokens
    print_tokens(source);

    FILE *output_file = fopen(output_filename, "w");
    if (!output_file)
    {
        free(source);
        perror("Error opening output file");
        return 1;
    }

    Lexer *lexer = lexer_create(source);
    Parser *parser = parser_create(lexer);
    SymbolTable *symbol_table = symbol_table_create();
    Optimizer *optimizer = optimizer_create(symbol_table);
    CodeGenerator *generator = codegen_create(output_file, symbol_table);

    ASTNode *ast = parser_parse_program(parser);
    if (!ast)
    {
        fprintf(stderr, "Parsing failed\n");
        goto cleanup;
    }

    ast = optimizer_optimize(optimizer, ast);
    if (!codegen_generate(generator, ast))
    {
        fprintf(stderr, "Code generation failed\n");
        goto cleanup;
    }

    ast_destroy_node(ast);
    codegen_destroy(generator);
    optimizer_destroy(optimizer);
    symbol_table_destroy(symbol_table);
    parser_destroy(parser);
    lexer_destroy(lexer);
    fclose(output_file);
    free(source);

    return 0;

cleanup:
    if (ast)
        ast_destroy_node(ast);
    codegen_destroy(generator);
    optimizer_destroy(optimizer);
    symbol_table_destroy(symbol_table);
    parser_destroy(parser);
    lexer_destroy(lexer);
    fclose(output_file);
    free(source);
    return 1;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <input.sl> <output.asm>\n", argv[0]);
        return 1;
    }

    return compile_file(argv[1], argv[2]);
}