#include "parser.h"

static ASTNode *parser_parse_if_statement(Parser *parser);
static ASTNode *parser_parse_while_statement(Parser *parser);
static ASTNode *parser_parse_assignment_statement(Parser *parser);
static ASTNode *parser_parse_block_statement(Parser *parser);
static ASTNode *parser_parse_primary(Parser *parser);
static ASTNode *parser_parse_shift(Parser *parser);
static ASTNode *parser_parse_multiplicative(Parser *parser);
static ASTNode *parser_parse_additive(Parser *parser);
static ASTNode *parser_parse_comparison(Parser *parser);

int get_token_precedence(TokenType type)
{
    switch (type)
    {
    case TOKEN_EQUAL:
    case TOKEN_NOT_EQUAL:
        return PRECEDENCE_EQUALS;
    case TOKEN_LESS:
    case TOKEN_GREATER:
        return PRECEDENCE_LESSGREATER;
    case TOKEN_SHIFT_LEFT:
        return PRECEDENCE_SHIFT;
    case TOKEN_PLUS:
    case TOKEN_MINUS:
        return PRECEDENCE_SUM;
    case TOKEN_MULTIPLY:
    case TOKEN_DIVIDE:
        return PRECEDENCE_PRODUCT;
    default:
        return PRECEDENCE_LOWEST;
    }
}

Parser *parser_create(Lexer *lexer)
{
    Parser *parser = (Parser *)malloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->current_token = NULL;
    parser->peek_token = NULL;
    parser_advance_token(parser);
    parser_advance_token(parser);
    return parser;
}

void parser_destroy(Parser *parser)
{
    if (parser->current_token)
        token_destroy(parser->current_token);
    if (parser->peek_token)
        token_destroy(parser->peek_token);
    free(parser);
}

void parser_advance_token(Parser *parser)
{
    if (parser->current_token)
        token_destroy(parser->current_token);
    parser->current_token = parser->peek_token;
    parser->peek_token = lexer_next_token(parser->lexer);
}

int parser_expect_token(Parser *parser, TokenType type)
{
    if (parser->current_token->type == type)
    {
        parser_advance_token(parser);
        return 1;
    }
    return 0;
}

void parser_error(Parser *parser, const char *message)
{
    fprintf(stderr, "Parse error at line %d, column %d: %s\n",
            parser->current_token->line,
            parser->current_token->column,
            message);
}

ASTNode *parser_parse_program(Parser *parser)
{
    ASTNode *program = ast_create_node(NODE_PROGRAM);
    program->data.block.statements = NULL;
    program->data.block.statement_count = 0;

    while (parser->current_token->type != TOKEN_EOF)
    {
        ASTNode *statement = parser_parse_statement(parser);
        if (statement)
        {
            ast_add_statement(program, statement);
        }
        else
        {
            parser_advance_token(parser);
        }
    }

    return program;
}

ASTNode *parser_parse_statement(Parser *parser)
{
    switch (parser->current_token->type)
    {
    case TOKEN_IF:
        return parser_parse_if_statement(parser);
    case TOKEN_WHILE:
        return parser_parse_while_statement(parser);
    case TOKEN_IDENTIFIER:
        return parser_parse_assignment_statement(parser);
    case TOKEN_LBRACE:
        return parser_parse_block_statement(parser);
    default:
        parser_error(parser, "Unexpected statement");
        return NULL;
    }
}

static ASTNode *parser_parse_primary(Parser *parser)
{
    switch (parser->current_token->type)
    {
    case TOKEN_INTEGER:
    {
        int value = atoi(parser->current_token->value);
        ASTNode *node = ast_create_integer(value);
        parser_advance_token(parser);
        return node;
    }
    case TOKEN_IDENTIFIER:
    {
        ASTNode *node = ast_create_identifier(parser->current_token->value);
        parser_advance_token(parser);
        return node;
    }
    case TOKEN_LPAREN:
    {
        parser_advance_token(parser);
        ASTNode *expr = parser_parse_expression(parser);
        if (!parser_expect_token(parser, TOKEN_RPAREN))
        {
            parser_error(parser, "Expected ')'");
            if (expr)
                ast_destroy_node(expr);
            return NULL;
        }
        return expr;
    }
    default:
        parser_error(parser, "Unexpected token in expression");
        return NULL;
    }
}

static ASTNode *parser_parse_shift(Parser *parser)
{
    ASTNode *left = parser_parse_multiplicative(parser);
    if (!left)
        return NULL;

    while (parser->current_token->type == TOKEN_SHIFT_LEFT)
    {
        TokenType operator= parser->current_token->type;
        parser_advance_token(parser);

        ASTNode *right = parser_parse_multiplicative(parser);
        if (!right)
        {
            ast_destroy_node(left);
            return NULL;
        }

        left = ast_create_binary_op(operator, left, right);
    }

    return left;
}

static ASTNode *parser_parse_multiplicative(Parser *parser)
{
    ASTNode *left = parser_parse_primary(parser);
    if (!left)
        return NULL;

    while (parser->current_token->type == TOKEN_MULTIPLY ||
           parser->current_token->type == TOKEN_DIVIDE)
    {
        TokenType operator= parser->current_token->type;
        parser_advance_token(parser);

        ASTNode *right = parser_parse_primary(parser);
        if (!right)
        {
            ast_destroy_node(left);
            return NULL;
        }

        left = ast_create_binary_op(operator, left, right);
    }

    return left;
}

static ASTNode *parser_parse_additive(Parser *parser)
{
    ASTNode *left = parser_parse_shift(parser);
    if (!left)
        return NULL;

    while (parser->current_token->type == TOKEN_PLUS ||
           parser->current_token->type == TOKEN_MINUS)
    {
        TokenType operator= parser->current_token->type;
        parser_advance_token(parser);

        ASTNode *right = parser_parse_shift(parser);
        if (!right)
        {
            ast_destroy_node(left);
            return NULL;
        }

        left = ast_create_binary_op(operator, left, right);
    }

    return left;
}

static ASTNode *parser_parse_comparison(Parser *parser)
{
    ASTNode *left = parser_parse_additive(parser);
    if (!left)
        return NULL;

    while (parser->current_token->type == TOKEN_LESS ||
           parser->current_token->type == TOKEN_GREATER ||
           parser->current_token->type == TOKEN_EQUAL ||
           parser->current_token->type == TOKEN_NOT_EQUAL)
    {
        TokenType operator= parser->current_token->type;
        parser_advance_token(parser);

        ASTNode *right = parser_parse_additive(parser);
        if (!right)
        {
            ast_destroy_node(left);
            return NULL;
        }

        left = ast_create_binary_op(operator, left, right);
    }

    return left;
}

ASTNode *parser_parse_expression(Parser *parser)
{
    return parser_parse_comparison(parser);
}

static ASTNode *parser_parse_assignment_statement(Parser *parser)
{
    if (parser->current_token->type != TOKEN_IDENTIFIER)
    {
        parser_error(parser, "Expected identifier");
        return NULL;
    }

    char *name = strdup(parser->current_token->value);
    parser_advance_token(parser);

    if (!parser_expect_token(parser, TOKEN_ASSIGN))
    {
        free(name);
        parser_error(parser, "Expected '='");
        return NULL;
    }

    ASTNode *value = parser_parse_expression(parser);
    if (!value)
    {
        free(name);
        return NULL;
    }

    if (!parser_expect_token(parser, TOKEN_SEMICOLON))
    {
        free(name);
        ast_destroy_node(value);
        parser_error(parser, "Expected ';'");
        return NULL;
    }

    return ast_create_assignment(name, value);
}

static ASTNode *parser_parse_if_statement(Parser *parser)
{
    parser_advance_token(parser);

    if (!parser_expect_token(parser, TOKEN_LPAREN))
    {
        parser_error(parser, "Expected '('");
        return NULL;
    }

    ASTNode *condition = parser_parse_expression(parser);
    if (!condition)
        return NULL;

    if (!parser_expect_token(parser, TOKEN_RPAREN))
    {
        ast_destroy_node(condition);
        parser_error(parser, "Expected ')'");
        return NULL;
    }

    ASTNode *if_body = parser_parse_statement(parser);
    if (!if_body)
    {
        ast_destroy_node(condition);
        return NULL;
    }

    ASTNode *else_body = NULL;
    if (parser->current_token->type == TOKEN_ELSE)
    {
        parser_advance_token(parser);
        else_body = parser_parse_statement(parser);
        if (!else_body)
        {
            ast_destroy_node(condition);
            ast_destroy_node(if_body);
            return NULL;
        }
    }

    return ast_create_if(condition, if_body, else_body);
}

static ASTNode *parser_parse_while_statement(Parser *parser)
{
    parser_advance_token(parser);

    if (!parser_expect_token(parser, TOKEN_LPAREN))
    {
        parser_error(parser, "Expected '('");
        return NULL;
    }

    ASTNode *condition = parser_parse_expression(parser);
    if (!condition)
        return NULL;

    if (!parser_expect_token(parser, TOKEN_RPAREN))
    {
        ast_destroy_node(condition);
        parser_error(parser, "Expected ')'");
        return NULL;
    }

    ASTNode *body = parser_parse_statement(parser);
    if (!body)
    {
        ast_destroy_node(condition);
        return NULL;
    }

    return ast_create_while(condition, body);
}

static ASTNode *parser_parse_block_statement(Parser *parser)
{
    parser_advance_token(parser);

    ASTNode *block = ast_create_block();

    while (parser->current_token->type != TOKEN_RBRACE &&
           parser->current_token->type != TOKEN_EOF)
    {
        ASTNode *statement = parser_parse_statement(parser);
        if (statement)
        {
            ast_add_statement(block, statement);
        }
    }

    if (!parser_expect_token(parser, TOKEN_RBRACE))
    {
        ast_destroy_node(block);
        parser_error(parser, "Expected '}'");
        return NULL;
    }

    return block;
}