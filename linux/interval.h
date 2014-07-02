#ifndef __INTERVAL_H_
#define __INTERVAL_H_

#include "common.h"

//Function poiters
typedef void (*FPInterval)(void *, double *, int, double **, int *);
typedef int (*FPCheck)(void *, double *, int);

typedef struct tagInterval Interval;
typedef struct tagCombinedInterval CombinedInterval;
typedef struct tagCompositeInterval CompositeInterval;

/** This is a single continuous interval for one variable */
struct tagInterval{
	/** GT_LT, GTE_LT, GT_LTE, GTE_LTE */
	int type;
	char variable;
	double leftVal;
	double rightVal;
	int isLeftInfinity, isRightInfinity;
	FPCheck fcheck;
	FPInterval fgetInterval;
};

//AND
/**
	In case our expression has multiple variables
	this type of interval express for a continuous space for the expression
	
	For example: 0<x<=5 AND y>3
 */
struct tagCombinedInterval{
	Interval **list;
	int loggedSize;
	int size;
	FPCheck fcheck;
	FPInterval fgetInterval;
};

//OR
struct tagCompositeInterval{
	void **list;
	int loggedSize;
	int size;
	FPCheck fcheck;
	FPInterval fgetInterval;
};

struct tagExpDomain{
	void *domain;
	int type;
};

Interval *newInterval(char var, double lval, double rval);
CombinedInterval *newCombinedInterval();
CompositeInterval *newCompositeInterval();

int isInInterval(void *interval, double *values, int varCount);
int isInCombinedInterval(void *interval, double *values, int varCount);
int isInCompositeInterval(void *interval, double *values, int varCount);

void getInterval(void *interval, double *values, int varCount, double **outInterval, int *outlen);

/**
	outInterval
		This output parameter, it's a matrix N row and 2 columns which each row is for each continuous interval of a single variable
*/
void getCombinedInterval(void *interval, double *values, int varCount, double **outInterval, int *outlen);

/**
	outInterval
		This output parameter, it's a matrix N row and M columns which each row is for each continuous space for the expression
		It means that each row will hold a combined-interval for n-tule variables
*/
void getCompositeInterval(void *interval, double *values, int varCount, double **outInterval, int *outlen);

#endif