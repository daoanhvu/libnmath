#ifndef _NFUNCTION_H
#define _NFUNCTION_H

#include <iostream>
#include "criteria.h"
#include "common.h"
#include "nlablexer.h"
#include "nlabparser.h"

namespace nmath {
	typedef struct tagListCriteria {
		Criteria **list;
		unsigned int loggedSize;
		unsigned int size;
	} ListCriteria;

	class NFunction {
	private:
		char *text;
		unsigned int textLen;
		char variables[4];
		char valLen;

		NLabLexer *mLexer;
		NLabParser *mParser;

		NMASTList prefix;
		NMASTList domain;
		ListCriteria criteria;

		Token *mTokens;
		int mTokenCapability;
		int mTokeSize;

		NMAST **variableNode;
		int numVarNode;

		int errorCode;
		int errorColumn;

		ListFData* getSpaceFor2UnknownVariables(const float *inputInterval, float epsilon);

	public:
		NFunction();
		~NFunction();
		int parse(const char *str, int len);
		void release();
		int reduce();
		double dcalc(double *values, int numOfValue);
		ListFData* getSpace(const float *values, float epsilone);

		char* getText() const { return text; }

		char getVarCount() const { return valLen; }
		NMASTList* getPrefixList() const { return (NMASTList*)&prefix; }
		NMASTList* getDomainList() const { return (NMASTList*)&domain; }
		ListCriteria* getCriteriaList() const { return (ListCriteria*)&criteria; }

		friend std::ostream& operator<< (std::ostream& os, const NFunction& f);
	};

	void releaseNMATree(NMASTList **t);

	NMAST* cloneTree(NMAST *t, NMAST *cloneParent);
	NMAST* d_product(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x);
	NMAST* d_sin(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x);
	NMAST* d_cos(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x);
	NMAST* d_tan(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x);
	NMAST* d_cotan(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x);
	NMAST* d_asin(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x);
	NMAST* d_acos(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x);
	NMAST* d_atan(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x);
	NMAST* d_sqrt(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x);
	NMAST* d_pow_exp(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x);
	NMAST* d_sum_subtract(NMAST *t, int type, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x);
	NMAST* d_quotient(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x);
	NMAST* d_sqrt(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x);
	bool isContainVar(NMAST *t, char x);
#ifdef _WIN32
	unsigned int __stdcall reduce_t(void *param);
	unsigned int __stdcall calc_t(void *param);
	unsigned int __stdcall calcF_t(void *param);
	unsigned int __stdcall derivative(void *p);
#else
	void* reduce_t(void *param);
	void* calc_t(void *param);
	void* calcF_t(void *param);
	void* derivative(void *p);
#endif

}
#endif