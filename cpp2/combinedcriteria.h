#ifndef _COMBINEDCRITERIA_H
#define _COMBINEDCRITERIA_H

#include "criteria.h"
#include "SimpleCriteria.h"

using namespace std;

/*
	This class denote a combining criteria that consists of criterias for those hold variable that are different each other.
	Example:
		(0 < x < 1) AND (y>0): this case we got a combined criteria that consists of two criteria, one is (0 < x < 1) 
		and the another is (y>0)
*/
namespace nmath {
	class CombinedCriteria: public Criteria {
		private:
			SimpleCriteria** list;
			int mLoggedSize;
			int mSize;
			
		public:
			CombinedCriteria();
			~CombinedCriteria();
			void release();

			int size() const { return mSize; }
			int loggedSize() const { return mLoggedSize; }
			bool check(const double* values);
			void moveListTo(CombinedCriteria& c);
			void add(SimpleCriteria* c);
			SimpleCriteria* remove(int index);

			Criteria* and(SimpleCriteria& c);
			Criteria* and(CombinedCriteria& c);
			Criteria* and(CompositeCriteria& c);

			Criteria* or(SimpleCriteria& c);
			Criteria* or(CombinedCriteria& c);
			CompositeCriteria* or(CompositeCriteria& c);

			/**
				Combine (AND) this criteria with each pair of value in bounds
			*/
			CombinedCriteria* getInterval(const float* bounds, int varCount);
			Criteria* operator &(Criteria& c);
			Criteria* operator |(Criteria& c);
			CombinedCriteria& operator =(const CombinedCriteria &);
			SimpleCriteria* operator [](int index) const;
			CombinedCriteria* clone();
			
	};
}

#endif