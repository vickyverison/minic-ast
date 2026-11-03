#include "ast.h"

#include <stdio.h>
#include <string.h>

extern FILE *yyin;
extern int yyparse(void);
extern AstNode *ast_root;
extern int parse_failed;

static void usage(const char *program)
{
    fprintf(stderr,
        "Usage:\n"
        "  %s run    source.mc\n"
        "  %s python source.mc\n"
        "  %s ast    source.mc\n"
        "  %s json   source.mc output.json\n",
        program, program, program, program);
}

int main(int argc, char **argv)
{
    if (argc < 3 || argc > 4) {
        usage(argv[0]);
        return 2;
    }

    const char *mode = argv[1];
    const char *source_path = argv[2];

    if (strcmp(mode, "json") == 0 && argc != 4) {
        usage(argv[0]);
        return 2;
    }
    if (strcmp(mode, "json") != 0 && argc != 3) {
        usage(argv[0]);
        return 2;
    }

    FILE *source = fopen(source_path, "r");
    if (!source) {
        perror(source_path);
        return 1;
    }

    yyin = source;
    int parse_result = yyparse();
    fclose(source);

    if (parse_result != 0 || parse_failed || !ast_root) {
        ast_free(ast_root);
        return 1;
    }

    int result = 0;

    if (strcmp(mode, "run") == 0) {
        result = ast_execute(ast_root);
    } else if (strcmp(mode, "python") == 0) {
        ast_generate_python(ast_root, stdout);
    } else if (strcmp(mode, "ast") == 0) {
        ast_print_tree(ast_root, 0);
    } else if (strcmp(mode, "json") == 0) {
        FILE *output = fopen(argv[3], "w");
        if (!output) {
            perror(argv[3]);
            ast_free(ast_root);
            return 1;
        }
        ast_write_json(ast_root, output);
        fclose(output);
    } else {
        usage(argv[0]);
        result = 2;
    }

    ast_free(ast_root);
    return result;
}