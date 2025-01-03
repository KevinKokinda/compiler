#include "symbol_table.h"
#include <stdio.h>

SymbolTable *symbol_table_create(void)
{
    SymbolTable *table = (SymbolTable *)malloc(sizeof(SymbolTable));
    table->head = NULL;
    table->current_scope = 0;
    return table;
}

void symbol_table_destroy(SymbolTable *table)
{
    Symbol *current = table->head;
    while (current != NULL)
    {
        Symbol *next = current->next;
        free(current->name);
        free(current);
        current = next;
    }
    free(table);
}

void symbol_table_enter_scope(SymbolTable *table)
{
    table->current_scope++;
}

void symbol_table_exit_scope(SymbolTable *table)
{
    symbol_table_remove_scope(table, table->current_scope);
    table->current_scope--;
}

Symbol *symbol_table_add(SymbolTable *table, const char *name, SymbolType type)
{
    // sym check
    Symbol *existing = symbol_table_lookup_current_scope(table, name);
    if (existing != NULL)
    {
        return NULL; 
    }

    Symbol *symbol = (Symbol *)malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->type = type;
    symbol->scope_level = table->current_scope;
    symbol->is_initialized = 0;

    
    symbol->next = table->head;
    table->head = symbol;

    return symbol;
}

Symbol *symbol_table_lookup(SymbolTable *table, const char *name)
{
    Symbol *current = table->head;
    Symbol *most_recent = NULL;
    int highest_scope = -1;

    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
        {
            if (current->scope_level > highest_scope)
            {
                most_recent = current;
                highest_scope = current->scope_level;
            }
        }
        current = current->next;
    }

    return most_recent;
}

Symbol *symbol_table_lookup_current_scope(SymbolTable *table, const char *name)
{
    Symbol *current = table->head;

    while (current != NULL)
    {
        if (current->scope_level == table->current_scope &&
            strcmp(current->name, name) == 0)
        {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

void symbol_table_mark_initialized(SymbolTable *table, const char *name)
{
    Symbol *symbol = symbol_table_lookup(table, name);
    if (symbol != NULL)
    {
        symbol->is_initialized = 1;
    }
}

int symbol_table_is_initialized(SymbolTable *table, const char *name)
{
    Symbol *symbol = symbol_table_lookup(table, name);
    return (symbol != NULL && symbol->is_initialized);
}

void symbol_table_remove_scope(SymbolTable *table, int scope_level)
{
    Symbol *current = table->head;
    Symbol *prev = NULL;

    while (current != NULL)
    {
        Symbol *next = current->next;

        if (current->scope_level == scope_level)
        {
            // Remove this symbol
            if (prev == NULL)
            {
                table->head = next;
            }
            else
            {
                prev->next = next;
            }
            free(current->name);
            free(current);
        }
        else
        {
            prev = current;
        }

        current = next;
    }
}

int symbol_table_variable_exists(SymbolTable *table, const char *name)
{
    return symbol_table_lookup(table, name) != NULL;
}

ScopeVariables *symbol_table_get_scope_variables(SymbolTable *table, int scope_level)
{
    ScopeVariables *scope_vars = (ScopeVariables *)malloc(sizeof(ScopeVariables));
    scope_vars->variables = NULL;
    scope_vars->count = 0;
    scope_vars->capacity = 0;

    Symbol *current = table->head;
    while (current != NULL)
    {
        if (current->scope_level == scope_level)
        {
            if (scope_vars->count >= scope_vars->capacity)
            {
                scope_vars->capacity = (scope_vars->capacity == 0) ? 8 : scope_vars->capacity * 2;
                scope_vars->variables = realloc(scope_vars->variables,
                                                scope_vars->capacity * sizeof(char *));
            }
            scope_vars->variables[scope_vars->count++] = strdup(current->name);
        }
        current = current->next;
    }

    return scope_vars;
}

void scope_variables_destroy(ScopeVariables *scope_vars)
{
    if (scope_vars)
    {
        for (int i = 0; i < scope_vars->count; i++)
        {
            free(scope_vars->variables[i]);
        }
        free(scope_vars->variables);
        free(scope_vars);
    }
}