#ifndef __INTERVAL_H_
#define __INTERVAL_H_

#include "common.h"

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
void buildCompositeCriteria(const NMAST *domain, const char *vars, int varCount, OutBuiltCriteria *result);

ListFData* getSpaces(Function *f, const DATA_TYPE_FP *bd, int bdlen, DATA_TYPE_FP epsilon);

void buildCriteria(Function *f);

#endif