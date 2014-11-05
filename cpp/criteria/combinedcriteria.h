#ifndef _COMBINEDCRITERIA_H
#define _COMBINEDCRITERIA_H

#include "criteria.h"

using namespace std;

/*
	This class denote a combining criteria that consists of criterias for those hold variable that are different each other.
	Example:
		(0 < x < 1) AND (y>0): this case we got a combined criteria that consists of two criteria, one is (0 < x < 1) 
		and the another is (y>0)
*/

class CombinedCriteria {
	private:
		std::vector<Criteria&> v;
		
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
		
		CombinedCriteria& operator &(const Criteria& c);
		CombinedCriteria& operator &(const CombinedCriteria& c);
		CompositeCriteria& operator &(const CompositeCriteria& c);
		
		CombinedCriteria& operator |(const Criteria& c);
		CombinedCriteria& operator |(const CombinedCriteria& c);
		CompositeCriteria& operator |(const CompositeCriteria& c);
		
		CombinedCriteria& operator &=(const Criteria& c);
		CombinedCriteria& operator |=(const Criteria& c);
		
		static void operator =(CombinedCriteria &, const CombinedCriteria &);
		Criteria& operator [](int index);
		
};

inline int CombinedCriteria::size(){return v.size();}

#endif