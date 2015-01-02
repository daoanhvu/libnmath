#ifndef _NFUNCTION_H
#define _NFUNCTION_H

#include <iostream>
#include "criteria.h"
#include "common.h"
#include "nlablexer.h"
#include "nlabparser.h"
#include "compositecriteria.h"

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

		NMASTList prefix;		
		ListCriteria criteria;

		NMAST **variableNode;
		int numVarNode;

		int errorCode;
		int errorColumn;
		
		/*
			fidx Function expression index (the index of prefix.list)
			vidx variable index
		*/
		NMAST* getDerivativeByVariable(int fidx, int vidx);

		ListFData* getSpaceFor2UnknownVariables(const float *inputInterval, float epsilon, bool needNormalVector);
		FData* getSpaceFor2WithANDComposite(int prefixIndex, const float *inputInterval, float epsilon, const CompositeCriteria* c, NMAST **df);

	public:
		NFunction();
		~NFunction();
		
		int getErrorCode() { return errorCode; }
		int getErrorColumn() { return errorColumn; }

		int toString(char *str, int buflen);
		
		int parse(const char *str, int len, NLabLexer *mLexer, NLabParser *mParser);
		int parse(Token *mTokens, int mTokeSize, NLabParser *mParser);
		void release();
		int reduce();
		double dcalc(double *values, int numOfValue);
		ListFData* getSpace(const float *inputInterval, float epsilon, bool needNormalVector);
		char getVariable(int index) { return variables[index]; }
		char* getText() const { return text; }
		int getVarCount() const { return valLen; }
		
		NMASTList* getPrefixList() const { return (NMASTList*)&prefix; }
		NMAST* getPrefix(int index) const { return prefix.list[index]; }
		ListCriteria* getCriteriaList() const { return (ListCriteria*)&criteria; }
		Criteria* getCriteria(int index) const { return criteria.list[index]; }
#ifdef _WIN32
		friend std::ostream& operator<< (std::ostream& os, const NFunction& f);
#endif
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