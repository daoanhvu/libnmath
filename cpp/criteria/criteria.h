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

		/* And this criteria with pair of values */
		Criteria& and(const float *values);
		Criteria& operator &(const Criteria* c);
		CombinedCriteria& operator &(const CombinedCriteria& c);
		CompositeCriteria& operator &(const CompositeCriteria& c);
		
		CompositeCriteria& operator |(const Criteria& c);
		CompositeCriteria& operator |(const CombinedCriteria& c);
		CompositeCriteria& operator |(const CompositeCriteria& c);

		static Criteria& operator =(Criteria& c1, Criteria& c2);

		/* check if value does belong to this interval */
		int check(float value);
};


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