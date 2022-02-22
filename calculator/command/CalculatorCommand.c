//
// Created by yao on 2022/2/19.
//

#include "CalculatorCommand.h"
#include <stdio.h>
#include <string.h>

#define DEFINE_COMMAND(NAME, HELP)                              \
    static int calc_##NAME(Calculator_t *calc, Array *param);   \
    static Command_t command_##NAME = {                         \
        .parent.destructor = NULL,                              \
        .help = (HELP),                                         \
        .name = #NAME,                                          \
        .fun = calc_##NAME                                      \
    };                                                          \
    static int calc_##NAME(Calculator_t *calc, Array *param)


DEFINE_COMMAND(env, NULL) {
    for (size_t i = 0; i < calc->env.len; i++) {
        EnvPair *pair = Array_get(&calc->env, i);
        printf("%s=%s\n", pair->name, pair->val);
    }
    return 1;
}

DEFINE_COMMAND(set, NULL) {
    if (param->len != 2) {
        fprintf(stderr, "参数错误\n");
        return 0;
    }
    CommandParam_t *param0 = Array_get(param, 0);
    CommandParam_t *param1 = Array_get(param, 1);
    return Calculator_addEnv(calc, param0->param, param1->param);
}

DEFINE_COMMAND(unset, NULL) {
    if (param->len != 1) {
        fprintf(stderr, "参数错误\n");
        return 0;
    }
    CommandParam_t *param0 = Array_get(param, 0);
    if (!Calculator_deleteEnv(calc, param0->param))
        printf("没有参数 %s\n", param0->param);
    return 1;
}

#define HELP_HELP_STR "使用方法：\n"                                               \
"  计算表达式：直接输入表达式并回车\n"                                                 \
"  支持的运算符：+-*/加减乘除 %求模 ^指数"                                            \
"  命令行：第一个字符使用'>'并在后面输入命令，例：>help\n"                               \
"  函数：<function>([param]...) 例：sin($pi/6)\n"                                  \
"  环境变量：使用前缀'$'表示使用变量，可在中间使用，例：$pi\n"                             \
"      特殊变量：\n"                                                               \
"          pi：圆周率\n"                                                           \
"          e：自然常数\n"                                                          \
"          ans：上一次的结果\n"                                                     \
"  基本命令：\n"                                                                   \
"      list <cmd|fun>：列出所有命令或函数\n"                                         \
"      help [fun|cmd <function name>]：显示当前帮助或显示指定函数或命令的帮助\n"         \
"      env：查看当前存储的变量\n"                                                    \
"      set <name> <val>：设置变量\n"                                               \
"      unset <name>：删除变量\n"

DEFINE_COMMAND(help, HELP_HELP_STR) {
    CommandParam_t *param0 = Array_get(param, 0);
    CommandParam_t *param1 = Array_get(param, 1);
    if (param0) {
        if (param1 && strcmp(param0->param, "fun") == 0) {
            Function_t *function = Calculator_findFunction(calc, param1->param);
            if (function) {
                if (function->help) puts(function->help);
                else printf("函数%s没有帮助信息\n", param1->param);
            } else {
                fprintf(stderr, "错误 未找到函数%s\n", param1->param);
            }
        } else if (param1 && strcmp(param0->param, "cmd") == 0) {
            Command_t *command = Calculator_findCommand(calc, param1->param);
            if (command) {
                if (command->help) puts(command->help);
                else printf("函数%s没有帮助信息\n", param1->param);
            } else {
                fprintf(stderr, "错误 未找到函数%s\n", param1->param);
            }
        } else {
            fprintf(stderr, "错误 %s:参数错误，详见>help\n", param0->param);
        }

    } else {
        puts(HELP_HELP_STR);
    }
    return 1;
}

DEFINE_COMMAND(list, "list <cmd|fun>：列出所有命令或函数") {
    CommandParam_t *param0 = Array_get(param, 0);
    if (param0 && strcmp(param0->param, "cmd") == 0) {
        for (int i = 0; i < calc->cmd.len; i++) {
            Command_t *command = Array_get(&calc->cmd, i);
            puts(command->name);
        }
    } else if (param0 && strcmp(param0->param, "fun") == 0) {
        for (int i = 0; i < calc->functions.len; i++) {
            Function_t *function = Array_get(&calc->functions, i);
            puts(function->name);
        }
    } else {
        fprintf(stderr, "错误 %s:参数错误，详见>help cmd list\n", param0 ? param0->param : "");
        return 0;
    }
    return 1;
}

DEFINE_COMMAND(load, "load <path>：读取并执行文件，一行一个表达式") {
    CommandParam_t *param0 = Array_get(param, 0);
    FILE *file = fopen(param0->param, "r");
    if (file) {
        char *line = NULL;
        size_t size = 0;
        __ssize_t str_len;
        while ((str_len = getline(&line, &size, file)) > 0) {
            if (line[str_len - 1] == '\n') line[str_len - 1] = 0;
            double ans = 0;

            int ret = Calculator_parse(calc, line, &ans);
            if (ret == 1) {
                printf("%s=%lg\n", line, ans);
            } else if (ret == 0) {
                fprintf(stderr, "%s\n%s\n", line, calc->err_str);
            }

            free(line);
            line = NULL;
            size = 0;
        }
        if (line) free(line);
        fclose(file);
        return 1;
    } else {
        fprintf(stderr, "no such file %s\n", param0->param);
        return 0;
    }
}

int registerCommand(Calculator_t *calc) {
    Calculator_registerCommand(calc, &command_set);
    Calculator_registerCommand(calc, &command_unset);
    Calculator_registerCommand(calc, &command_env);
    Calculator_registerCommand(calc, &command_help);
    Calculator_registerCommand(calc, &command_list);
    Calculator_registerCommand(calc, &command_load);
    return 1;
}