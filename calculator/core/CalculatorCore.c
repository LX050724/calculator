//
// Created by yao on 2022/2/19.
//

#include <stdio.h>
#include "CalculatorCore.h"


static int Calculator_functionParse(Calculator_t *self, void ***node_pp, double *ret);

int Calculator_core(Calculator_t *self, void ***node_pp, double *ans) {
    int ret = 1;
    Stack dataStack = {0}, operatorStack = {0};

    ExpressNodeBase_t *node_p;
    while ((node_p = **node_pp)->type != TypeEnd) {
        switch (node_p->type) {
            case TypeConstant: {
                Stack_push(&dataStack, node_p, sizeof(Constant_t));
                break;
            }
            case TypeOperator: {
                Operator_t *operator_now = (Operator_t *) node_p;
                Operator_t *operator_top = (Operator_t *) Stack_top(&operatorStack);
                if (operator_top && operator_now->priority <= operator_top->priority) {
                    Constant_t constant = CONSTANT_INIT;
                    Constant_t *num2 = (Constant_t *) Stack_pop(&dataStack);
                    Constant_t *num1 = (Constant_t *) Stack_pop(&dataStack);
                    if (!operator_top->calc(self, num1 ? num1->val : 0, num2 ? num2->val : 0, &constant.val)) {
                        Stack_releaseNode(num2);
                        Stack_releaseNode(num1);
                        goto err_label;
                    }
                    Stack_releaseNode(num2);
                    Stack_releaseNode(num1);
                    Stack_push(&dataStack, &constant, sizeof(Constant_t));
                    Stack_releaseNode(Stack_pop(&operatorStack));
                }
                Stack_push(&operatorStack, operator_now, sizeof(Operator_t));
                break;
            }
            case TypeBracket_L: {
                Constant_t constant = CONSTANT_INIT;
                (*node_pp)++;
                if (!Calculator_core(self, node_pp, &constant.val))
                    goto err_label;
                Stack_push(&dataStack, &constant, sizeof(Constant_t));
                continue;
            }
            case TypeFunction: {
                Constant_t constant = CONSTANT_INIT;
                if (!Calculator_functionParse(self, node_pp, &constant.val))
                    goto err_label;
                Stack_push(&dataStack, &constant, sizeof(Constant_t));
                break;
            }
            case TypeEnd:
            case TypeComma:
            case TypeBracket_R:
                goto ret_label;
        }
        (*node_pp)++;
    }

    ret_label:
    while (operatorStack.len) {
        Operator_t *operator_top = (Operator_t *) Stack_pop(&operatorStack);
        Constant_t constant = CONSTANT_INIT;
        Constant_t *num2 = (Constant_t *) Stack_pop(&dataStack);
        Constant_t *num1 = (Constant_t *) Stack_pop(&dataStack);
        if (!operator_top->calc(self, num1 ? num1->val : 0, num2 ? num2->val : 0, &constant.val)) {
            Stack_releaseNode(num2);
            Stack_releaseNode(num1);
            Stack_releaseNode(operator_top);
            goto err_label;
        }
        Stack_releaseNode(num2);
        Stack_releaseNode(num1);
        Stack_releaseNode(operator_top);
        Stack_push(&dataStack, &constant, sizeof(Constant_t));
    }

    if (ans && dataStack.len) {
        Constant_t *constant = Stack_top(&dataStack);
        *ans = constant->val;
    }
    if (0) {
        err_label:
        ret = 0;
    }

    Stack_delete(&dataStack);
    Stack_delete(&operatorStack);
    return ret;
}

static int Calculator_functionParse(Calculator_t *self, void ***node_pp, double *ret) {
    Array param = {0};
    Function_t *function = (Function_t *) **node_pp;
    (*node_pp)++;
    ExpressNodeBase_t *node_p = **node_pp;
    if (node_p->type != TypeBracket_L) {
        sprintf(self->err_str, "错误 函数后必须接括号 %s", function->name);
        return 0;
    }
    (*node_pp)++;
    node_p = **node_pp;

    while (node_p->type != TypeBracket_R) {
        if (node_p->type == TypeEnd) {
            sprintf(self->err_str, "错误 意外的结束 %s", function->name);
            return 0;
        }
        if (node_p->type == TypeComma)
            (*node_pp)++;
        Constant_t constant = CONSTANT_INIT;
        Calculator_core(self, node_pp, &constant.val);
        Array_push(&param, &constant, sizeof(Constant_t));
        node_p = **node_pp;
    }

    int r = function->fun(self, &param, ret);
    Array_delete(&param);
    return r;
}