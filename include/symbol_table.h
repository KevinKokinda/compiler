#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdlib.h>
#include <string.h>

typedef enum
{
    SYMBOL_INTEGER
} SymbolType;

typedef struct Symbol
{
    char *name;
    SymbolType type;
    int scope_level;
    int is_initialized;
    struct Symbol *next;
} Symbol;

typedef struct SymbolTable
{
    Symbol *head;
    int current_scope;
} SymbolTable;

SymbolTable *symbol_table_create(void);
void symbol_table_destroy(SymbolTable *table);

void symbol_table_enter_scope(SymbolTable *table);
void symbol_table_exit_scope(SymbolTable *table);

Symbol *symbol_table_add(SymbolTable *table, const char *name, SymbolType type);
Symbol *symbol_table_lookup(SymbolTable *table, const char *name);
Symbol *symbol_table_lookup_current_scope(SymbolTable *table, const char *name);

void symbol_table_mark_initialized(SymbolTable *table, const char *name);
int symbol_table_is_initialized(SymbolTable *table, const char *name);

void symbol_table_remove_scope(SymbolTable *table, int scope_level);
int symbol_table_variable_exists(SymbolTable *table, const char *name);

typedef struct
{
    char **variables;
    int count;
    int capacity;
} ScopeVariables;

ScopeVariables *symbol_table_get_scope_variables(SymbolTable *table, int scope_level);
void scope_variables_destroy(ScopeVariables *scope_vars);

#endif