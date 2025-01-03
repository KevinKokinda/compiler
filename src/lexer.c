#include "lexer.h"

static void lexer_advance(Lexer *lexer);
static void lexer_skip_whitespace(Lexer *lexer);
static void lexer_skip_comment(Lexer *lexer);
static Token *lexer_make_token(TokenType type, char *value, int line, int column);
static char *lexer_read_identifier(Lexer *lexer);
static char *lexer_read_number(Lexer *lexer);

static void lexer_skip_comment(Lexer *lexer)
{
    lexer_advance(lexer); // Skip first '/'
    lexer_advance(lexer); // Skip second '/'

    // Skip until end of line or EOF
    while (lexer->current_char != '\0' && lexer->current_char != '\n')
    {
        lexer_advance(lexer);
    }
}

Lexer *lexer_create(char *source)
{
    Lexer *lexer = (Lexer *)malloc(sizeof(Lexer));
    lexer->source = source;
    lexer->source_length = strlen(source);
    lexer->current_pos = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->current_char = (lexer->source_length > 0) ? source[0] : '\0';
    return lexer;
}

void lexer_destroy(Lexer *lexer)
{
    free(lexer);
}

static void lexer_advance(Lexer *lexer)
{
    if (lexer->current_pos < lexer->source_length)
    {
        if (lexer->current_char == '\n')
        {
            lexer->line++;
            lexer->column = 1;
        }
        else
        {
            lexer->column++;
        }
        lexer->current_pos++;
        lexer->current_char = (lexer->current_pos < lexer->source_length) ? lexer->source[lexer->current_pos] : '\0';
    }
}

static void lexer_skip_whitespace(Lexer *lexer)
{
    while (lexer->current_char != '\0' && isspace(lexer->current_char))
    {
        lexer_advance(lexer);
    }
}

static Token *lexer_make_token(TokenType type, char *value, int line, int column)
{
    Token *token = (Token *)malloc(sizeof(Token));
    token->type = type;
    token->value = value;
    token->line = line;
    token->column = column;
    return token;
}

static char *lexer_read_identifier(Lexer *lexer)
{
    char buffer[MAX_IDENTIFIER_LENGTH + 1];
    int i = 0;

    while (lexer->current_char != '\0' &&
           (isalnum(lexer->current_char) || lexer->current_char == '_'))
    {
        if (i < MAX_IDENTIFIER_LENGTH)
        {
            buffer[i++] = lexer->current_char;
        }
        lexer_advance(lexer);
    }
    buffer[i] = '\0';

    return strdup(buffer);
}

static char *lexer_read_number(Lexer *lexer)
{
    char buffer[MAX_INTEGER_LENGTH + 1];
    int i = 0;

    while (lexer->current_char != '\0' && isdigit(lexer->current_char))
    {
        if (i < MAX_INTEGER_LENGTH)
        {
            buffer[i++] = lexer->current_char;
        }
        lexer_advance(lexer);
    }
    buffer[i] = '\0';

    return strdup(buffer);
}

Token *lexer_next_token(Lexer *lexer)
{
    lexer_skip_whitespace(lexer);

    int current_line = lexer->line;
    int current_column = lexer->column;

    if (lexer->current_char == '\0')
    {
        return lexer_make_token(TOKEN_EOF, NULL, current_line, current_column);
    }

    // Handle comments
    if (lexer->current_char == '/' &&
        lexer->current_pos + 1 < lexer->source_length &&
        lexer->source[lexer->current_pos + 1] == '/')
    {
        lexer_skip_comment(lexer);
        return lexer_next_token(lexer);
    }

    if (isalpha(lexer->current_char) || lexer->current_char == '_')
    {
        char *identifier = lexer_read_identifier(lexer);

        if (strcmp(identifier, "if") == 0)
        {
            return lexer_make_token(TOKEN_IF, identifier, current_line, current_column);
        }
        else if (strcmp(identifier, "else") == 0)
        {
            return lexer_make_token(TOKEN_ELSE, identifier, current_line, current_column);
        }
        else if (strcmp(identifier, "while") == 0)
        {
            return lexer_make_token(TOKEN_WHILE, identifier, current_line, current_column);
        }

        return lexer_make_token(TOKEN_IDENTIFIER, identifier, current_line, current_column);
    }

    if (isdigit(lexer->current_char))
    {
        return lexer_make_token(TOKEN_INTEGER, lexer_read_number(lexer),
                                current_line, current_column);
    }

    Token *token = NULL;
    switch (lexer->current_char)
    {
    case '+':
        token = lexer_make_token(TOKEN_PLUS, strdup("+"), current_line, current_column);
        break;
    case '-':
        token = lexer_make_token(TOKEN_MINUS, strdup("-"), current_line, current_column);
        break;
    case '*':
        token = lexer_make_token(TOKEN_MULTIPLY, strdup("*"), current_line, current_column);
        break;
    case '/':
        token = lexer_make_token(TOKEN_DIVIDE, strdup("/"), current_line, current_column);
        break;
    case '<':
        lexer_advance(lexer);
        if (lexer->current_char == '<')
        {
            token = lexer_make_token(TOKEN_SHIFT_LEFT, strdup("<<"), current_line, current_column);
            lexer_advance(lexer);
        }
        else
        {
            token = lexer_make_token(TOKEN_LESS, strdup("<"), current_line, current_column);
        }
        return token;
    case '=':
        lexer_advance(lexer);
        if (lexer->current_char == '=')
        {
            token = lexer_make_token(TOKEN_EQUAL, strdup("=="), current_line, current_column);
            lexer_advance(lexer);
        }
        else
        {
            token = lexer_make_token(TOKEN_ASSIGN, strdup("="), current_line, current_column);
        }
        return token;
    case '!':
        lexer_advance(lexer);
        if (lexer->current_char == '=')
        {
            token = lexer_make_token(TOKEN_NOT_EQUAL, strdup("!="), current_line, current_column);
            lexer_advance(lexer);
            return token;
        }
        token = lexer_make_token(TOKEN_ERROR, NULL, current_line, current_column);
        break;
    case '>':
        token = lexer_make_token(TOKEN_GREATER, strdup(">"), current_line, current_column);
        break;
    case '(':
        token = lexer_make_token(TOKEN_LPAREN, strdup("("), current_line, current_column);
        break;
    case ')':
        token = lexer_make_token(TOKEN_RPAREN, strdup(")"), current_line, current_column);
        break;
    case '{':
        token = lexer_make_token(TOKEN_LBRACE, strdup("{"), current_line, current_column);
        break;
    case '}':
        token = lexer_make_token(TOKEN_RBRACE, strdup("}"), current_line, current_column);
        break;
    case ';':
        token = lexer_make_token(TOKEN_SEMICOLON, strdup(";"), current_line, current_column);
        break;
    default:
        token = lexer_make_token(TOKEN_ERROR, NULL, current_line, current_column);
    }

    lexer_advance(lexer);
    return token;
}

void token_destroy(Token *token)
{
    if (token->value)
    {
        free(token->value);
    }
    free(token);
}

const char *token_type_to_string(TokenType type)
{
    switch (type)
    {
    case TOKEN_EOF:
        return "EOF";
    case TOKEN_IDENTIFIER:
        return "IDENTIFIER";
    case TOKEN_INTEGER:
        return "INTEGER";
    case TOKEN_PLUS:
        return "PLUS";
    case TOKEN_MINUS:
        return "MINUS";
    case TOKEN_MULTIPLY:
        return "MULTIPLY";
    case TOKEN_DIVIDE:
        return "DIVIDE";
    case TOKEN_SHIFT_LEFT:
        return "SHIFT_LEFT";
    case TOKEN_ASSIGN:
        return "ASSIGN";
    case TOKEN_SEMICOLON:
        return "SEMICOLON";
    case TOKEN_LPAREN:
        return "LPAREN";
    case TOKEN_RPAREN:
        return "RPAREN";
    case TOKEN_LBRACE:
        return "LBRACE";
    case TOKEN_RBRACE:
        return "RBRACE";
    case TOKEN_IF:
        return "IF";
    case TOKEN_ELSE:
        return "ELSE";
    case TOKEN_WHILE:
        return "WHILE";
    case TOKEN_LESS:
        return "LESS";
    case TOKEN_GREATER:
        return "GREATER";
    case TOKEN_EQUAL:
        return "EQUAL";
    case TOKEN_NOT_EQUAL:
        return "NOT_EQUAL";
    case TOKEN_ERROR:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}