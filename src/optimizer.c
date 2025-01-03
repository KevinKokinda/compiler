#include "optimizer.h"

Optimizer *optimizer_create(SymbolTable *symbol_table)
{
    Optimizer *optimizer = (Optimizer *)malloc(sizeof(Optimizer));
    optimizer->symbol_table = symbol_table;
    optimizer->changes_made = 0;
    optimizer->options.constant_folding_enabled = 1;
    optimizer->options.dead_code_elimination_enabled = 1;
    optimizer->options.strength_reduction_enabled = 1;
    return optimizer;
}

void optimizer_destroy(Optimizer *optimizer)
{
    free(optimizer);
}

void optimizer_set_options(Optimizer *optimizer, OptimizerOptions options)
{
    optimizer->options = options;
}

ASTNode *optimizer_optimize(Optimizer *optimizer, ASTNode *ast)
{
    if (!ast)
        return NULL;

    do
    {
        optimizer->changes_made = 0;

        if (optimizer->options.constant_folding_enabled)
        {
            ast = optimizer_constant_folding(optimizer, ast);
        }

        if (optimizer->options.dead_code_elimination_enabled)
        {
            ast = optimizer_dead_code_elimination(optimizer, ast);
        }

        if (optimizer->options.strength_reduction_enabled)
        {
            ast = optimizer_strength_reduction(optimizer, ast);
        }
    } while (optimizer->changes_made);

    return ast;
}

ASTNode *optimizer_constant_folding(Optimizer *optimizer, ASTNode *node)
{
    if (!node)
        return NULL;

    switch (node->type)
    {
    case NODE_PROGRAM:
    case NODE_BLOCK:
        for (size_t i = 0; i < node->data.block.statement_count; i++)
        {
            node->data.block.statements[i] = optimizer_constant_folding(optimizer, node->data.block.statements[i]);
        }
        break;

    case NODE_IF:
        node->data.if_stmt.condition = optimizer_constant_folding(optimizer, node->data.if_stmt.condition);
        node->data.if_stmt.if_body = optimizer_constant_folding(optimizer, node->data.if_stmt.if_body);
        if (node->data.if_stmt.else_body)
        {
            node->data.if_stmt.else_body = optimizer_constant_folding(optimizer, node->data.if_stmt.else_body);
        }
        break;

    case NODE_WHILE:
        node->data.while_loop.condition = optimizer_constant_folding(optimizer, node->data.while_loop.condition);
        node->data.while_loop.body = optimizer_constant_folding(optimizer, node->data.while_loop.body);
        break;

    case NODE_ASSIGNMENT:
        node->data.assignment.value = optimizer_constant_folding(optimizer, node->data.assignment.value);
        break;

    case NODE_BINARY_OP:
        node->data.binary_op.left = optimizer_constant_folding(optimizer, node->data.binary_op.left);
        node->data.binary_op.right = optimizer_constant_folding(optimizer, node->data.binary_op.right);

        if (optimizer_is_constant(node->data.binary_op.left) &&
            optimizer_is_constant(node->data.binary_op.right))
        {
            int result = optimizer_evaluate_constant_expression(node);
            ASTNode *constant_node = ast_create_integer(result);
            ast_destroy_node(node);
            optimizer->changes_made = 1;
            return constant_node;
        }
        break;

    default:
        break;
    }

    return node;
}

ASTNode *optimizer_dead_code_elimination(Optimizer *optimizer, ASTNode *node)
{
    if (!node)
        return NULL;

    switch (node->type)
    {
    case NODE_PROGRAM:
    case NODE_BLOCK:
    {
        size_t new_count = 0;
        for (size_t i = 0; i < node->data.block.statement_count; i++)
        {
            ASTNode *stmt = optimizer_dead_code_elimination(optimizer, node->data.block.statements[i]);
            if (stmt)
            {
                node->data.block.statements[new_count++] = stmt;
            }
            else
            {
                optimizer->changes_made = 1;
            }
        }
        node->data.block.statement_count = new_count;
        break;
    }

    case NODE_IF:
        if (optimizer_is_constant(node->data.if_stmt.condition))
        {
            int condition_value = optimizer_evaluate_constant_expression(node->data.if_stmt.condition);
            ASTNode *result = condition_value ? node->data.if_stmt.if_body : node->data.if_stmt.else_body;

            if (condition_value)
            {
                node->data.if_stmt.if_body = NULL;
            }
            else
            {
                node->data.if_stmt.else_body = NULL;
            }

            ast_destroy_node(node);
            optimizer->changes_made = 1;
            return result;
        }

        node->data.if_stmt.condition = optimizer_dead_code_elimination(optimizer, node->data.if_stmt.condition);
        node->data.if_stmt.if_body = optimizer_dead_code_elimination(optimizer, node->data.if_stmt.if_body);
        if (node->data.if_stmt.else_body)
        {
            node->data.if_stmt.else_body = optimizer_dead_code_elimination(optimizer, node->data.if_stmt.else_body);
        }
        break;

    case NODE_WHILE:
        if (optimizer_is_constant(node->data.while_loop.condition))
        {
            int condition_value = optimizer_evaluate_constant_expression(node->data.while_loop.condition);
            if (!condition_value)
            {
                ast_destroy_node(node);
                optimizer->changes_made = 1;
                return NULL;
            }
        }

        node->data.while_loop.condition = optimizer_dead_code_elimination(optimizer, node->data.while_loop.condition);
        node->data.while_loop.body = optimizer_dead_code_elimination(optimizer, node->data.while_loop.body);
        break;

    case NODE_ASSIGNMENT:
        node->data.assignment.value = optimizer_dead_code_elimination(optimizer, node->data.assignment.value);
        break;

    case NODE_BINARY_OP:
        node->data.binary_op.left = optimizer_dead_code_elimination(optimizer, node->data.binary_op.left);
        node->data.binary_op.right = optimizer_dead_code_elimination(optimizer, node->data.binary_op.right);
        break;

    default:
        break;
    }

    return node;
}

ASTNode *optimizer_strength_reduction(Optimizer *optimizer, ASTNode *node)
{
    if (!node)
        return NULL;

    if (node->type == NODE_BINARY_OP)
    {
        if (node->data.binary_op.operator== TOKEN_MULTIPLY &&
            optimizer_is_constant(node->data.binary_op.right))
        {
            int value = node->data.binary_op.right->data.integer.value;
            if ((value & (value - 1)) == 0)
            {
                int shift = 0;
                while (value > 1)
                {
                    value >>= 1;
                    shift++;
                }
                ASTNode *shift_amount = ast_create_integer(shift);
                node->data.binary_op.operator= TOKEN_SHIFT_LEFT;
                ast_destroy_node(node->data.binary_op.right);
                node->data.binary_op.right = shift_amount;
                optimizer->changes_made = 1;
            }
        }
    }

    switch (node->type)
    {
    case NODE_PROGRAM:
    case NODE_BLOCK:
        for (size_t i = 0; i < node->data.block.statement_count; i++)
        {
            node->data.block.statements[i] = optimizer_strength_reduction(optimizer, node->data.block.statements[i]);
        }
        break;

    case NODE_IF:
        node->data.if_stmt.condition = optimizer_strength_reduction(optimizer, node->data.if_stmt.condition);
        node->data.if_stmt.if_body = optimizer_strength_reduction(optimizer, node->data.if_stmt.if_body);
        if (node->data.if_stmt.else_body)
        {
            node->data.if_stmt.else_body = optimizer_strength_reduction(optimizer, node->data.if_stmt.else_body);
        }
        break;

    case NODE_WHILE:
        node->data.while_loop.condition = optimizer_strength_reduction(optimizer, node->data.while_loop.condition);
        node->data.while_loop.body = optimizer_strength_reduction(optimizer, node->data.while_loop.body);
        break;

    case NODE_ASSIGNMENT:
        node->data.assignment.value = optimizer_strength_reduction(optimizer, node->data.assignment.value);
        break;

    case NODE_BINARY_OP:
        node->data.binary_op.left = optimizer_strength_reduction(optimizer, node->data.binary_op.left);
        node->data.binary_op.right = optimizer_strength_reduction(optimizer, node->data.binary_op.right);
        break;

    default:
        break;
    }

    return node;
}

int optimizer_evaluate_constant_expression(ASTNode *node)
{
    if (!node)
        return 0;

    switch (node->type)
    {
    case NODE_INTEGER:
        return node->data.integer.value;

    case NODE_BINARY_OP:
        if (optimizer_is_constant(node->data.binary_op.left) &&
            optimizer_is_constant(node->data.binary_op.right))
        {
            int left = optimizer_evaluate_constant_expression(node->data.binary_op.left);
            int right = optimizer_evaluate_constant_expression(node->data.binary_op.right);

            switch (node->data.binary_op.operator)
            {
            case TOKEN_PLUS:
                return left + right;
            case TOKEN_MINUS:
                return left - right;
            case TOKEN_MULTIPLY:
                return left * right;
            case TOKEN_DIVIDE:
                return right != 0 ? left / right : 0;
            case TOKEN_LESS:
                return left < right;
            case TOKEN_GREATER:
                return left > right;
            case TOKEN_EQUAL:
                return left == right;
            case TOKEN_NOT_EQUAL:
                return left != right;
            case TOKEN_SHIFT_LEFT:
                return left << right;
            default:
                return 0;
            }
        }
        break;

    default:
        break;
    }

    return 0;
}

int optimizer_is_constant(ASTNode *node)
{
    return node && node->type == NODE_INTEGER;
}

UsedVariables *optimizer_find_used_variables(ASTNode *node)
{
    UsedVariables *used = (UsedVariables *)malloc(sizeof(UsedVariables));
    used->vars = NULL;
    used->count = 0;

    if (!node)
        return used;

    if (node->type == NODE_IDENTIFIER)
    {
        used->vars = (char **)malloc(sizeof(char *));
        used->vars[0] = strdup(node->data.identifier.name);
        used->count = 1;
        return used;
    }

    switch (node->type)
    {
    case NODE_PROGRAM:
    case NODE_BLOCK:
        for (size_t i = 0; i < node->data.block.statement_count; i++)
        {
            UsedVariables *child_used = optimizer_find_used_variables(node->data.block.statements[i]);
            used->vars = realloc(used->vars, (used->count + child_used->count) * sizeof(char *));
            memcpy(used->vars + used->count, child_used->vars, child_used->count * sizeof(char *));
            used->count += child_used->count;
            free(child_used->vars);
            free(child_used);
        }
        break;

    case NODE_IF:
    {
        UsedVariables *cond_used = optimizer_find_used_variables(node->data.if_stmt.condition);
        UsedVariables *if_used = optimizer_find_used_variables(node->data.if_stmt.if_body);
        UsedVariables *else_used = node->data.if_stmt.else_body ? optimizer_find_used_variables(node->data.if_stmt.else_body) : NULL;

        size_t total_size = cond_used->count + if_used->count;
        if (else_used)
            total_size += else_used->count;

        used->vars = realloc(used->vars, total_size * sizeof(char *));
        memcpy(used->vars, cond_used->vars, cond_used->count * sizeof(char *));
        used->count = cond_used->count;

        memcpy(used->vars + used->count, if_used->vars, if_used->count * sizeof(char *));
        used->count += if_used->count;

        if (else_used)
        {
            memcpy(used->vars + used->count, else_used->vars, else_used->count * sizeof(char *));
            used->count += else_used->count;
            used_variables_destroy(else_used);
        }

        used_variables_destroy(cond_used);
        used_variables_destroy(if_used);
    }
    break;

    case NODE_WHILE:
    {
        UsedVariables *cond_used = optimizer_find_used_variables(node->data.while_loop.condition);
        UsedVariables *body_used = optimizer_find_used_variables(node->data.while_loop.body);

        used->vars = realloc(used->vars, (cond_used->count + body_used->count) * sizeof(char *));
        memcpy(used->vars, cond_used->vars, cond_used->count * sizeof(char *));
        used->count = cond_used->count;

        memcpy(used->vars + used->count, body_used->vars, body_used->count * sizeof(char *));
        used->count += body_used->count;

        used_variables_destroy(cond_used);
        used_variables_destroy(body_used);
    }
    break;

    case NODE_ASSIGNMENT:
    {
        UsedVariables *value_used = optimizer_find_used_variables(node->data.assignment.value);
        used->vars = realloc(used->vars, (value_used->count + 1) * sizeof(char *));
        memcpy(used->vars, value_used->vars, value_used->count * sizeof(char *));
        used->count = value_used->count;
        used->vars[used->count++] = strdup(node->data.assignment.name);
        used_variables_destroy(value_used);
    }
    break;

    case NODE_BINARY_OP:
    {
        UsedVariables *left_used = optimizer_find_used_variables(node->data.binary_op.left);
        UsedVariables *right_used = optimizer_find_used_variables(node->data.binary_op.right);

        used->vars = realloc(used->vars, (left_used->count + right_used->count) * sizeof(char *));
        memcpy(used->vars, left_used->vars, left_used->count * sizeof(char *));
        used->count = left_used->count;

        memcpy(used->vars + used->count, right_used->vars, right_used->count * sizeof(char *));
        used->count += right_used->count;

        used_variables_destroy(left_used);
        used_variables_destroy(right_used);
    }
    break;

    default:
        break;
    }

    return used;
}

void used_variables_destroy(UsedVariables *used_vars)
{
    if (used_vars)
    {
        for (int i = 0; i < used_vars->count; i++)
        {
            free(used_vars->vars[i]);
        }
        free(used_vars->vars);
        free(used_vars);
    }
}