#ifndef _NMATH_H_
#define _NMATH_H_

#include "nlablexer.h"
#include "nlabparser.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Derivative param */
typedef struct tagDParam{
	NMAST *t;
	char x;
	short error;
	NMAST *returnValue;
}DParam;

/* Reduce param */
typedef struct tagRParam{
	NMAST *t;
	char *variables;
	DATA_TYPE_FP *values;
	DATA_TYPE_FP retv;
	short error;
}RParam;

void initFunct(Function *);
void toString(const NMAST *t, char *str, int *curpos, int len);
void resetFunction(Function *f, const char *str, const char *vars, int varCount, short *error);
DATA_TYPE_FP calc(Function *f, DATA_TYPE_FP *values, int numOfValue, int *);
int reduce(Function *f, int *);
void* reduce_t(void *);
void* calc_t(void *);
void nodeToString(NMAST *t, char *str, int len, int *newlen);
void releaseFunct(Function *);

/*
	In case of multi-variable function, we need to tell which variable that we are 
	getting derivative of
*/
void* derivative(void *);

#ifdef __cplusplus
}
#endif

#endif
