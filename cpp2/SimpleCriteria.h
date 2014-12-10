#ifndef __SIMPLECRITERIA_H_
#define __SIMPLECRITERIA_H_

//#include "common.h"
#include "criteria.h"

namespace nmath {

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
			void setType(int t) { type = t; }

			char getVariable()	{ return variable; }
			void setVariable(char var) { variable = var; }

			double getLeftValue() { return leftVal; }
			void setLeftValue(double v) { leftVal = v; }
			double getRightValue() { return rightVal; }
			void setRightValue(double v) { rightVal = v; }
			bool isRightInfinity() { return rightInfinity; }
			void setRightInfinity(bool rInf) { rightInfinity = rInf; }
			bool isLeftInfinity() { return leftInfinity; }
			void setLeftInfinity(bool lInf) { leftInfinity = lInf; }

			bool isOverlapped(const SimpleCriteria& c);
			
			Criteria* getInterval(const double *values, const char* var, int varCount);

			Criteria* and(SimpleCriteria& c);
			Criteria* and(CompositeCriteria& c);

			Criteria* or(SimpleCriteria& c);
			CompositeCriteria* or(CompositeCriteria& c);

			/* And this criteria with pair of values */
			SimpleCriteria* and(const double *values);
			Criteria* operator &(Criteria& c);
			Criteria* operator |(Criteria& c);

			Criteria* clone();
			SimpleCriteria& operator =(SimpleCriteria& c2);

			/* check if value does belong to this interval */
			bool check(const double* value);
	};
}

#endif