#ifndef __SIMPLECRITERIA_H_
#define __INTERVAL_H_

#include "common.h"
#include "criteria.h"

namespace nmath {

	class CombinedCriteria;
	class CompositeCriteria;

	class SimpleCriteria: public Criteria {
		private:
			/** GT_LT, GTE_LT, GT_LTE, GTE_LTE */
			int type;
			char variable;
			double leftVal;
			double rightVal;
			char rightInfinity;
			char leftInfinity;

			CombinedCriteria* and(const SimpleCriteria& c);
			CombinedCriteria* and(const CombinedCriteria& c);
			CompositeCriteria* and(const CompositeCriteria& c);

			CombinedCriteria* or(const SimpleCriteria& c);
			CombinedCriteria* or(const CombinedCriteria& c);
			CompositeCriteria* or(const CompositeCriteria& c);

		public:
			SimpleCriteria();
			SimpleCriteria(int type, char var, float lval, float rval, 
											int leftInfinity, int rightInfinity);

			/* And this criteria with pair of values */
			Criteria* and(const float *values);
			Criteria* operator &(const Criteria &c);
			Criteria* operator |(const Criteria &c);

			SimpleCriteria* clone();
			static SimpleCriteria& operator =(SimpleCriteria& c1, SimpleCriteria& c2);

			/* check if value does belong to this interval */
			int check(float value);
	};
}

#endif