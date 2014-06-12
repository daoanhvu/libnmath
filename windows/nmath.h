#ifndef _NMATH_H_
#define _NMATH_H_

#include "nlablexer.h"
#include "nlabparser.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagDParam{
	NMAST *t;
	char x;
	int error;
	NMAST *returnValue;
}DParam;

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
unsigned int __stdcall reduce_t(void *);
unsigned int __stdcall calc_t(void *);
void toString(const NMAST *t, char *str, int *curpos, int len);
void releaseFunct(Function *);

/*
	In case of multi-variable function, we need to tell which variable that we are 
	getting derivative of
*/
unsigned int __stdcall derivative(void *);

#ifdef __cplusplus
}
#endif

#endif
