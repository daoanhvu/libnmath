#ifndef _COMPOSITECRITERIA_H
#define _COMPOSITECRITERIA_H

#include <vector>
#include "criteria.h"
#include "combinedcriteria.h"

class CompositeCriteria: public Criteria {
	private:
		std::vector<CombinedCriteria&>  v;
		
	public:
		CompositeCriteria();
		~CompositeCriteria();
		void release();

		int size();
		int check(const float* values);
		
		CombinedCriteria& operator [](int index);
		
		Criteria& operator |(Criteria &);
		Criteria& operator &(Criteria &);

		CompositeCriteria& operator |=(CombinedCriteria &);
		CompositeCriteria& operator &=(Criteria &);
		CompositeCriteria& operator &=(CombinedCriteria &);

		static void operator =(CompositeCriteria &, CompositeCriteria &);
};

inline int CompositeCriteria::getSize() { return v.size(); }

#endif