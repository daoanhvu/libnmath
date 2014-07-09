#ifndef __INTERVAL_H_
#define __INTERVAL_H_

#include "common.h"

typedef struct tagCriteria Criteria;
typedef struct tagCombinedCriteria CombinedCriteria;
typedef struct tagCompositeCriteria CompositeCriteria;
typedef struct tagFData FData;
typedef struct tagListFData ListFData;
typedef struct tagOutBuiltCriteria OutBuiltCriteria;

//Function poiters
typedef void (*FPGetIntervalF)(const void *, const DATA_TYPE_FP *, int, void *);
typedef int (*FPCheck)(const void *, DATA_TYPE_FP *, int);

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
	char objectType;
	FPCheck fcheck;
	FPGetIntervalF fgetInterval;
	
	/** GT_LT, GTE_LT, GT_LTE, GTE_LTE */
	short type;
	char variable, available;
	DATA_TYPE_FP leftVal;
	DATA_TYPE_FP rightVal;
	char isLeftInfinity, isRightInfinity;
};

//AND
/**
	In case our expression has multiple variables
	this type of interval express for a continuous space for the expression
	
	For example: 0<x<=5 AND y>3
 */
struct tagCombinedCriteria{
	char objectType;
	FPCheck fcheck;
	FPGetIntervalF fgetInterval;
	Criteria **list;
	unsigned short loggedSize;
	unsigned short size;
};

//OR
struct tagCompositeCriteria{
	char objectType;
	FPCheck fcheck;
	FPGetIntervalF fgetInterval;
	CombinedCriteria **list;
	unsigned short loggedSize;
	unsigned short size;
};

struct tagFData{
	DATA_TYPE_FP *data;
	unsigned short dataSize;
	unsigned short loggedSize;
	char dimension;
	
	int *rowInfo;
	unsigned short rowCount;
	unsigned short loggedRowCount;
};

struct tagListFData{
	FData **list;
	unsigned short loggedSize;
	unsigned short size;
};

struct tagOutBuiltCriteria{
	void *cr;
};

Criteria *newCriteria(int type, char var, DATA_TYPE_FP lval, DATA_TYPE_FP rval, 
										int leftInfinity, int rightInfinity);
CombinedCriteria *newCombinedInterval();
CompositeCriteria *newCompositeInterval();

int isInInterval(const void *interval, DATA_TYPE_FP *values, int varCount);
int isInCombinedInterval(const void *interval, DATA_TYPE_FP *values, int varCount);
int isInCompositeInterval(const void *interval, DATA_TYPE_FP *values, int varCount);

void getInterval(const void *interval, const DATA_TYPE_FP *values, int varCount, void *outInterval);

/**
	outListInterval [OUT] CombinedCriteria
*/
void getCombinedInterval(const void *interval, const DATA_TYPE_FP *values, int varCount, void *outListInterval);

/**
	outDomain [OUT] 
		it's a CompositeCriteria
*/
void getCompositeInterval(const void *interval, const DATA_TYPE_FP *values, int varCount, void *outDomain);

/**

*/
void buildCompositeCriteria(NMAST *ast, OutBuiltCriteria *result);

ListFData *getSpaces(Function *f, const DATA_TYPE_FP *bd, int bdlen, DATA_TYPE_FP epsilon);

#endif