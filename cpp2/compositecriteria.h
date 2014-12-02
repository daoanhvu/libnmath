#ifndef _COMPOSITECRITERIA_H
#define _COMPOSITECRITERIA_H

#include "criteria.h"
#include "combinedcriteria.h"

namespace nmath {
	class CompositeCriteria: public Criteria {
		private:
			CombinedCriteria** list;
			int mLoggedSize;
			int mSize;

			CompositeCriteria* and(const SimpleCriteria& c);
			CompositeCriteria* and(const CombinedCriteria& c);
			CompositeCriteria* and(const CompositeCriteria& c);

			CompositeCriteria* or(const SimpleCriteria& c);
			CompositeCriteria* or(const CombinedCriteria& c);
			CompositeCriteria* or(const CompositeCriteria& c);
			
		public:
			CompositeCriteria();
			~CompositeCriteria();
			void release();

			int size() const { return mSize; }
			int loggedSize() const { return mLoggedSize; }
			int check(const float* values);

			void add(CombinedCriteria* c);
			
			CombinedCriteria* operator [](int index);
			
			Criteria& operator |(Criteria &);
			Criteria& operator &(Criteria &);

			CompositeCriteria& operator |=(CombinedCriteria &);
			CompositeCriteria& operator &=(Criteria &);
			CompositeCriteria& operator &=(CombinedCriteria &);

			CompositeCriteria& operator =(CompositeCriteria &);
	};
}

#endif