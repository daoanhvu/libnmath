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
	int error;
	NMAST *returnValue;
}DParam;

/* Reduce param */
typedef struct tagRParam{
	NMAST *t;
	char *variables;
	double *values;
	double retv;
	int error;
}RParam;

void initFunct(Function *);

void resetFunction(Function *f, const char *str, const char *vars, int varCount, int *error);
double calc(Function *f, double *values, int numOfValue, int *);
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
