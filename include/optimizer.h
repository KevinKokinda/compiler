#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "ast.h"
#include "symbol_table.h"

typedef struct
{
    int constant_folding_enabled;
    int dead_code_elimination_enabled;
    int strength_reduction_enabled;
} OptimizerOptions;

typedef struct
{
    OptimizerOptions options;
    SymbolTable *symbol_table;
    int changes_made;
} Optimizer;

Optimizer *optimizer_create(SymbolTable *symbol_table);
void optimizer_destroy(Optimizer *optimizer);

void optimizer_set_options(Optimizer *optimizer, OptimizerOptions options);
ASTNode *optimizer_optimize(Optimizer *optimizer, ASTNode *ast);

ASTNode *optimizer_constant_folding(Optimizer *optimizer, ASTNode *node);
ASTNode *optimizer_dead_code_elimination(Optimizer *optimizer, ASTNode *node);
ASTNode *optimizer_strength_reduction(Optimizer *optimizer, ASTNode *node);

int optimizer_evaluate_constant_expression(ASTNode *node);
int optimizer_is_constant(ASTNode *node);

typedef struct
{
    char **vars;
    int count;
} UsedVariables;

UsedVariables *optimizer_find_used_variables(ASTNode *node);
void used_variables_destroy(UsedVariables *used_vars);

int optimizer_can_eliminate_code(ASTNode *node);
ASTNode *optimizer_simplify_expression(ASTNode *node);

#endif