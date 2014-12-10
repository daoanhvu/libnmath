#ifndef _COMPOSITECRITERIA_H
#define _COMPOSITECRITERIA_H

#include "criteria.h"
#include "combinedcriteria.h"

namespace nmath {
	class CompositeCriteria: public Criteria {
		private:
			Criteria** list;
			int mLoggedSize;
			int mSize;

			int logicOp; // AND OR

			CompositeCriteria* and(SimpleCriteria& c);
			CompositeCriteria* and(CompositeCriteria& c);

			CompositeCriteria* or(SimpleCriteria& c);
			CompositeCriteria* or(CompositeCriteria& c);
			
		public:
			CompositeCriteria();
			~CompositeCriteria();
			void release();
			Criteria* clone();

			int logicOperator() const { return logicOp; }
			void setOperator(int op)	{ logicOp = op; }

			int size() const { return mSize; }
			int loggedSize() const { return mLoggedSize; }
			bool check(const double* values);
			Criteria* getInterval(const double *values, const char* var, int varCount);

			void add(Criteria* c);
			
			Criteria* operator [](int index) const;
			
			Criteria* operator |(Criteria &);
			Criteria* operator &(Criteria &);

			CompositeCriteria& operator |=(Criteria &);
			CompositeCriteria& operator &=(Criteria &);

			CompositeCriteria& operator =(CompositeCriteria &);
	};
}

#endif