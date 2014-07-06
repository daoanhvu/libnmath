#ifndef __INTERVAL_H_
#define __INTERVAL_H_

#include "common.h"

typedef struct tagCriteria Criteria;
typedef struct tagCombinedCriteria CombinedCriteria;
typedef struct tagCompositeCriteria CompositeCriteria;
typedef struct tagFData FData;
typedef struct tagListFData ListFData;

//Function poiters
typedef void (*FPGetIntervalF)(void *, DATA_TYPE_FP *, int, void *);
typedef int (*FPCheck)(void *, DATA_TYPE_FP *, int);

#define SIMPLE_CRITERIA 		0
#define COMBINED_CRITERIA 		1
#define COMPOSITE_CRITERIA 	2

/**
	This is a single continuous interval for one variable
	Example:
		variable = 'x' and type=GT_LT we read it out like this:
		x is greater than leftVal and x is less than rightValue
*/
struct tagCriteria{
	int objectType;
	FPCheck fcheck;
	FPGetIntervalF fgetInterval;
	
	/** GT_LT, GTE_LT, GT_LTE, GTE_LTE */
	int type, available;
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
	int objectType;
	FPCheck fcheck;
	FPGetIntervalF fgetInterval;
	Criteria **list;
	int loggedSize;
	int size;
};

//OR
struct tagCompositeCriteria{
	int objectType;
	FPCheck fcheck;
	FPGetIntervalF fgetInterval;
	CombinedCriteria **list;
	int loggedSize;
	int size;
};

struct tagFData{
	DATA_TYPE_FP *data;
	int dataSize;
	int loggedSize;
	
	int *rowInfo;
	int rowCount;
	int loggedRowCount;
};

struct tagListFData{
	FData **list;
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

void getInterval(void *interval, DATA_TYPE_FP *values, int varCount, void *outInterval);

/**
	outListInterval [OUT] CombinedCriteria
*/
void getCombinedInterval(void *interval, DATA_TYPE_FP *values, int varCount, void *outListInterval);

/**
	outDomain [OUT] 
		it's a CompositeCriteria
*/
void getCompositeInterval(void *interval, DATA_TYPE_FP *values, int varCount, void *outDomain);

/**

*/
void buildCompositeCriteria(NMAST *ast, void **outCriteria);

#endif