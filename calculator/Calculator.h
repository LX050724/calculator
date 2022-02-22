//
// Created by yao on 2022/2/18.
//

#ifndef CACULATOR_CACUTALOR_H
#define CACULATOR_CACUTALOR_H

#include <stdint.h>
#include "utli/Stack.h"
#include "utli/Array.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    NodeBase parent;
    enum {
        TypeConstant,
        TypeOperator,
        TypeBracket_L,
        TypeBracket_R,
        TypeComma,
        TypeFunction,
        TypeEnd,
    } type;
} ExpressNodeBase_t;

#define BRACKET_L_NODE_INIT {.type = TypeBracket_L, .parent.destructor = NULL}
#define BRACKET_R_NODE_INIT {.type = TypeBracket_R, .parent.destructor = NULL}
#define COMMA_NODE_INIT {.type = TypeComma, .parent.destructor = NULL}
#define END_NODE_INIT {.type = TypeEnd, .parent.destructor = NULL}


typedef struct {
    Array operators;
    Array functions;
    Array env;
    Array cmd;
    char err_str[512];
} Calculator_t;

typedef struct {
    ExpressNodeBase_t parent;
    double val;
} Constant_t;

#define CONSTANT_INIT {.parent= {.type = TypeConstant, .parent.destructor = NULL}, .val = 0}

typedef struct {
    ExpressNodeBase_t parent;
    int priority;
    char sym;
    int (*calc)(Calculator_t *, double, double, double *);
} Operator_t;

typedef struct {
    ExpressNodeBase_t parent;
    char *name;
    char *help;
    int (*fun)(Calculator_t *, Array *, double *);
} Function_t;

typedef struct {
    NodeBase parent;
    char *help;
    char *name;
    int (*fun)(Calculator_t *, Array *);
} Command_t;

typedef struct {
    NodeBase parent;
    char *param;
} CommandParam_t;

typedef struct {
    NodeBase parent;
    char *name;
    char *val;
} EnvPair;

int Calculator_init(Calculator_t *self);
void Calculator_delete(Calculator_t *self);
int Calculator_registerOperator(Calculator_t *self, Operator_t *operator);
int Calculator_registerFunction(Calculator_t *self, Function_t *function);
int Calculator_addEnv(Calculator_t *self, const char *name, const char *val);
const char *Calculator_findEnv(Calculator_t *self, const char *name);
int Calculator_deleteEnv(Calculator_t *self, const char *name);
int Calculator_parse(Calculator_t *self, const char *express, double *ans);
int Calculator_registerCommand(Calculator_t *self, Command_t *command);
Function_t *Calculator_findFunction(Calculator_t *self, const char *name);
Command_t *Calculator_findCommand(Calculator_t *self, const char *cmd);

#ifdef __cplusplus
};
#endif

#endif //CACULATOR_CACUTALOR_H
