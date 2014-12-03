#ifndef _NFUNCTION_H
#define _NFUNCTION_H

#include "common.h"
#include "nlablexer.h"
#include "nlabparser.h"

namespace nmath {
	typedef struct tagDParam {
		NMAST *t;
		char variables[4];
		int error;
		double *values;
		double retv;
		NMAST *returnValue;
	}DParam;

	typedef struct tagDParamF {
		NMAST *t;
		char variables[4];
		int error;
		float *values;
		float retv;
		NMAST *returnValue;
	}DParamF;

	class NFunction {
	private:
		char *text;
		unsigned int textLen;
		char variables[4];
		char valLen;

		//Aggregation association
		NLabLexer *mLexer;
		NLabParser *mParser;

		NMASTList *prefix;
		NMASTList *domain;
		ListCriteria *criterias;

		NMAST **variableNode;
		int numVarNode;

		int errorCode;
		int errorColumn;

	public:
		NFunction();
		~NFunction();

		int parse(const char *str, int len);
		void release();

		int reduce();
		double dcalc(double *values, int numOfValue);
	};

#ifdef _WIN32
	unsigned int __stdcall reduce_t(void *param);
	unsigned int __stdcall calc_t(void *param);
#else
	void* reduce_t(void *param);
	void* calc_t(void *param);
#endif

}
#endif