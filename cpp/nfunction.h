#ifndef _NFUNCTION_H
#define _NFUNCTION_H

#include "../linux/common.h"

class NFunction {
	private:
		char *str;
		unsigned int len;

		char variable[4];
		char valLen;

		NMASTList *prefix;
		NMASTList *domain;
		ListCriteria *criterias;

		NMAST **variableNode;
		int numVarNode;

	public:
		NFunction();
		~NFunction();

		void parse();
		
		void release();
};

#endif