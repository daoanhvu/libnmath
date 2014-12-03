#ifndef __SIMPLECRITERIA_H_
#define __SIMPLECRITERIA_H_

//#include "common.h"
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
			bool rightInfinity;
			bool leftInfinity;

		public:
			SimpleCriteria();
			SimpleCriteria(int type, char var, double lval, double rval, 
											bool leftInfinity, bool rightInfinity);

			int getType()	{ return type; }
			char getVariable()	{ return variable; }
			double getLeftValue() { return leftVal; }
			double getRightValue() { return rightVal; }
			bool isRightInfinity() { return rightInfinity; }
			bool isLeftInfinity() { return leftInfinity; }

			bool isOverlapped(const SimpleCriteria& c);

			Criteria* and(SimpleCriteria& c);
			Criteria* and(CombinedCriteria& c);
			Criteria* and(CompositeCriteria& c);

			Criteria* or(SimpleCriteria& c);
			CompositeCriteria* or(CombinedCriteria& c);
			CompositeCriteria* or(CompositeCriteria& c);

			/* And this criteria with pair of values */
			SimpleCriteria* and(const double *values);
			Criteria* operator &(Criteria& c);
			Criteria* operator |(Criteria& c);

			SimpleCriteria* clone();
			SimpleCriteria& operator =(SimpleCriteria& c2);

			/* check if value does belong to this interval */
			bool check(const double* value);
	};
}

#endif