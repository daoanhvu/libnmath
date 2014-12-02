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

			Criteria* and(SimpleCriteria& c);
			Criteria* and(CombinedCriteria& c);
			Criteria* and(CompositeCriteria& c);

			Criteria* or(SimpleCriteria& c);
			CombinedCriteria* or(CombinedCriteria& c);
			CompositeCriteria* or(CompositeCriteria& c);

		public:
			SimpleCriteria();
			SimpleCriteria(int type, char var, float lval, float rval, 
											char leftInfinity, char rightInfinity);

			int getType()	{ return type; }
			char getVariable()	{ return variable; }
			double getLeftValue() { return leftVal; }
			double getRightValue() { return rightVal; }
			bool isRightInfinity() { return rightInfinity; }
			bool isLeftInfinity() { return leftInfinity; }

			bool isOverlapped(const SimpleCriteria& c);

			/* And this criteria with pair of values */
			SimpleCriteria* and(const float *values);
			Criteria* operator &(Criteria& c);
			Criteria* operator |(Criteria& c);

			SimpleCriteria* clone();
			SimpleCriteria& operator =(SimpleCriteria& c2);

			/* check if value does belong to this interval */
			int check(float value);
	};
}

#endif