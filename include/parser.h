#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct
{
    Lexer *lexer;
    Token *current_token;
    Token *peek_token;
} Parser;

typedef enum
{
    PRECEDENCE_LOWEST,
    PRECEDENCE_EQUALS,      // ==
    PRECEDENCE_LESSGREATER, // < >
    PRECEDENCE_SHIFT,       // << >>
    PRECEDENCE_SUM,         // + -
    PRECEDENCE_PRODUCT,     // * /
    PRECEDENCE_PREFIX       // -X or !X
} Precedence;

Parser *parser_create(Lexer *lexer);
void parser_destroy(Parser *parser);

ASTNode *parser_parse_program(Parser *parser);
ASTNode *parser_parse_statement(Parser *parser);
ASTNode *parser_parse_expression(Parser *parser);

void parser_advance_token(Parser *parser);
int parser_expect_token(Parser *parser, TokenType type);
void parser_error(Parser *parser, const char *message);
int get_token_precedence(TokenType type);

#endif