#include <stdio.h>
#include <string.h>
#include "Calculator.h"

int main() {
    printf("输入exit退出\n");
    double ans = 0;
    Calculator_t calc;
    Calculator_init(&calc);
    Calculator_parse(&calc, ">help", &ans);

    while (1) {
        char *line = NULL;
        size_t size = 0;
        __ssize_t str_len;
        do printf("~ ");
        while ((str_len = getline(&line, &size, stdin)) <= 1);
        if (line[str_len - 1] == '\n') line[str_len - 1] = 0;
        if (strcmp(line, "exit") == 0) {
            free(line);
            break;
        }
        ans = 0;
        int ret = Calculator_parse(&calc, line, &ans);
        if (ret == 1) {
            printf("%lg\n", ans);
        } else if (ret == 0) {
            fprintf(stderr, "%s\n", calc.err_str);
        }
        free(line);
    }
    Calculator_delete(&calc);
    return 0;
}
