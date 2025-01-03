#include "ast.h"
#include <stdio.h>
#include <string.h>

ASTNode *ast_create_node(ASTNodeType type)
{
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = type;
    node->line = 0;
    node->column = 0;
    return node;
}

void ast_destroy_node(ASTNode *node)
{
    if (!node)
        return;

    switch (node->type)
    {
    case NODE_PROGRAM:
    case NODE_BLOCK:
        for (size_t i = 0; i < node->data.block.statement_count; i++)
        {
            ast_destroy_node(node->data.block.statements[i]);
        }
        free(node->data.block.statements);
        break;

    case NODE_IF:
        ast_destroy_node(node->data.if_stmt.condition);
        ast_destroy_node(node->data.if_stmt.if_body);
        if (node->data.if_stmt.else_body)
        {
            ast_destroy_node(node->data.if_stmt.else_body);
        }
        break;

    case NODE_WHILE:
        ast_destroy_node(node->data.while_loop.condition);
        ast_destroy_node(node->data.while_loop.body);
        break;

    case NODE_ASSIGNMENT:
        free(node->data.assignment.name);
        ast_destroy_node(node->data.assignment.value);
        break;

    case NODE_BINARY_OP:
        ast_destroy_node(node->data.binary_op.left);
        ast_destroy_node(node->data.binary_op.right);
        break;

    case NODE_IDENTIFIER:
        free(node->data.identifier.name);
        break;

    case NODE_INTEGER:
    case NODE_ERROR:
        break;
    }

    free(node);
}

ASTNode *ast_create_integer(int value)
{
    ASTNode *node = ast_create_node(NODE_INTEGER);
    node->data.integer.value = value;
    return node;
}

ASTNode *ast_create_identifier(const char *name)
{
    ASTNode *node = ast_create_node(NODE_IDENTIFIER);
    node->data.identifier.name = strdup(name);
    return node;
}

ASTNode *ast_create_binary_op(TokenType operator, ASTNode * left, ASTNode *right)
{
    ASTNode *node = ast_create_node(NODE_BINARY_OP);
    node->data.binary_op.operator= operator;
    node->data.binary_op.left = left;
    node->data.binary_op.right = right;
    return node;
}

ASTNode *ast_create_assignment(const char *name, ASTNode *value)
{
    ASTNode *node = ast_create_node(NODE_ASSIGNMENT);
    node->data.assignment.name = strdup(name);
    node->data.assignment.value = value;
    return node;
}

ASTNode *ast_create_if(ASTNode *condition, ASTNode *if_body, ASTNode *else_body)
{
    ASTNode *node = ast_create_node(NODE_IF);
    node->data.if_stmt.condition = condition;
    node->data.if_stmt.if_body = if_body;
    node->data.if_stmt.else_body = else_body;
    return node;
}

ASTNode *ast_create_while(ASTNode *condition, ASTNode *body)
{
    ASTNode *node = ast_create_node(NODE_WHILE);
    node->data.while_loop.condition = condition;
    node->data.while_loop.body = body;
    return node;
}

ASTNode *ast_create_block(void)
{
    ASTNode *node = ast_create_node(NODE_BLOCK);
    node->data.block.statements = NULL;
    node->data.block.statement_count = 0;
    return node;
}

void ast_add_statement(ASTNode *block, ASTNode *statement)
{
    if (block->type != NODE_BLOCK && block->type != NODE_PROGRAM)
    {
        fprintf(stderr, "Error: Attempting to add statement to non-block node\n");
        return;
    }

    size_t new_size = block->data.block.statement_count + 1;
    block->data.block.statements = realloc(block->data.block.statements,
                                           new_size * sizeof(ASTNode *));
    block->data.block.statements[block->data.block.statement_count] = statement;
    block->data.block.statement_count = new_size;
}

static void ast_print_indent(int indent)
{
    for (int i = 0; i < indent; i++)
    {
        printf("  ");
    }
}

void ast_print(ASTNode *node, int indent)
{
    if (!node)
        return;

    ast_print_indent(indent);

    switch (node->type)
    {
    case NODE_PROGRAM:
        printf("Program:\n");
        for (size_t i = 0; i < node->data.block.statement_count; i++)
        {
            ast_print(node->data.block.statements[i], indent + 1);
        }
        break;

    case NODE_BLOCK:
        printf("Block:\n");
        for (size_t i = 0; i < node->data.block.statement_count; i++)
        {
            ast_print(node->data.block.statements[i], indent + 1);
        }
        break;

    case NODE_IF:
        printf("If:\n");
        ast_print_indent(indent + 1);
        printf("Condition:\n");
        ast_print(node->data.if_stmt.condition, indent + 2);
        ast_print_indent(indent + 1);
        printf("Then:\n");
        ast_print(node->data.if_stmt.if_body, indent + 2);
        if (node->data.if_stmt.else_body)
        {
            ast_print_indent(indent + 1);
            printf("Else:\n");
            ast_print(node->data.if_stmt.else_body, indent + 2);
        }
        break;

    case NODE_WHILE:
        printf("While:\n");
        ast_print_indent(indent + 1);
        printf("Condition:\n");
        ast_print(node->data.while_loop.condition, indent + 2);
        ast_print_indent(indent + 1);
        printf("Body:\n");
        ast_print(node->data.while_loop.body, indent + 2);
        break;

    case NODE_ASSIGNMENT:
        printf("Assignment: %s =\n", node->data.assignment.name);
        ast_print(node->data.assignment.value, indent + 1);
        break;

    case NODE_BINARY_OP:
        printf("BinaryOp: %d\n", node->data.binary_op.operator);
        ast_print(node->data.binary_op.left, indent + 1);
        ast_print(node->data.binary_op.right, indent + 1);
        break;

    case NODE_IDENTIFIER:
        printf("Identifier: %s\n", node->data.identifier.name);
        break;

    case NODE_INTEGER:
        printf("Integer: %d\n", node->data.integer.value);
        break;

    case NODE_ERROR:
        printf("Error\n");
        break;
    }
}