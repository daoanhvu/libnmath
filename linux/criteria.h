#ifndef __INTERVAL_H_
#define __INTERVAL_H_

#include "common.h"

typedef struct tagInterval Interval;
typedef struct tagCriteria Criteria;
typedef struct tagCombinedCriteria CombinedCriteria;
typedef struct tagCompositeCriteria CompositeCriteria;
typedef struct tagListInterval ListInterval;
typedef struct tagDomain Domain;

//Function poiters
typedef void (*FPGetIntervalF)(void *, DATA_TYPE_FP *, int, Interval *);
typedef void (*FPGetListIntervalF)(void *, DATA_TYPE_FP *, int, ListInterval *, int *);
typedef void (*FPGetDomainF)(void *, DATA_TYPE_FP *, int, Domain *, int *);
typedef int (*FPCheck)(void *, DATA_TYPE_FP *, int);

/** This is a single continuous interval for one variable */
struct tagCriteria{
	FPCheck fcheck;
	FPGetIntervalF fgetInterval;
	/** GT_LT, GTE_LT, GT_LTE, GTE_LTE */
	int type;
	char variable;
	DATA_TYPE_FP leftVal;
	DATA_TYPE_FP rightVal;
	int isLeftInfinity, isRightInfinity;
};

//AND
/**
	In case our expression has multiple variables
	this type of interval express for a continuous space for the expression
	
	For example: 0<x<=5 AND y>3
 */
struct tagCombinedCriteria{
	FPCheck fcheck;
	FPGetListIntervalF fgetInterval;
	Criteria **list;
	int loggedSize;
	int size;
};

//OR
struct tagCompositeCriteria{
	FPCheck fcheck;
	FPGetDomainF fgetInterval;
	CombinedCriteria **list;
	int loggedSize;
	int size;
};

struct tagInterval{
	int available;
	DATA_TYPE_FP leftVal;
	DATA_TYPE_FP rightVal;
};

struct tagListInterval{
	Interval **list;
	int loggedSize;
	int size;
};

struct tagDomain{
	ListInterval **list;
	int loggedSize;
	int size;
};

Criteria *newCriteria(int type, char var, DATA_TYPE_FP lval, DATA_TYPE_FP rval, 
										int leftInfinity, int rightInfinity);
CombinedCriteria *newCombinedInterval();
CompositeCriteria *newCompositeInterval();

int isInInterval(void *interval, DATA_TYPE_FP *values, int varCount);
int isInCombinedInterval(void *interval, DATA_TYPE_FP *values, int varCount);
int isInCompositeInterval(void *interval, DATA_TYPE_FP *values, int varCount);

void getInterval(void *interval, DATA_TYPE_FP *values, int varCount, Interval *outInterval);

/**
	outInterval
		This output parameter, it's a matrix N row and 2 columns which each row is for each continuous interval of a single variable
*/
void getCombinedInterval(void *interval, DATA_TYPE_FP *values, int varCount, ListInterval *outListInterval, int *outlen);

/**
	outInterval
		This output parameter, it's a matrix N row and M columns which each row is for each continuous space for the expression
		It means that each row will hold a combined-interval for n-tule variables
*/
void getCompositeInterval(void *interval, DATA_TYPE_FP *values, int varCount, Domain *outDomain, int *outlen);

#endif