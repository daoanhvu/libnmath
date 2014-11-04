#ifndef __INTERVAL_H_
#define __INTERVAL_H_

#include "common.h"

class Criteria {
	private:
		/** GT_LT, GTE_LT, GT_LTE, GTE_LTE */
		int type;
		char variable;
		double leftVal;
		double rightVal;
		
		/*
			bit 0: right infinity
			bit 1: left infinity
			bit 2: available
		*/
		char flag;

	public:
		Criteria();
		Criteria::Criteria(int type, char var, float lval, float rval, 
										int leftInfinity, int rightInfinity);
		~Criteria();

		/* And this criteria with pair of values */
		Criteria* and(const float *values, int varCount);
		CompositeCriteria* or(const Criteria *c);
		int isInInterval(float value);
};

Criteria *newCriteria(int type, char var, float lval, float rval, 
										int leftInfinity, int rightInfinity);
CombinedCriteria *newCombinedInterval();
CompositeCriteria *newCompositeInterval();

int isInInterval(const void *interval, float *values, int varCount);
int isInCombinedInterval(const void *interval, float *values, int varCount);
int isInCompositeInterval(const void *interval, float *values, int varCount);

void getInterval(const void *interval, const float *values, int varCount, void *outInterval);

/**
	outListInterval [OUT] CombinedCriteria
*/
void getCombinedInterval(const void *interval, const float *values, int varCount, void *outListInterval);

/**
	outDomain [OUT] 
		it's a CompositeCriteria
*/
void getCompositeInterval(const void *interval, const float *values, int varCount, void *outDomain);

/**

*/
void buildCompositeCriteria(const NMAST *domain, const char *vars, int varCount, OutBuiltCriteria *result);

ListFData* getSpaces(Function *f, const float *bd, int bdlen, float epsilon);

void buildCriteria(Function *f);

#endif