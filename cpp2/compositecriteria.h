#ifndef _COMPOSITECRITERIA_H
#define _COMPOSITECRITERIA_H

#include "criteria.h"
#include "SimpleCriteria.h"

namespace nmath {
	class CompositeCriteria: public Criteria {
		private:
			Criteria** list;
			int mLoggedSize;
			int mSize;

			int logicOp; // AND OR

			Criteria* andSimpleSelf(SimpleCriteria& c);
			Criteria* andCompositeSelf(CompositeCriteria& c);
			
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

			void add(Criteria* c);

			CompositeCriteria& normalize(const char* vars, int varcount);
			
			bool containsVar(char var);

			istream& operator >>(istream& is);
			ostream& operator <<(ostream& os);

			Criteria* andSelf(Criteria& c);
			CompositeCriteria* and(SimpleCriteria& c);
			CompositeCriteria* and(CompositeCriteria& c);
			CompositeCriteria* or(SimpleCriteria& c);
			CompositeCriteria* or(CompositeCriteria& c);
			
			Criteria* operator [](int index) const;
			Criteria* get(int index) const { return list[index]; }

			Criteria* operator |(Criteria &);
			Criteria* operator &(Criteria &);
			CompositeCriteria& operator |=(Criteria &);
			CompositeCriteria& operator &=(Criteria &);
			CompositeCriteria& operator =(CompositeCriteria &);

			Criteria* getInterval(const double *values, const char* var, int varCount);
			Criteria* getIntervalF(const float *values, const char* var, int varCount);
	};
}

#endif