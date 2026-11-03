#include "ast.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static AstNode *new_node(AstType type)
{
    AstNode *node = calloc(1, sizeof(*node));
    if (!node) {
        fprintf(stderr, "Out of memory\n");
        exit(2);
    }
    node->type = type;
    return node;
}

static char *copy_string(const char *text)
{
    size_t size = strlen(text) + 1;
    char *copy = malloc(size);
    if (!copy) {
        fprintf(stderr, "Out of memory\n");
        exit(2);
    }
    memcpy(copy, text, size);
    return copy;
}

static void set_op(AstNode *node, const char *op)
{
    snprintf(node->op, sizeof(node->op), "%s", op);
}

AstNode *ast_number(long value)
{
    AstNode *node = new_node(AST_NUMBER);
    node->value = value;
    return node;
}

AstNode *ast_variable(const char *name)
{
    AstNode *node = new_node(AST_VARIABLE);
    node->name = copy_string(name);
    return node;
}

AstNode *ast_binary(const char *op, AstNode *left, AstNode *right)
{
    AstNode *node = new_node(AST_BINARY);
    set_op(node, op);
    node->left = left;
    node->right = right;
    return node;
}

AstNode *ast_unary(const char *op, AstNode *child)
{
    AstNode *node = new_node(AST_UNARY);
    set_op(node, op);
    node->left = child;
    return node;
}

AstNode *ast_declaration(const char *name, AstNode *initializer)
{
    AstNode *node = new_node(AST_DECLARATION);
    node->name = copy_string(name);
    node->left = initializer;
    return node;
}

AstNode *ast_assignment(const char *name, AstNode *value)
{
    AstNode *node = new_node(AST_ASSIGNMENT);
    node->name = copy_string(name);
    node->left = value;
    return node;
}

AstNode *ast_print(AstNode *expression)
{
    AstNode *node = new_node(AST_PRINT);
    node->left = expression;
    return node;
}

AstNode *ast_if(AstNode *condition, AstNode *then_branch, AstNode *else_branch)
{
    AstNode *node = new_node(AST_IF);
    node->left = condition;
    node->right = then_branch;
    node->third = else_branch;
    return node;
}

AstNode *ast_while(AstNode *condition, AstNode *body)
{
    AstNode *node = new_node(AST_WHILE);
    node->left = condition;
    node->right = body;
    return node;
}

AstNode *ast_block(AstNode *statements)
{
    AstNode *node = new_node(AST_BLOCK);
    node->left = statements;
    return node;
}

AstNode *ast_append(AstNode *list, AstNode *item)
{
    if (!list) {
        return item;
    }

    AstNode *last = list;
    while (last->next) {
        last = last->next;
    }
    last->next = item;
    return list;
}

static const char *type_name(AstType type)
{
    switch (type) {
    case AST_NUMBER:      return "Number";
    case AST_VARIABLE:    return "Variable";
    case AST_BINARY:      return "Binary";
    case AST_UNARY:       return "Unary";
    case AST_DECLARATION: return "Declaration";
    case AST_ASSIGNMENT:  return "Assignment";
    case AST_PRINT:       return "Print";
    case AST_IF:          return "If";
    case AST_WHILE:       return "While";
    case AST_BLOCK:       return "Block";
    }
    return "Unknown";
}

void ast_print_tree(const AstNode *node, int depth)
{
    if (!node) {
        return;
    }

    for (int i = 0; i < depth; ++i) {
        fputs("  ", stdout);
    }

    printf("%s", type_name(node->type));

    if (node->type == AST_NUMBER) {
        printf(": %ld", node->value);
    } else if (node->type == AST_VARIABLE ||
               node->type == AST_DECLARATION ||
               node->type == AST_ASSIGNMENT) {
        printf(": %s", node->name);
    } else if (node->type == AST_BINARY || node->type == AST_UNARY) {
        printf(": %s", node->op);
    }

    putchar('\n');

    ast_print_tree(node->left, depth + 1);
    ast_print_tree(node->right, depth + 1);
    ast_print_tree(node->third, depth + 1);
    ast_print_tree(node->next, depth);
}

void ast_free(AstNode *node)
{
    if (!node) {
        return;
    }

    free(node->name);
    ast_free(node->left);
    ast_free(node->right);
    ast_free(node->third);
    ast_free(node->next);
    free(node);
}

/* Semantic evaluation */

typedef struct {
    const char *name;
    long value;
    int initialized;
} Variable;

typedef struct {
    Variable variables[1024];
    size_t count;
    unsigned long steps;
    int failed;
} Context;

static void semantic_error(Context *context, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    fputs("Semantic error: ", stderr);
    vfprintf(stderr, format, args);
    fputc('\n', stderr);
    va_end(args);
    context->failed = 1;
}

static int find_variable(const Context *context, const char *name)
{
    for (size_t i = 0; i < context->count; ++i) {
        if (strcmp(context->variables[i].name, name) == 0) {
            return (int)i;
        }
    }
    return -1;
}

static long evaluate_expression(const AstNode *node, Context *context)
{
    if (!node || context->failed) {
        return 0;
    }

    if (node->type == AST_NUMBER) {
        return node->value;
    }

    if (node->type == AST_VARIABLE) {
        int index = find_variable(context, node->name);
        if (index < 0) {
            semantic_error(context, "variable '%s' was not declared", node->name);
            return 0;
        }
        if (!context->variables[index].initialized) {
            semantic_error(context, "variable '%s' is used before initialization",
                           node->name);
            return 0;
        }
        return context->variables[index].value;
    }

    if (node->type == AST_UNARY) {
        long value = evaluate_expression(node->left, context);
        if (strcmp(node->op, "-") == 0) {
            return -value;
        }
        semantic_error(context, "unsupported unary operator '%s'", node->op);
        return 0;
    }

    if (node->type != AST_BINARY) {
        semantic_error(context, "expected an expression");
        return 0;
    }

    long left = evaluate_expression(node->left, context);
    long right = evaluate_expression(node->right, context);
    if (context->failed) {
        return 0;
    }

    if (strcmp(node->op, "+") == 0)  return left + right;
    if (strcmp(node->op, "-") == 0)  return left - right;
    if (strcmp(node->op, "*") == 0)  return left * right;
    if (strcmp(node->op, "/") == 0) {
        if (right == 0) {
            semantic_error(context, "division by zero");
            return 0;
        }
        return left / right;
    }
    if (strcmp(node->op, "==") == 0) return left == right;
    if (strcmp(node->op, "!=") == 0) return left != right;
    if (strcmp(node->op, "<") == 0)  return left < right;
    if (strcmp(node->op, ">") == 0)  return left > right;
    if (strcmp(node->op, "<=") == 0) return left <= right;
    if (strcmp(node->op, ">=") == 0) return left >= right;

    semantic_error(context, "unsupported operator '%s'", node->op);
    return 0;
}

static void execute_statement(const AstNode *node, Context *context)
{
    if (!node || context->failed) {
        return;
    }

    if (++context->steps > 100000) {
        semantic_error(context, "execution limit reached");
        return;
    }

    switch (node->type) {
    case AST_BLOCK:
        for (const AstNode *item = node->left; item && !context->failed;
             item = item->next) {
            execute_statement(item, context);
        }
        break;

    case AST_DECLARATION: {
        if (find_variable(context, node->name) >= 0) {
            semantic_error(context, "variable '%s' was already declared", node->name);
            break;
        }
        if (context->count >= 1024) {
            semantic_error(context, "too many variables");
            break;
        }

        Variable *variable = &context->variables[context->count++];
        variable->name = node->name;
        variable->initialized = node->left != NULL;
        variable->value = node->left ? evaluate_expression(node->left, context) : 0;
        break;
    }

    case AST_ASSIGNMENT: {
        int index = find_variable(context, node->name);
        if (index < 0) {
            semantic_error(context, "variable '%s' was not declared", node->name);
            break;
        }
        long value = evaluate_expression(node->left, context);
        if (!context->failed) {
            context->variables[index].value = value;
            context->variables[index].initialized = 1;
        }
        break;
    }

    case AST_PRINT: {
        long value = evaluate_expression(node->left, context);
        if (!context->failed) {
            printf("%ld\n", value);
        }
        break;
    }

    case AST_IF: {
        long condition = evaluate_expression(node->left, context);
        if (!context->failed) {
            execute_statement(condition ? node->right : node->third, context);
        }
        break;
    }

    case AST_WHILE:
        for (;;) {
            long condition = evaluate_expression(node->left, context);
            if (context->failed || !condition) {
                break;
            }
            if (++context->steps > 100000) {
                semantic_error(context, "execution limit reached");
                break;
            }
            execute_statement(node->right, context);
        }
        break;

    default:
        semantic_error(context, "unexpected node in statement");
        break;
    }
}

int ast_execute(const AstNode *root)
{
    Context context = {0};
    execute_statement(root, &context);
    return context.failed ? 1 : 0;
}

/* Python code generation */

static void indent(FILE *out, int depth)
{
    for (int i = 0; i < depth; ++i) {
        fputs("    ", out);
    }
}

static void emit_expression(const AstNode *node, FILE *out)
{
    if (!node) {
        fputs("0", out);
        return;
    }

    switch (node->type) {
    case AST_NUMBER:
        fprintf(out, "%ld", node->value);
        break;
    case AST_VARIABLE:
        fputs(node->name, out);
        break;
    case AST_UNARY:
        fprintf(out, "(-");
        emit_expression(node->left, out);
        fputs(")", out);
        break;
    case AST_BINARY:
        if (strcmp(node->op, "/") == 0) {
            fputs("int((", out);
            emit_expression(node->left, out);
            fputs(") / (", out);
            emit_expression(node->right, out);
            fputs("))", out);
        } else {
            fputc('(', out);
            emit_expression(node->left, out);
            fprintf(out, " %s ", node->op);
            emit_expression(node->right, out);
            fputc(')', out);
        }
        break;
    default:
        fputs("0", out);
        break;
    }
}

static void emit_statement(const AstNode *node, FILE *out, int depth);

static void emit_suite(const AstNode *node, FILE *out, int depth)
{
    if (!node) {
        indent(out, depth);
        fputs("pass\n", out);
        return;
    }

    if (node->type == AST_BLOCK) {
        if (!node->left) {
            indent(out, depth);
            fputs("pass\n", out);
            return;
        }
        for (const AstNode *item = node->left; item; item = item->next) {
            emit_statement(item, out, depth);
        }
    } else {
        emit_statement(node, out, depth);
    }
}

static void emit_statement(const AstNode *node, FILE *out, int depth)
{
    if (!node) {
        return;
    }

    switch (node->type) {
    case AST_BLOCK:
        emit_suite(node, out, depth);
        break;

    case AST_DECLARATION:
        indent(out, depth);
        fprintf(out, "%s = ", node->name);
        if (node->left) {
            emit_expression(node->left, out);
        } else {
            fputs("None", out);
        }
        fputc('\n', out);
        break;

    case AST_ASSIGNMENT:
        indent(out, depth);
        fprintf(out, "%s = ", node->name);
        emit_expression(node->left, out);
        fputc('\n', out);
        break;

    case AST_PRINT:
        indent(out, depth);
        fputs("print(", out);
        emit_expression(node->left, out);
        fputs(")\n", out);
        break;

    case AST_IF:
        indent(out, depth);
        fputs("if ", out);
        emit_expression(node->left, out);
        fputs(":\n", out);
        emit_suite(node->right, out, depth + 1);
        if (node->third) {
            indent(out, depth);
            fputs("else:\n", out);
            emit_suite(node->third, out, depth + 1);
        }
        break;

    case AST_WHILE:
        indent(out, depth);
        fputs("while ", out);
        emit_expression(node->left, out);
        fputs(":\n", out);
        emit_suite(node->right, out, depth + 1);
        break;

    default:
        break;
    }
}

void ast_generate_python(const AstNode *root, FILE *out)
{
    fputs("# Generated from MiniC\n", out);
    emit_suite(root, out, 0);
}

/* JSON output for the browser visualizer */

static void write_json_string(const char *text, FILE *out)
{
    fputc('"', out);
    for (const unsigned char *p = (const unsigned char *)text; *p; ++p) {
        switch (*p) {
        case '"':  fputs("\\\"", out); break;
        case '\\': fputs("\\\\", out); break;
        case '\n': fputs("\\n", out); break;
        case '\r': fputs("\\r", out); break;
        case '\t': fputs("\\t", out); break;
        default:
            if (*p < 0x20) {
                fprintf(out, "\\u%04x", *p);
            } else {
                fputc(*p, out);
            }
        }
    }
    fputc('"', out);
}

static void write_json_node(const AstNode *node, FILE *out)
{
    if (!node) {
        fputs("null", out);
        return;
    }

    fprintf(out, "{\"type\":\"%s\"", type_name(node->type));

    if (node->type == AST_NUMBER) {
        fprintf(out, ",\"value\":%ld", node->value);
    }
    if (node->name) {
        fputs(",\"name\":", out);
        write_json_string(node->name, out);
    }
    if (node->op[0]) {
        fputs(",\"op\":", out);
        write_json_string(node->op, out);
    }

    fputs(",\"children\":[", out);
    const AstNode *children[] = {
        node->left, node->right, node->third, node->next
    };
    int first = 1;
    for (size_t i = 0; i < sizeof(children) / sizeof(children[0]); ++i) {
        if (!children[i]) {
            continue;
        }
        if (!first) {
            fputc(',', out);
        }
        write_json_node(children[i], out);
        first = 0;
    }
    fputs("]}", out);
}

void ast_write_json(const AstNode *root, FILE *out)
{
    write_json_node(root, out);
    fputc('\n', out);
}