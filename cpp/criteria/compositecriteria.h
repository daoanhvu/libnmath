#ifndef _COMPOSITECRITERIA_H
#define _COMPOSITECRITERIA_H

#include "combinedcriteria.h"

class CompositeCriteria {
	private:
		CombinedCriteria **list;
		unsigned int loggedSize;
		unsigned int size;

	public:
		CompositeCriteria();
		~CompositeCriteria();

		int getSize();
		CombinedCriteria operator [](int index);
		int isInInterval(const float* values);
};

inline int CompositeCriteria::getSize() {return size;}

#endif