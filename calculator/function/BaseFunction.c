//
// Created by yao on 2022/2/18.
//

#include "BaseFunction.h"
#include <math.h>
#include <stdio.h>

#define DEFINE_FUNCTION(NAME, HELP)                                         \
    static int calc_##NAME(Calculator_t *calc, Array *param, double *ans);  \
    static Function_t function_##NAME = {                                   \
        .parent = {.type = TypeFunction,.parent.destructor = NULL},         \
        .help = (HELP),                                                     \
        .name = #NAME,                                                      \
        .fun = calc_##NAME                                                  \
    };                                                                      \
    static int calc_##NAME(Calculator_t *calc, Array *param, double *ans)

DEFINE_FUNCTION(sin, NULL) {
    if (param->len != 1) return 0;
    double num1 = ((Constant_t *) Array_get(param, 0))->val;
    *ans = sin(num1);
    return 1;
}

DEFINE_FUNCTION(cos, NULL) {
    if (param->len != 1) return 0;
    double num1 = ((Constant_t *) Array_get(param, 0))->val;
    *ans = cos(num1);
    return 1;
}

DEFINE_FUNCTION(tan, NULL) {
    if (param->len != 1) return 0;
    double num1 = ((Constant_t *) Array_get(param, 0))->val;
    *ans = tan(num1);
    return 1;
}

DEFINE_FUNCTION(asin, NULL) {
    if (param->len != 1) return 0;
    double num1 = ((Constant_t *) Array_get(param, 0))->val;
    *ans = asin(num1);
    return 1;
}

DEFINE_FUNCTION(acos, NULL) {
    if (param->len != 1) return 0;
    double num1 = ((Constant_t *) Array_get(param, 0))->val;
    *ans = acos(num1);
    return 1;
}

DEFINE_FUNCTION(atan, NULL) {
    if (param->len != 1) return 0;
    double num1 = ((Constant_t *) Array_get(param, 0))->val;
    *ans = atan(num1);
    return 1;
}

DEFINE_FUNCTION(ln, NULL) {
    if (param->len != 1) return 0;
    double num1 = ((Constant_t *) Array_get(param, 0))->val;
    *ans = log(num1);
    return 1;
}

DEFINE_FUNCTION(log10, NULL) {
    if (param->len != 1) return 0;
    double num1 = ((Constant_t *) Array_get(param, 0))->val;
    *ans = log10(num1);
    return 1;
}

DEFINE_FUNCTION(log2, NULL) {
    if (param->len != 1) return 0;
    double num1 = ((Constant_t *) Array_get(param, 0))->val;
    *ans = log2(num1);
    return 1;
}

DEFINE_FUNCTION(log, "log(<底数>, <真数>)：自定义底数和真数的对数运算") {
    if (param->len != 2) {
        sprintf(calc->err_str, "log参数错误");
        return 0;
    }
    double num1 = ((Constant_t *) Array_get(param, 0))->val;
    double num2 = ((Constant_t *) Array_get(param, 1))->val;
    *ans = log(num2) / log(num1);
    return 1;
}

int registerBaseFunction(Calculator_t *calc) {
    Calculator_registerFunction(calc, &function_sin);
    Calculator_registerFunction(calc, &function_cos);
    Calculator_registerFunction(calc, &function_tan);
    Calculator_registerFunction(calc, &function_asin);
    Calculator_registerFunction(calc, &function_acos);
    Calculator_registerFunction(calc, &function_atan);
    Calculator_registerFunction(calc, &function_ln);
    Calculator_registerFunction(calc, &function_log10);
    Calculator_registerFunction(calc, &function_log2);
    Calculator_registerFunction(calc, &function_log);
    return 1;
}
