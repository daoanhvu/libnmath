#ifndef _COMBINEDCRITERIA_H
#define _COMBINEDCRITERIA_H

#include "criteria.h"

/*
	This class denote a combining criteria that consists of criterias for those hold variable that are different each other.
	Example:
		(0 < x < 1) AND (y>0): this case we got a combined criteria that consists of two criteria, one is (0 < x < 1) 
		and the another is (y>0)
*/

class CombinedCriteria {
	private:
		Criteria **list;
		int loggedSize;
		int size;
	public:
		CombinedCriteria();
		~CombinedCriteria();
		void release();

		int getSize();
		int getLoggedSize();

		/**
			Combine (AND) this criteria with each pair of value in bounds
		*/
		CombinedCriteria* getInterval(const float *bounds, int varCount);
		static void operator =(CombinedCriteria &, CombinedCriteria &);
		Criteria* operator [](int index);
};

inline int CombinedCriteria::getSize(){return size;}

#endif