#ifndef _COMBINEDCRITERIA_H
#define _COMBINEDCRITERIA_H

#include <vector>
#include "criteria.h"

using namespace std;

/*
	This class denote a combining criteria that consists of criterias for those hold variable that are different each other.
	Example:
		(0 < x < 1) AND (y>0): this case we got a combined criteria that consists of two criteria, one is (0 < x < 1) 
		and the another is (y>0)
*/
namespace nmath {
	class CombinedCriteria {
		private:
			std::vector<Criteria&> v;

			CombinedCriteria* and(const SimpleCriteria& c);
			CombinedCriteria* and(const CombinedCriteria& c);
			CompositeCriteria* and(const CompositeCriteria& c);

			CombinedCriteria* or(const SimpleCriteria& c);
			CombinedCriteria* or(const CombinedCriteria& c);
			CompositeCriteria* or(const CompositeCriteria& c);
			
		public:
			CombinedCriteria();
			~CombinedCriteria();
			void release();

			int size();
			int check(const float* values);
			void moveListTo(CombinedCriteria& c);
			void add(Criteria& c);

			/**
				Combine (AND) this criteria with each pair of value in bounds
			*/
			CombinedCriteria& getInterval(const float& bounds, int varCount);
			
			Criteria* operator &(const Criteria& c);
			Criteria* operator |(const Criteria& c);
			
			static void operator =(CombinedCriteria &, const CombinedCriteria &);
			Criteria& operator [](int index);
			
	};
	inline int CombinedCriteria::size() { return v.size(); }
}

#endif