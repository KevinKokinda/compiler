#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include "lexer.h"

typedef enum
{
    NODE_PROGRAM,
    NODE_BLOCK,
    NODE_IF,
    NODE_WHILE,
    NODE_ASSIGNMENT,
    NODE_BINARY_OP,
    NODE_IDENTIFIER,
    NODE_INTEGER,
    NODE_ERROR
} ASTNodeType;

typedef struct ASTNode
{
    ASTNodeType type;
    union
    {
        struct
        {
            struct ASTNode **statements;
            size_t statement_count;
        } block;

        struct
        {
            struct ASTNode *condition;
            struct ASTNode *if_body;
            struct ASTNode *else_body;
        } if_stmt;

        struct
        {
            struct ASTNode *condition;
            struct ASTNode *body;
        } while_loop;

        struct
        {
            char *name;
            struct ASTNode *value;
        } assignment;

        struct
        {
            TokenType operator;
            struct ASTNode *left;
            struct ASTNode *right;
        } binary_op;

        struct
        {
            char *name;
        } identifier;

        struct
        {
            int value;
        } integer;
    } data;

    int line;
    int column;
} ASTNode;

ASTNode *ast_create_node(ASTNodeType type);
void ast_destroy_node(ASTNode *node);

ASTNode *ast_create_integer(int value);
ASTNode *ast_create_identifier(const char *name);
ASTNode *ast_create_binary_op(TokenType operator, ASTNode * left, ASTNode *right);
ASTNode *ast_create_assignment(const char *name, ASTNode *value);
ASTNode *ast_create_if(ASTNode *condition, ASTNode *if_body, ASTNode *else_body);
ASTNode *ast_create_while(ASTNode *condition, ASTNode *body);
ASTNode *ast_create_block(void);
void ast_add_statement(ASTNode *block, ASTNode *statement);

void ast_print(ASTNode *node, int indent);

#endif