#ifndef _NMATH_H_
#define _NMATH_H_

#include "nlablexer.h"
#include "nlabparser.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Parameter used for derivative_t & reduce_t * calc_t */
typedef struct tagDParam{
	NMAST *t;
	char variables[4];
	short error;
	DATA_TYPE_FP *values;
	DATA_TYPE_FP retv;
	NMAST *returnValue;
}DParam;

void initFunct(Function *);
void toString(const NMAST *t, char *str, int *curpos, int len);
void resetFunction(Function *f, const char *str, const char *vars, int varCount, short *error);
DATA_TYPE_FP calc(Function *f, DATA_TYPE_FP *values, int numOfValue, int *);
int reduce(Function *f, int *);
#ifdef WINDOWS
unsigned int __stdcall reduce_t(void *param);
unsigned int __stdcall calc_t(void *param);
unsigned int __stdcall derivative(void *p);
#else
void* reduce_t(void *);
void* calc_t(void *);
/*
	In case of multi-variable function, we need to tell which variable that we are 
	getting derivative of
*/
void* derivative(void *);
#endif

//void nodeToString(NMAST *t, char *str, int len, int *newlen);
void releaseFunct(Function *f);

#ifdef __cplusplus
}
#endif

#endif
