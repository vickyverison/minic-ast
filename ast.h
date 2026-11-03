#ifndef AST_H
#define AST_H

#include <stdio.h>

typedef enum {
    AST_NUMBER,
    AST_VARIABLE,
    AST_BINARY,
    AST_UNARY,
    AST_DECLARATION,
    AST_ASSIGNMENT,
    AST_PRINT,
    AST_IF,
    AST_WHILE,
    AST_BLOCK
} AstType;

typedef struct AstNode {
    AstType type;
    long value;
    char op[3];
    char *name;

    struct AstNode *left;
    struct AstNode *right;
    struct AstNode *third;
    struct AstNode *next;
} AstNode;

AstNode *ast_number(long value);
AstNode *ast_variable(const char *name);
AstNode *ast_binary(const char *op, AstNode *left, AstNode *right);
AstNode *ast_unary(const char *op, AstNode *child);
AstNode *ast_declaration(const char *name, AstNode *initializer);
AstNode *ast_assignment(const char *name, AstNode *value);
AstNode *ast_print(AstNode *expression);
AstNode *ast_if(AstNode *condition, AstNode *then_branch, AstNode *else_branch);
AstNode *ast_while(AstNode *condition, AstNode *body);
AstNode *ast_block(AstNode *statements);
AstNode *ast_append(AstNode *list, AstNode *item);

void ast_print_tree(const AstNode *node, int depth);
void ast_free(AstNode *node);
int ast_execute(const AstNode *root);
void ast_generate_python(const AstNode *root, FILE *out);
void ast_write_json(const AstNode *root, FILE *out);

#endif