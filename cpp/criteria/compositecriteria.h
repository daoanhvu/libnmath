#ifndef _COMPOSITECRITERIA_H
#define _COMPOSITECRITERIA_H

#include "combinedcriteria.h"

using namespace std;

class CompositeCriteria {
	private:
		std::vector<CombinedCriteria&>  v;
		
	public:
		CompositeCriteria();
		~CompositeCriteria();
		void release();

		int size();
		int check(const float* values);
		
		CombinedCriteria operator [](int index);
		
		CompositeCriteria& operator |(Criteria &);
		CompositeCriteria& operator |(CompositeCriteria &);
		CompositeCriteria& operator |(CombinedCriteria &);
		
		CompositeCriteria& operator &(Criteria &);
		CompositeCriteria& operator &(CompositeCriteria &);
		CompositeCriteria& operator &(CombinedCriteria &);
		
		CompositeCriteria& operator |=(CombinedCriteria &);
		CompositeCriteria& operator &=(Criteria &);
		CompositeCriteria& operator &=(CombinedCriteria &);

		static void operator =(CompositeCriteria &, CompositeCriteria &);
};

inline int CompositeCriteria::getSize() {return v.size();}

#endif