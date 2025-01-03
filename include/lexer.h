#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_IDENTIFIER_LENGTH 255
#define MAX_INTEGER_LENGTH 32

typedef enum
{
    TOKEN_EOF = 0,
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_SHIFT_LEFT,
    TOKEN_ASSIGN,
    TOKEN_SEMICOLON,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_LESS,
    TOKEN_GREATER,
    TOKEN_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_ERROR
} TokenType;

typedef struct
{
    TokenType type;
    char *value;
    int line;
    int column;
} Token;

typedef struct
{
    char *source;
    size_t source_length;
    size_t current_pos;
    size_t line;
    size_t column;
    char current_char;
} Lexer;

Lexer *lexer_create(char *source);
void lexer_destroy(Lexer *lexer);
Token *lexer_next_token(Lexer *lexer);
void token_destroy(Token *token);
const char *token_type_to_string(TokenType type);

#endif