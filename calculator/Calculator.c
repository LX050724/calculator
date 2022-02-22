//
// Created by yao on 2022/2/18.
//

#include <string.h>
#include "Calculator.h"
#include "operator/BaseOperator.h"
#include "function/BaseFunction.h"
#include "core/CalculatorCore.h"
#include "command/CalculatorCommand.h"
#include <stdio.h>

#define CALC_MEM_CHECK(P) if((P) == NULL) { strcpy(self->err_str, "内存分配失败"); return 0;}

static char *remove_space(char *str);
static int isNumber(char c);
static int isLetter(char c);
static int isSymbol(char c);

static int Calculator_PreProcessing(Calculator_t *self, const char *express, Array *array);

int Calculator_init(Calculator_t *self) {
    memset(self, 0, sizeof(Calculator_t));

    Calculator_addEnv(self, "pi", "3.14159265358979323846");
    Calculator_addEnv(self, "e", "2.7182818284590452354");
    registerBaseOperator(self);
    registerBaseFunction(self);
    registerCommand(self);
    return 1;
}

void Calculator_delete(Calculator_t *self) {
    Array_delete(&self->operators);
    Array_delete(&self->functions);
    Array_delete(&self->env);
    Array_delete(&self->cmd);
}

int Calculator_registerOperator(Calculator_t *self, Operator_t *operator) {
    if (operator == NULL || !isSymbol(operator->sym)) return 0;
    return Array_push(&self->operators, operator, sizeof(Operator_t));
}

int Calculator_registerFunction(Calculator_t *self, Function_t *function) {
    return Array_push(&self->functions, function, sizeof(Function_t));
}

static char **Calculator_getEnvPtr(Calculator_t *self, const char *name) {
    for (size_t i = 0; i < self->env.len; i++) {
        EnvPair *pair = Array_get(&self->env, i);
        if (pair) {
            if (strcmp(name, pair->name) == 0)
                return &pair->val;
        }
    }
    return NULL;
}

const char *Calculator_findEnv(Calculator_t *self, const char *name) {
    char **p = Calculator_getEnvPtr(self, name);
    return p ? *p : NULL;
}

static void EnvPair_destructor(void *p) {
    EnvPair *pair = p;
    free(pair->name);
    free(pair->val);
}


int Calculator_addEnv(Calculator_t *self, const char *name, const char *val) {
    char **p = Calculator_getEnvPtr(self, name);
    if (p) {
        *p = realloc(*p, strlen(val) + 1);
        strcpy(*p, val);
        return 1;
    } else {
        EnvPair pair = {.parent.destructor = EnvPair_destructor};
        CALC_MEM_CHECK(pair.name = malloc(strlen(name) + 1))
        CALC_MEM_CHECK(pair.val = malloc(strlen(val) + 1))
        strcpy(pair.name, name);
        strcpy(pair.val, val);
        return Array_push(&self->env, &pair, sizeof(EnvPair));
    }
}

int Calculator_deleteEnv(Calculator_t *self, const char *name) {
    for (size_t i = 0; i < self->env.len; i++) {
        EnvPair *pair = Array_get(&self->env, i);
        if (pair) {
            if (strcmp(name, pair->name) == 0)
                return Array_remove(&self->env, i);
        }
    }
    return 0;
}

static int Calculator_findOperator(Calculator_t *self, char sym, Operator_t *operator) {
    for (size_t i = 0; i < self->operators.len; i++) {
        Operator_t *item = Array_get(&self->operators, i);
        if (item->sym == sym) {
            *operator = *item;
            return 1;
        }
    }
    return 0;
}

Function_t *Calculator_findFunction(Calculator_t *self, const char *name) {
    for (size_t i = 0; i < self->functions.len; i++) {
        Function_t *item = Array_get(&self->functions, i);
        if (strcmp(item->name, name) == 0) return item;
    }
    return NULL;
}

int Calculator_cmd(Calculator_t *self, const char *cmd) {
    int ret = 2;
    Array params = {0};
    char *str = malloc(strlen(cmd) + 1);
    CALC_MEM_CHECK(str)
    strcpy(str, cmd);

    char *p = strtok(str, " ");
    Command_t *command = Calculator_findCommand(self, p);
    if (command) {
        p = strtok(NULL, " ");
        while (p) {
            CommandParam_t param = {.param = p, .parent.destructor = NULL};
            Array_push(&params, &param, sizeof(CommandParam_t));
            p = strtok(NULL, " ");
        }
        if (!command->fun(self, &params))
            goto err_label;
    } else if (p) {
        sprintf(self->err_str, "错误 %s：未找到命令\n", p);
        goto err_label;
    }

    if (0) {
        err_label:
        ret = 0;
    }
    Array_delete(&params);
    free(str);
    return ret;
}

int Calculator_parse(Calculator_t *self, const char *express, double *ans) {
    if (express == NULL) {
        strcpy(self->err_str, "传入参数为NULL");
        return 0;
    }

    if (*express == '>')
        return Calculator_cmd(self, express + 1);

    Array array = {0};
    char val_buf[128] = {0};
    if (!Calculator_PreProcessing(self, express, &array)) {
        Array_delete(&array);
        return 0;
    }
    void **p = (void **) array.p;
    if (!Calculator_core(self, &p, ans)) {
        Array_delete(&array);
        return 0;
    }
    sprintf(val_buf, "%lf", *ans);
    Calculator_addEnv(self, "ans", val_buf);
    Array_delete(&array);
    return 1;
}

static int Calculator_PreProcessing(Calculator_t *self, const char *express, Array *array) {
    int ret = 1;
    char *str = malloc(strlen(express) + 1);
    CALC_MEM_CHECK(str)
    strcpy(str, express);

    remove_space(str);
    char *str_ptr = str;
    while (*str_ptr) {
        if (*str_ptr == '$') {
            // 环境变量或特定常量解析
            char name[128] = {0};
            int index = 0;
            str_ptr++;
            while (isLetter(*str_ptr))
                name[index++] = *str_ptr++;
            const char *val_str = Calculator_findEnv(self, name);
            if (val_str == NULL) {
                sprintf(self->err_str, "错误 未定义变量 %s\n", name);
                goto err_label;
            }
            Constant_t constantNode = CONSTANT_INIT;
            constantNode.val = strtod(val_str, NULL);
            Array_push(array, &constantNode, sizeof(Constant_t));
            continue;
        }
        if (*str_ptr <= '9' && *str_ptr >= '0') {
            // 常量解析
            Constant_t constantNode = CONSTANT_INIT;
            constantNode.val = strtod(str_ptr, &str_ptr);
            Array_push(array, &constantNode, sizeof(Constant_t));
            continue;
        } else if (isLetter(*str_ptr) || *str_ptr == '_') {
            // 函数解析
            char fun[128] = {0};
            int index = 0;
            while (isLetter(*str_ptr) || isNumber(*str_ptr) || *str_ptr == '_')
                fun[index++] = *str_ptr++;
            Function_t *function = Calculator_findFunction(self, fun);
            if (function) {
                Array_push(array, function, sizeof(Function_t));
            } else {
                sprintf(self->err_str, "错误 未定义函数 %s\n", fun);
                goto err_label;
            }
            continue;
        } else if (isSymbol(*str_ptr)) {
            Operator_t operator;
            if (Calculator_findOperator(self, *str_ptr, &operator)) {
                // 第一个元素就是符号或上一个元素也是符号时判断是否为负数
                ExpressNodeBase_t *last = Array_get(array, array->len - 1);
                if (last == NULL || last->type == TypeOperator) {
                    char *p = str_ptr;
                    Constant_t constantNode = CONSTANT_INIT;
                    constantNode.val = strtod(p, &p);
                    if (str_ptr == p) {
                        // 解析失败
                        sprintf(self->err_str, "表达式错误\n");
                        goto err_label;
                    } else {
                        // 解析成功作为常量放入数组，并移动指针
                        Array_push(array, &constantNode, sizeof(Constant_t));
                        str_ptr = p;
                        continue;
                    }
                } else Array_push(array, &operator, sizeof(Operator_t));
            } else {
                sprintf(self->err_str, "错误 未定义符号 %c\n", *str_ptr);
                goto err_label;
            }
        } else if (*str_ptr == '(') {
            ExpressNodeBase_t node = BRACKET_L_NODE_INIT;
            Array_push(array, &node, sizeof(ExpressNodeBase_t));
        } else if (*str_ptr == ')') {
            ExpressNodeBase_t node = BRACKET_R_NODE_INIT;
            Array_push(array, &node, sizeof(ExpressNodeBase_t));
        } else if (*str_ptr == ',') {
            ExpressNodeBase_t node = COMMA_NODE_INIT;
            Array_push(array, &node, sizeof(ExpressNodeBase_t));
        } else {
            sprintf(self->err_str, "错误 未知符号 %c\n", *str_ptr);
            goto err_label;
        }
        str_ptr++;
    }
    ExpressNodeBase_t end_node = END_NODE_INIT;
    Array_push(array, &end_node, sizeof(ExpressNodeBase_t));
    if (0) {
        err_label:
        ret = 0;
    }
    free(str);
    return ret;
}

static char *remove_space(char *str) {
    size_t read = 0, end = 0;
    while (str[read] != 0) {
        if (str[read] == ' ') read++;
        else str[end++] = str[read++];
    }
    str[end] = 0;
    return str;
}

static int isNumber(char c) {
    return (c <= '9' && c >= '0');
}

static int isLetter(char c) {
    return ((c <= 'Z' && c >= 'A') ||
            (c <= 'z' && c >= 'a'));
}

static int isSymbol(char c) {
    switch (c) {
        case '~':
        case '!':
        case '@':
        case '%':
        case '^':
        case '&':
        case '*':
        case '/':
        case '\\':
        case '-':
        case '+':
        case '#':
            return 1;
        default:
            return 0;
    }
}

int Calculator_registerCommand(Calculator_t *self, Command_t *command) {
    return Array_push(&self->cmd, command, sizeof(Command_t));
}

Command_t *Calculator_findCommand(Calculator_t *self, const char *cmd) {
    if (cmd == NULL) return NULL;
    for (size_t i = 0; i < self->cmd.len; i++) {
        Command_t *command = Array_get(&self->cmd, i);
        if (command) {
            if (strcmp(cmd, command->name) == 0)
                return command;
        }
    }
    return NULL;
}