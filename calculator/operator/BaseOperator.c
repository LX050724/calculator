//
// Created by yao on 2022/2/18.
//

#include "BaseOperator.h"
#include <math.h>
#include <stdio.h>

#define PARENT_INIT {.type = TypeOperator,.parent.destructor = NULL}

static int calc_add(Calculator_t *calc, double num1, double num2, double *ans) {
    *ans = num1 + num2;
    return 1;
}


int calc_mut(Calculator_t *calc, double num1, double num2, double *ans) {
    *ans = num1 * num2;
    return 1;
}

int calc_sub(Calculator_t *calc, double num1, double num2, double *ans) {
    *ans = num1 - num2;
    return 1;
}

int calc_div(Calculator_t *calc, double num1, double num2, double *ans) {
    *ans = num1 / num2;
    return 1;
}

int calc_mod(Calculator_t *calc, double num1, double num2, double *ans) {
    if ((int64_t) num1 != num1 || (int64_t) num2 != num2) {
        sprintf(calc->err_str, "错误 整数求模参数不为整数 %lg%%%lg", num1, num2);
        return 0;
    }
    *ans = (double) ((int64_t) num1 % (int64_t) num2);
    return 1;
}

int calc_pow(Calculator_t *calc, double num1, double num2, double *ans) {
    *ans = pow(num1, num2);
    return 1;
}

static Operator_t operator_add = {
        .parent = PARENT_INIT,
        .sym = '+',
        .priority = 1,
        .calc = calc_add
};

static Operator_t operator_sub = {
        .parent = PARENT_INIT,
        .sym = '-',
        .priority = 1,
        .calc = calc_sub
};

static Operator_t operator_mut = {
        .parent = PARENT_INIT,
        .sym = '*',
        .priority = 2,
        .calc = calc_mut
};

static Operator_t operator_div = {
        .parent = PARENT_INIT,
        .sym = '/',
        .priority = 2,
        .calc = calc_div
};

static Operator_t operator_mod = {
        .parent = PARENT_INIT,
        .sym = '%',
        .priority = 2,
        .calc = calc_mod
};

static Operator_t operator_pow = {
        .parent = PARENT_INIT,
        .sym = '^',
        .priority = 3,
        .calc = calc_pow
};

int registerBaseOperator(Calculator_t *calc) {
    Calculator_registerOperator(calc, &operator_add);
    Calculator_registerOperator(calc, &operator_sub);
    Calculator_registerOperator(calc, &operator_mut);
    Calculator_registerOperator(calc, &operator_div);
    Calculator_registerOperator(calc, &operator_mod);
    Calculator_registerOperator(calc, &operator_pow);
    return 1;
}
