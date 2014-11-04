#ifndef _COMBINEDCRITERIA_H
#define _COMBINEDCRITERIA_H

#include "criteria.h"

class CombinedCriteria {
	private:
		Criteria *list;
		int loggedSize;
		int size;
	public:
		CombinedCriteria();
		~CombinedCriteria();

		int getSize();
		int getLoggedSize();

		/**
			Combine (AND) this criteria with each pair of value in bounds
		*/
		CombinedCriteria* getInterval(const float *bounds, int varCount);
		static void operator =(CombinedCriteria &, CombinedCriteria &);
};

#endif