#include <string>
#include <cstdlib>
#include <math.h>

#ifdef _DEBUG
#include <stdio.h>
#include <iostream>
#endif

#ifdef _WIN32
#include <Windows.h>
#include <process.h>
#endif

#ifdef _ADEBUG
#include <android/log.h>
#define LOG_TAG "NFunction"
#define LOG_LEVEL 10
#define LOGI(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__);}
#define LOGE(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__);}
#endif

#include "StringUtil.h"
#include "nfunction.h"
#include "nlablexer.h"
#include "nlabparser.h"
#include "SimpleCriteria.h"
#include "compositecriteria.h"

using namespace nmath;

NFunction::NFunction(): text(0), valLen(0) {
	prefix.list = 0;
	prefix.loggedSize = 0;
	prefix.size = 0;

	criteria.list = 0;
	criteria.size = 0;
	criteria.loggedSize = 0;
}

NFunction::~NFunction() {
	release();
}
#ifdef _WIN32
std::ostream& nmath::operator <<(std::ostream& os, const NFunction& f) {
	int i;
	os << "\n Function: " << f.getText() << "\n";
	os << "Number of variable: " << (int)f.getVarCount() << "\n";
	if(f.getPrefixList()->size > 0){
		for(i = 0; i< f.getPrefixList()->size; i++){
			os << "Prefix Expresion "<< i <<": \t \n";
			printNMAST(f.getPrefixList()->list[i], 0, os);

			if(f.getCriteriaList()->list[i] != NULL) {
				os << "\n \t Criteria: \n";
				os << (*f.getCriteriaList()->list[i]) << "\n";
			}
		}
	}

	return os;
}
#endif

/*
Parse the input string in object f to NMAST tree
*/
int NFunction::parse(const char *str, int len, NLabLexer *mLexer, NLabParser *mParser) {
	int i, k, mTokeSize;
	Criteria *c;
	CompositeCriteria *cc;
	NMASTList domain;
	Token mTokens[100];
	
	domain.list = 0;
	domain.loggedSize = 0;
	domain.size = 0;

	mLexer->reset();

	if (text != NULL) {
		delete[] text;
	}

	text = new char[len+1];
	memcpy(text, str, len);
	textLen = len;
	text[len] = '\0';

	mTokeSize = mLexer->lexicalAnalysis(text, textLen, 0, mTokens, 100/*capacity of mTokens*/, 0);
	errorColumn = mLexer->getErrorColumn();
	errorCode = mLexer->getErrorCode();
	if (errorCode != NMATH_NO_ERROR) {
		return errorCode;
	}
		
	//TODO: Need to release prefix and domain before call parseFunctionExpression from NLabParser
	errorCode = mParser->parseFunctionExpression(mTokens, mTokeSize, &prefix, &domain);
	errorColumn = mParser->getErrorColumn();
	if (errorCode == NMATH_NO_ERROR) {
		valLen = mParser->variableCount();
		if (valLen > 0)
			memcpy(variables, mParser->variables(), valLen);

		if (domain.list != NULL) {
			criteria.list = (Criteria**)malloc(sizeof(Criteria*) * domain.size);
			criteria.loggedSize = domain.size;
			criteria.size = domain.size;
			memset(criteria.list, 0, criteria.loggedSize* sizeof(int));

			for (i = 0; i < domain.size; i++) {
				c = NULL;
				if (domain.list[i] != NULL) {
					c = nmath::buildCriteria(domain.list[i]);
					//atemp to normalize criteria so that it hold criteria for all variable in every it's element
					if ( (c!=NULL) && c->getCClassType() == COMPOSITE &&
						((CompositeCriteria*)c)->logicOperator() == OR) {
						cc = (CompositeCriteria*)c;
						cc->normalize(variables, valLen);
					}
				}
				criteria.list[i] = c;
			}
		}
	}
	
	if(domain.list != NULL) {
		for(i=0; i<domain.size; i++)
			::clearTree(&(domain.list[i]));
		free(domain.list);
		domain.list = NULL;
	}

	return errorCode;
}

/*
Parse the input string in object f to NMAST tree
*/
int NFunction::parse(Token *mTokens, int mTokeSize, NLabParser *mParser) {
	int i, k;
	Criteria *c;
	CompositeCriteria *cc;
	NMASTList domain;
	
	domain.list = 0;
	domain.loggedSize = 0;
	domain.size = 0;
	
	//TODO: Need to release prefix and domain before call parseFunctionExpression from NLabParser
#ifdef _ADEBUG
	//LOGI(1, "Number of token: %d", mTokeSize);
#endif
	errorCode = mParser->parseFunctionExpression(mTokens, mTokeSize, &prefix, &domain);
	errorColumn = mParser->getErrorColumn();
	if (errorCode == NMATH_NO_ERROR) {
		valLen = mParser->variableCount();
		if (valLen > 0)
			memcpy(variables, mParser->variables(), valLen);
#ifdef _ADEBUG
	//LOGI(1, "Variable on left: %c; variable on right: %c", prefix.list[0]->left->right->variable, prefix.list[0]->right->right->variable);
#endif		
		if (domain.list != NULL) {
			criteria.list = (Criteria**)malloc(sizeof(Criteria*) * domain.size);
			criteria.loggedSize = domain.size;
			criteria.size = domain.size;

			memset(criteria.list, 0, criteria.loggedSize*sizeof(int));

			for (i = 0; i < domain.size; i++) {
				c = NULL;
				if (domain.list[i] != NULL) {
					c = nmath::buildCriteria(domain.list[i]);
					//atemp to normalize criteria so that it hold criteria for all variable in every it's element
					if ( (c!=NULL) && c->getCClassType() == COMPOSITE &&
						((CompositeCriteria*)c)->logicOperator() == OR) {
						cc = (CompositeCriteria*)c;
						cc->normalize(variables, valLen);
					}
				}
				criteria.list[i] = c;
			}
		}
	}
	
	if(domain.list != NULL) {
		for(i=0; i<domain.size; i++)
			::clearTree(&(domain.list[i]));
		free(domain.list);
		domain.list = NULL;
	}

	return errorCode;
}

void NFunction::release() {
	int i;
	if (text != NULL) {
		delete[] text;
		text = 0;
		textLen = 0;
	}

	if(prefix.list != NULL) {
		for(i=0; i<prefix.size; i++)
			::clearTree(&(prefix.list[i]));
		free(prefix.list);
		prefix.list = NULL;
		prefix.loggedSize = 0;
		prefix.size = 0;
	}

	if (criteria.list != NULL) {
		for (i = 0; i < criteria.size; i++) {
			if (criteria.list[i] != NULL)
				delete criteria.list[i];
		}
		free(criteria.list);
		criteria.list = NULL;
		criteria.loggedSize = 0;
		criteria.size = 0;
	}
}

double NFunction::dcalc(double *values, int numOfValue) {
	DParam rp;
	rp.error = 0;
	rp.t = *(prefix.list);
	memcpy(rp.values, values, numOfValue * 2 * sizeof(double));
	memcpy(rp.variables, variables, 4);
	calc_t((void*)&rp);
	return rp.retv;
}

int NFunction::reduce() {
	DParam dp;
	int i;

	for(i=0; i<prefix.size; i++) {
		dp.t = prefix.list[i];
		dp.error = 0;
		nmath::reduce_t(&dp);
		prefix.list[i] = dp.t;
	}
	return 0;
}

int NFunction::toString(char *str, int buflen) {
	int start = 0;
	nmath::toString(prefix.list[0], str, &start, buflen);
	return start;
}

/*
	fidx Function expression index (the index of prefix.list)
	vidx variable index
*/
NMAST* NFunction::getDerivativeByVariable(int fidx, int vidx) {
	DParam d;
	
	d.t = prefix.list[fidx];
	d.error = 0;
	d.returnValue = NULL;
	d.variables[0] = variables[vidx];
	
	nmath::derivative(&d);
	
	return d.returnValue;
}

FData* NFunction::getSpaceFor2WithANDComposite(int prefixIndex, const float *inputInterval, 
			float epsilon, const CompositeCriteria* c, NMAST **df) {
	FData *sp;
	DParamF param;
	DParamF dparam0;
	DParamF dparam1;
	float min[2];
	float max[2];
	float *dataTemp;
	int *rowInfoTemp;
	float z;
	int i, j, k, elementOnRow;
	SimpleCriteria *sc;
	char currentVar;

	//used for normalizing nv
	float mod;
	
	for(k=0; k<valLen; k++) {
		currentVar = variables[k];
		//search for criteria that bounds the current variable
		for(j=0; j<c->size(); j++) {
			sc = (SimpleCriteria*)c->get(j);
			if(sc->getVariable() == currentVar) {
				min[k] = sc->getLeftValue();
				if(sc->getType() == GT_LT || sc->getType() == GT_LTE)
					min[k] = sc->getLeftValue() + epsilon;
					
				max[k] = sc->getRightValue();
				if(sc->getType() == GT_LT || sc->getType() == GTE_LT)
					max[k] = sc->getRightValue() - epsilon;
		
				break;
			}
		}
	}
	
	sp = (FData*)malloc(sizeof(FData));
	sp->dimension = ((df==0)?3:6);
	sp->loggedSize = 20;
	sp->dataSize = 0;
	sp->data = (float*)malloc(sizeof(float) * sp->loggedSize);
	sp->loggedRowCount = 0;
	sp->rowCount = 0;
	sp->rowInfo= NULL;

	param.t = prefix.list[prefixIndex];
	param.variables[0] = variables[0];
	param.variables[1] = variables[1];
	param.error = 0;
	
	param.values[0] = min[0];
	while(param.values[0] < max[0] ) {
		elementOnRow = 0;
		param.values[1] = min[1];
		while(param.values[1] < max[1]) {
			calcF_t((void*)&param);
			z = param.retv;
//#ifdef _ADEBUG
//			LOGI(1, "x = %f, y=%f, z=%f", param.values[0], param.values[1], z);
//#endif
			if( sp->dataSize >= sp->loggedSize - (sp->dimension) ){
				sp->loggedSize += 20;
				dataTemp = (float*)realloc(sp->data, sizeof(float) * sp->loggedSize);
				if(dataTemp != NULL)
					sp->data = dataTemp;
			}
			sp->data[sp->dataSize++] = param.values[0];
			sp->data[sp->dataSize++] = param.values[1];
			sp->data[sp->dataSize++] = z;
			
			/******** Now, calculate the normal vector at x, y, z **************/
			if(df != NULL) {
				dparam0.t = df[0];
				dparam0.variables[0] = variables[0];
				dparam0.variables[1] = variables[1];
				dparam0.error = 0;
				dparam0.values[0] = param.values[0];
				dparam0.values[1] = param.values[1];
				calcF_t((void*)&dparam0);
				
				dparam1.t = df[1];
				dparam1.variables[0] = variables[0];
				dparam1.variables[1] = variables[1];
				dparam1.error = 0;
				dparam1.values[0] = param.values[0];
				dparam1.values[1] = param.values[1];
				calcF_t((void*)&dparam1);

				//mod = sqrt(dparam0.retv*dparam0.retv + dparam1.retv*dparam1.retv + 1);
				//sp->data[sp->dataSize++] = dparam0.retv/mod;
				//sp->data[sp->dataSize++] = dparam1.retv/mod;
				//sp->data[sp->dataSize++] = -1/mod;

				sp->data[sp->dataSize++] = dparam0.retv;
				sp->data[sp->dataSize++] = dparam1.retv;
				sp->data[sp->dataSize++] = -1.0f;
			}
			/*******************************************/
			
			elementOnRow++;
			param.values[1] += epsilon;
		}
		
		if(sp->rowCount >= sp->loggedRowCount){
			sp->loggedRowCount += 10;
			rowInfoTemp = (int*)realloc(sp->rowInfo, sizeof(int) * sp->loggedRowCount);
			if(rowInfoTemp != NULL)
				sp->rowInfo = rowInfoTemp;
		}

		sp->rowInfo[sp->rowCount++] = elementOnRow;
		param.values[0] += epsilon;
	}
	
	return sp;
}

ListFData* NFunction::getSpaceFor2UnknownVariables(const float *inputInterval, float epsilon, bool needNormalVector) {
	ListFData *lstData = NULL;
	FData **tempList;
	FData *sp;
	DParamF param;
	DParam reduceParam;
	CompositeCriteria *outCriteria;
	SimpleCriteria *sc;
	int i, j;
	NMAST *df[2] = {0, 0};
	
	CompositeCriteria cc;
	cc.setOperator(AND);
	sc = new SimpleCriteria(GTE_LTE, variables[0], inputInterval[0], inputInterval[1], false, false);
	cc.add(sc);
	sc = new SimpleCriteria(GTE_LTE, variables[1], inputInterval[2], inputInterval[3], false, false);
	cc.add(sc);
	sc = NULL;

	lstData = new ListFData;
	lstData->size = 0;
	lstData->loggedSize = prefix.size;
	lstData->list = (FData**)malloc(sizeof(FData*) * lstData->loggedSize);

	for(i=0; i<prefix.size; i++) {
		if(needNormalVector) {
			df[0] = getDerivativeByVariable(i, 0);
			reduceParam.t = df[0];
			reduce_t((void*)&reduceParam);
			df[0] = reduceParam.t;

			df[1] = getDerivativeByVariable(i, 1);
			reduceParam.t = df[1];
			reduce_t((void*)&reduceParam);
			df[1] = reduceParam.t;
#ifdef _DEBUG
			std::cout << "dx/df: \n";
			printNMAST(df[0], 0, std::cout);
			std::cout << "dy/df: \n";
			printNMAST(df[1], 0, std::cout);
#endif
		}
		
		if(criteria.list[i] == NULL) {
			if (needNormalVector)
				sp = getSpaceFor2WithANDComposite(i, inputInterval, epsilon, &cc, df);
			else
				sp = getSpaceFor2WithANDComposite(i, inputInterval, epsilon, &cc, 0);
				
			if(lstData->size >= lstData->loggedSize) {
				lstData->loggedSize += 2;
				tempList = (FData**)realloc(lstData->list, sizeof(FData*) * lstData->loggedSize);
				if(tempList != NULL)
					lstData->list = tempList;
			}
			lstData->list[lstData->size++] = sp;
		} else {
			outCriteria = (CompositeCriteria*)criteria.list[i]->getIntervalF(inputInterval, this->variables, valLen);
#ifdef _DEBUG
			std::cout << "\n Get Interval: \n";
			std::cout << ((Criteria&)*outCriteria) << "\n";
#endif
			switch(outCriteria->logicOperator()) {
				case AND:
					if (needNormalVector)
						sp = getSpaceFor2WithANDComposite(i, inputInterval, epsilon, outCriteria, df);
					else
						sp = getSpaceFor2WithANDComposite(i, inputInterval, epsilon, outCriteria, 0);
						
					if(lstData->size >= lstData->loggedSize) {
						lstData->loggedSize += 2;
						tempList = (FData**)realloc(lstData->list, sizeof(FData*) * lstData->loggedSize);
						if(tempList != NULL)
							lstData->list = tempList;
					}
					lstData->list[lstData->size++] = sp;
				break;
				
				case OR:
					for(j=0; j<outCriteria->size(); j++) {
						if(needNormalVector)
							sp = getSpaceFor2WithANDComposite(i, inputInterval, epsilon, (CompositeCriteria*)(outCriteria->get(j)), df);
						else 
							sp = getSpaceFor2WithANDComposite(i, inputInterval, epsilon, (CompositeCriteria*)(outCriteria->get(j)), 0);
						if(lstData->size >= lstData->loggedSize) {
							lstData->loggedSize += 2;
							tempList = (FData**)realloc(lstData->list, sizeof(FData*) * lstData->loggedSize);
							if(tempList != NULL)
								lstData->list = tempList;
						}
						lstData->list[lstData->size++] = sp;
					}
				break;
			}
		}
		
		//free derivative trees
		if(df[0] != 0) {
			clearTree(df);
		}
		
		if(df[1] != 0) {
			clearTree(df+1);
		}
		
	} //end for

	return lstData;
}

ListFData* NFunction::getSpace(const float *inputInterval, float epsilon, bool needNormalVector) {
	ListFData *lstData = NULL;
	FData *sp;
	DParamF param;
	SimpleCriteria* sc;
	CompositeCriteria* cc;
	Criteria* outCriteria;
	float y, lastX, rightVal;
	int elementOnRow = 0;
	float *tmpP;

	switch (valLen) {
		case 1:
			if (criteria.list == NULL) {

				sp = (FData*)malloc(sizeof(FData));
				sp->dimension = 2;
				sp->loggedSize = 20;
				sp->dataSize = 0;
				sp->data = (float*)malloc(sizeof(float) * sp->loggedSize);
				sp->rowCount = 0;
				sp->loggedRowCount = 1;
				sp->rowInfo = (int*)malloc(sizeof(int));

				param.error = NMATH_NO_ERROR;
				param.variables[0] = variables[0];
				param.values[0] = inputInterval[0];
				param.t = this->prefix.list[0];
				while (param.values[0] <= inputInterval[1]) {
					calcF_t(&param);
					y = param.retv;

					if (sp->dataSize >= sp->loggedSize - 2){
						sp->loggedSize += 20;
						tmpP = (float*)realloc(sp->data, sizeof(float) * sp->loggedSize);
						if (tmpP != NULL)
							sp->data = tmpP;
					}
					sp->data[sp->dataSize++] = param.values[0];
					sp->data[sp->dataSize++] = y;
					elementOnRow++;
					lastX = param.values[0];
					param.values[0] += epsilon;
				}

				if ((lastX < inputInterval[1]) && (param.values[0] > inputInterval[1])) {
					param.values[0] = inputInterval[1];
					calcF_t((void*)&param);
					y = param.retv;
					if (sp->dataSize >= sp->loggedSize - 2){
						sp->loggedSize += 4;
						tmpP = (float*)realloc(sp->data, sizeof(float) * sp->loggedSize);
						if (tmpP != NULL)
							sp->data = tmpP;
					}
					sp->data[sp->dataSize++] = param.values[0];
					sp->data[sp->dataSize++] = y;
					elementOnRow++;
				}

				sp->rowInfo[sp->rowCount++] = elementOnRow;
				lstData = new ListFData;
				lstData->size = 0;
				lstData->list = (FData**)malloc(sizeof(FData*) * 1);
				lstData->list[lstData->size++] = sp;
				

				return lstData;
			}

			outCriteria = criteria.list[0]->getIntervalF(inputInterval, variables, valLen);

			if (outCriteria == NULL) return NULL;

			/*
				Because this is an one-unknown variable function so output criteria is SIMPLE or
				OR-COMPOSITE criteria

				if out criteria is SIMPLE: this function is continueous on the output interval
				otherwise this function is not continueous on the output interval, in other word, it's continueous 
				in output criteria partially
			*/
			switch (outCriteria->getCClassType()) {
				case SIMPLE:
					sc = (SimpleCriteria*)outCriteria;

					sp = (FData*)malloc(sizeof(FData));
					sp->dimension = 2;
					sp->loggedSize = 20;
					sp->dataSize = 0;
					sp->data = (float*)malloc(sizeof(float) * sp->loggedSize);
					sp->rowCount = 0;
					sp->loggedRowCount = 1;
					sp->rowInfo = (int*)malloc(sizeof(int));

					param.error = NMATH_NO_ERROR;
					param.variables[0] = variables[0];
					param.values[0] = (float)sc->getLeftValue();
					param.t = prefix.list[0];
					rightVal = sc->getRightValue();
					while (param.values[0] <= rightVal) {
						calcF_t(&param);
						y = param.retv;

						if (sp->dataSize >= sp->loggedSize - 2){
							sp->loggedSize += 20;
							tmpP = (float*)realloc(sp->data, sizeof(float) * sp->loggedSize);
							if (tmpP != NULL)
								sp->data = tmpP;
						}
						sp->data[sp->dataSize++] = param.values[0];
						sp->data[sp->dataSize++] = y;
						elementOnRow++;
						lastX = param.values[0];
						param.values[0] += epsilon;
					}

					if ((lastX < rightVal) && (param.values[0] > rightVal)) {
						param.values[0] = rightVal;
						calcF_t((void*)&param);
						y = param.retv;
						if (sp->dataSize >= sp->loggedSize - 2){
							sp->loggedSize += 4;
							tmpP = (float*)realloc(sp->data, sizeof(float) * sp->loggedSize);
							if (tmpP != NULL)
								sp->data = tmpP;
						}
						sp->data[sp->dataSize++] = param.values[0];
						sp->data[sp->dataSize++] = y;
						elementOnRow++;
					}

					sp->rowInfo[sp->rowCount++] = elementOnRow;
					lstData = new ListFData;
					lstData->size = 0;
					lstData->list = (FData**)malloc(sizeof(FData*) * 1);
					lstData->list[lstData->size++] = sp;
					break;

				case COMPOSITE: //OR-COMPOSITE criteria
					int i; 
					cc = (CompositeCriteria*)outCriteria;
					lstData = new ListFData;
					lstData->size = 0;
					lstData->list = (FData**)malloc(sizeof(FData*) * cc->size());
					for (i = 0; i < cc->size(); i++) {
						sc = (SimpleCriteria*)cc->get(i);

						sp = (FData*)malloc(sizeof(FData));
						sp->dimension = 2;
						sp->loggedSize = 20;
						sp->dataSize = 0;
						sp->data = (float*)malloc(sizeof(float) * sp->loggedSize);
						sp->rowCount = 0;
						sp->loggedRowCount = 1;
						sp->rowInfo = (int*)malloc(sizeof(int));

						param.error = NMATH_NO_ERROR;
						param.variables[0] = variables[0];
						param.values[0] = sc->getLeftValue();
						param.t = prefix.list[0];
						rightVal = sc->getRightValue();

						while (param.values[0] <= rightVal) {
							calcF_t(&param);
							y = param.retv;

							if (sp->dataSize >= sp->loggedSize - 2){
								sp->loggedSize += 20;
								tmpP = (float*)realloc(sp->data, sizeof(float) * sp->loggedSize);
								if (tmpP != NULL)
									sp->data = tmpP;
							}
							sp->data[sp->dataSize++] = param.values[0];
							sp->data[sp->dataSize++] = y;
							elementOnRow++;
							lastX = param.values[0];
							param.values[0] += epsilon;
						}

						if ((lastX < rightVal) && (param.values[0] > rightVal)) {
							param.values[0] = rightVal;
							calcF_t((void*)&param);
							y = param.retv;
							if (sp->dataSize >= sp->loggedSize - 2){
								sp->loggedSize += 4;
								tmpP = (float*)realloc(sp->data, sizeof(float) * sp->loggedSize);
								if (tmpP != NULL)
									sp->data = tmpP;
							}
							sp->data[sp->dataSize++] = param.values[0];
							sp->data[sp->dataSize++] = y;
							elementOnRow++;
						}

						sp->rowInfo[sp->rowCount++] = elementOnRow;
						lstData->list[lstData->size++] = sp;
					}
					break;
			}

			break;

		case 2:
			lstData = getSpaceFor2UnknownVariables(inputInterval, epsilon, needNormalVector);
			break;

		case 3:
			break;
	}

	return lstData;
}

/*****************************************************************************************************************/

void nmath::releaseNMATree(NMASTList **t) {
	int i, j = 0, k = 0;

	if ((*t) == NULL) return;

	if ((*t) != NULL){
		for (i = 0; i<(*t)->size; i++){
			clearTree(&((*t)->list[i]));
		}
		if ((*t)->list != NULL){
			free((*t)->list);
		}
		(*t)->size = 0;
		(*t)->loggedSize = 0;
		free(*t);
		*t = NULL;
	}
}


/*
Check if a tree contains variable x
@param t the tree
@param x variable to check if existed
*/
bool nmath::isContainVar(NMAST *t, char x){

	if ((t == NULL) || (t->type == NUMBER) || (t->type == PI_TYPE) || (t->type == E_TYPE))
		return false;

	if (t->type == VARIABLE) {
		if (t->variable == x)
			return (t->sign>0);
		return false;
	}

	return (isContainVar(t->left, x) || isContainVar(t->right, x));
}


NMAST * nmath::cloneTree(NMAST *t, NMAST *cloneParent){
	NMAST *c;

	if(t==NULL) {
		return 0;
	}

	c = (NMAST*)malloc(sizeof(NMAST));
	memcpy(c, t, sizeof(NMAST));
	c->parent = cloneParent;
	c->left = cloneTree(t->left, c);
	c->right = cloneTree(t->right, c);
	return c;
}


#ifdef _WIN32
unsigned int __stdcall nmath::reduce_t(void *param){
	HANDLE thread_1 = 0, thread_2 = 0;
#else
void* nmath::reduce_t(void *param){
	pthread_t thrLeft, thrRight;
	int idThrLeft = -1, idThrRight = -1;
#endif
	DParam *dp = (DParam *)param;
	NMAST *p;
	NMAST *o1, *o2;
	DParam this_param_left;
	DParam this_param_right;
	this_param_left.error = this_param_right.error = 0;

	/* If the tree is NULL */
	if ((dp->t) == NULL){
		return 0;
	}

#ifdef _WIN32
	/*
	If this node is has left child, and the left child is an operator or a function then we
	create a thread to reduce the left child.
	*/
	if (((dp->t)->left) != NULL && isFunctionOROperator(((dp->t)->left)->type)){
		this_param_left.t = (dp->t)->left;
		thread_1 = (HANDLE)_beginthreadex(NULL, 0, &reduce_t, (void*)&this_param_left, 0, NULL);
	}

	/*
	If this node is has right child, and the right child is an operator or a function then we
	create a thread to reduce the right child.
	*/
	if (((dp->t)->right) != NULL && isFunctionOROperator(((dp->t)->right)->type)){
		this_param_right.t = (dp->t)->right;
		thread_2 = (HANDLE)_beginthreadex(NULL, 0, &reduce_t, (void*)&this_param_right, 0, NULL);
	}
	if (thread_1 != 0){
		WaitForSingleObject(thread_1, INFINITE);
		CloseHandle(thread_1);
	}

	if (thread_2 != 0){
		WaitForSingleObject(thread_2, INFINITE);
		CloseHandle(thread_2);
	}
#else
	/*
	If this node is has left child, and the left child is an operator or a function then we
	create a thread to reduce the left child.
	*/
	if (((dp->t)->left) != NULL && isFunctionOROperator(((dp->t)->left)->type)){
		this_param_left.t = (dp->t)->left;
		idThrLeft = pthread_create(&thrLeft, NULL, reduce_t, (void*)(&this_param_left));
	}
	if (((dp->t)->right) != NULL && isFunctionOROperator(((dp->t)->right)->type)){
		this_param_right.t = (dp->t)->right;
		idThrRight = pthread_create(&thrRight, NULL, reduce_t, (void*)(&this_param_right));
	}
	if (idThrLeft == 0)
		pthread_join(thrLeft, NULL);
	if (idThrRight == 0)
		pthread_join(thrRight, NULL);
#endif

	if (this_param_left.error != 0){
		dp->error = this_param_left.error;
#ifdef _WIN32
		return dp->error;
#else
		return &(dp->error);
#endif
	}

	if (this_param_right.error != 0){
		dp->error = this_param_right.error;
#ifdef _WIN32
		return dp->error;
#else
		return &(dp->error);
#endif
	}
	/*************************************************************************************/

	/*
	We don't reduce a node if it's a variable, a NAME, a number, PI, E
	*/
	if (((dp->t)->type == VARIABLE) || ((dp->t)->type == NAME) || isConstant((dp->t)->type))
#ifdef _WIN32
		return dp->error;
#else
		return &(dp->error);
#endif

	//if this node is an operator
	if (((dp->t)->type == PLUS) || ((dp->t)->type == MINUS) || ((dp->t)->type == MULTIPLY)
		|| ((dp->t)->type == DIVIDE) || ((dp->t)->type == POWER)) {
		if (((dp->t)->left != NULL) && ((dp->t)->right != NULL)) {

			/*****************************************************************/
			if (isConstant((dp->t)->right->type) &&
				((dp->t)->left->type == PLUS || (dp->t)->left->type == MINUS ||
				(dp->t)->left->type == MULTIPLY /*|| (dp->t)->left->type == DIVIDE */)) {

				o1 = dp->t;
				o2 = (dp->t)->left;
				if (o1->priority == o2->priority){
					if (isConstant(o2->left->type)) {
						switch (o1->type){
						case PLUS:
							o2->left->value += o1->right->value;
							break;

						case MINUS:
							o2->left->value -= o1->right->value;
							break;

						case MULTIPLY:
							o2->left->value *= o1->right->value;
							break;

							//case DIVIDE:
							//break;
						}

						if ((o1->parent != NULL) && (o1 == o1->parent->left)) {
							//o1 is left child
							o1->parent->left = o2;
						}
						else if (o1->parent != NULL) {
							o1->parent->right = o2;
						}
						o2->parent = o1->parent;
						dp->t = o2;

						//remove o1 from the current tree
						o1->parent = NULL;
						o1->left = NULL;
						clearTree(&o1);
						//free(o1);
					}
					else if (isConstant(o2->right->type)){
						switch (o1->type){
						case PLUS:
							o2->right->value += o1->right->value;
							break;

						case MINUS:
							o2->right->value -= o1->right->value;
							break;

						case MULTIPLY:
							o2->right->value *= o1->right->value;
							break;

							//case DIVIDE:
							//break;

						}

						if ((o1->parent != NULL) && o1 == o1->parent->left) {
							//o1 is left child
							o1->parent->left = o2;
						}
						else if (o1->parent != NULL) {
							o1->parent->right = o2;
						}

						//remove o1 from the current tree
						o2->parent = o1->parent;
						dp->t = o2;
						o1->parent = NULL;
						o1->left = NULL;
						clearTree(&o1);
						//free(o1);
					}

					if (o2->type == PLUS && o2->left->value < 0){
						o2->type = MINUS;
						o2->left->value = -(o2->left->value);
					}
				}

			}
			else if (isConstant((dp->t)->left->type) &&
				((dp->t)->right->type == PLUS || (dp->t)->right->type == MINUS ||
				(dp->t)->right->type == MULTIPLY /*|| (dp->t)->right->type == DIVIDE */)) {

				o1 = dp->t;
				o2 = (dp->t)->right;
				if (o1->priority == o2->priority){
					if (isConstant(o2->left->type)){
						switch (o1->type) {
						case PLUS:
							o2->left->value += o1->left->value;
							break;

						case MINUS:
							o2->left->value -= o1->left->value;
							break;

						case MULTIPLY:
							o2->left->value *= o1->left->value;
							break;

							//case DIVIDE:
							//break;
						}

						if ((o1->parent != NULL) && (o1 == o1->parent->left)) {
							//o1 is left child
							o1->parent->left = o2;
						}
						else if (o1->parent != NULL) {
							o1->parent->right = o2;
						}
						o2->parent = o1->parent;
						dp->t = o2;

						//remove o1 from the current tree
						o1->parent = NULL;
						o1->right = NULL;
						clearTree(&o1);
						//free(o1);
					}
					else if (isConstant(o2->right->type)){
						switch (o1->type){
						case PLUS:
							o2->right->value += o1->left->value;
							break;

						case MINUS:
							o2->right->value -= o1->left->value;
							break;

						case MULTIPLY:
							o2->right->value *= o1->left->value;
							break;

							//case DIVIDE:
							//break;

						}

						if ((o1->parent != NULL) && o1 == o1->parent->left) {
							//o1 is left child
							o1->parent->left = o2;
						}
						else if (o1->parent != NULL) {
							o1->parent->right = o2;
						}

						//remove o1 from the current tree
						o2->parent = o1->parent;
						dp->t = o2;
						o1->parent = NULL;
						o1->right = NULL;
						clearTree(&o1);
						//free(o1);
					}

					if (o2->type == PLUS && o2->left->value < 0){
						o2->type = MINUS;
						o2->left->value = -(o2->left->value);
					}
				}

			}

			/****************************************************************/

			/* take care of special cases */
			if ((dp->t)->type == PLUS){
				/* 0 + something */
				if ((((dp->t)->left)->type == NUMBER) && ((dp->t)->left)->value == 0.0) {

					//remove the left child (value 0)
					p = (dp->t)->left;
					free(p);
					(dp->t)->left = NULL;

					//move right child to its parent
					if ((dp->t)->right != NULL){
						p = (dp->t)->right;

						(dp->t)->type = p->type;
						(dp->t)->variable = p->variable;
						(dp->t)->value = p->value;
						(dp->t)->valueType = p->valueType;
						//(dp->t)->frValue = p->frValue;
						(dp->t)->priority = getPriorityOfType((dp->t)->type);
						(dp->t)->sign = p->sign;
						/* NO copy parent */
						(dp->t)->left = p->left;
						(dp->t)->right = p->right;

						/* Now release p */
						free(p);
						p = NULL;
#ifdef _WIN32
						return dp->error;
#else
						return &(dp->error);
#endif
					}
				}

				/* something + 0 */
				if ((((dp->t)->right)->type == NUMBER) && (((dp->t)->right)->value == 0.0)){

					//remove the right child (value 0)
					p = (dp->t)->right;
					free(p);
					(dp->t)->right = NULL;

					if ((dp->t)->left != NULL){
						p = (dp->t)->left;

						(dp->t)->type = p->type;
						(dp->t)->variable = p->variable;
						(dp->t)->value = p->value;
						(dp->t)->valueType = p->valueType;
						//(dp->t)->frValue = p->frValue;
						(dp->t)->priority = getPriorityOfType((dp->t)->type);
						(dp->t)->sign = p->sign;

						/* NO copy parent */
						(dp->t)->left = p->left;
						(dp->t)->right = p->right;

						/* Now release p */
						free(p);
						p = NULL;
#ifdef _WIN32
						return dp->error;
#else
						return &(dp->error);
#endif
					}
				}

				//Left and right child of this node are null, 
				// I'm not sure that this piece of code can be reached
				if (((dp->t)->left == NULL) || ((dp->t)->right == NULL))
#ifdef _WIN32
					return dp->error;
#else
					return &(dp->error);
#endif
			}

			if ((dp->t)->type == MULTIPLY){
				/* (0 * something) OR (something * 0) */
				if ((((dp->t)->left)->type == NUMBER && ((dp->t)->left)->value == 0) ||
					(((dp->t)->right)->type == NUMBER && ((dp->t)->right)->value == 0)){

					clearTree(&((dp->t)->left));
					clearTree(&((dp->t)->right));

					(dp->t)->type = NUMBER;
					(dp->t)->value = 0;
					(dp->t)->sign = 1;
					(dp->t)->variable = 0;

					/* MUST return here */
#ifdef _WIN32
					return dp->error;
#else
					return &(dp->error);
#endif
				}

				/* (1 * something) */
				if ((((dp->t)->left)->type == NUMBER && ((dp->t)->left)->value == 1)){

					clearTree(&((dp->t)->left));
					p = (dp->t)->right;

					(dp->t)->type = p->type;
					(dp->t)->value = p->value;
					(dp->t)->sign = p->sign;
					(dp->t)->variable = p->variable;
					//(dp->t)->frValue = p->frValue;
					(dp->t)->priority = getPriorityOfType((dp->t)->type);
					(dp->t)->valueType = p->valueType;
					(dp->t)->left = p->left;
					(dp->t)->right = p->right;

					free(p);

#ifdef _WIN32
					return dp->error;
#else
					return &(dp->error);
#endif
				}

				/* (something * 1) */
				if ((((dp->t)->right)->type == NUMBER && ((dp->t)->right)->value == 1)){
					clearTree(&((dp->t)->right));
					p = (dp->t)->left;
					(dp->t)->type = p->type;
					(dp->t)->value = p->value;
					(dp->t)->sign = p->sign;
					(dp->t)->variable = p->variable;
					//(dp->t)->frValue = p->frValue;
					(dp->t)->priority = getPriorityOfType((dp->t)->type);
					(dp->t)->valueType = p->valueType;
					(dp->t)->left = p->left;
					(dp->t)->right = p->right;
					free(p);
#ifdef _WIN32
					return dp->error;
#else
					return &(dp->error);
#endif
				}
			}

			if ((dp->t)->type == POWER){
				if (((dp->t)->right)->type == NUMBER){
					if (((dp->t)->right)->value == 0.0){
						// something ^ 0 
						clearTree(&((dp->t)->left));
						clearTree(&((dp->t)->right));

						(dp->t)->type = NUMBER;
						(dp->t)->variable = 0;
						(dp->t)->valueType = TYPE_FLOATING_POINT;
						(dp->t)->value = 1.0;
						//((dp->t)->frValue).numerator = 1;
						//((dp->t)->frValue).denomerator = 1;
						(dp->t)->priority = getPriorityOfType((dp->t)->type);
						(dp->t)->sign = 1;
						(dp->t)->left = (dp->t)->right = NULL;
#ifdef _WIN32
						return dp->error;
#else
						return &(dp->error);
#endif
					}

					if (((dp->t)->right)->value == 1.0){
						// something ^ 1 
						p = (dp->t)->right;
						free(p);

						p = (dp->t)->left;

						(dp->t)->type = p->type;
						(dp->t)->variable = p->variable;
						(dp->t)->valueType = p->valueType;
						(dp->t)->value = p->value;
						//(dp->t)->frValue = p->frValue;
						(dp->t)->priority = getPriorityOfType((dp->t)->type);
						(dp->t)->sign = p->sign;
						(dp->t)->left = p->left;
						(dp->t)->right = p->right;
						free(p);

#ifdef _WIN32
						return dp->error;
#else
						return &(dp->error);
#endif
					}
				}

				// 0 ^ something
				if ((((dp->t)->left)->type == NUMBER) && (((dp->t)->left)->value == 0.0)){

					p = (dp->t)->right;
					free(p);

					p = (dp->t)->left;
					free(p);

					(dp->t)->type = NUMBER;
					(dp->t)->valueType = TYPE_FLOATING_POINT;
					(dp->t)->value = 0.0;
					//((dp->t)->frValue).numerator = 0;
					//((dp->t)->frValue).denomerator = 1;
					(dp->t)->sign = p->sign;
					(dp->t)->left = (dp->t)->right = NULL;

#ifdef _WIN32
					return dp->error;
#else
					return &(dp->error);
#endif
				}

				// number ^ number, this is a trivial case
				if (((dp->t)->left != NULL) && ((dp->t)->right != NULL) && isConstant(((dp->t)->left)->type)
					&& isConstant(((dp->t)->right)->type)){
					// printf("Process 2-opt function %d\n", (t->right)->type);
					(dp->t)->value = doCalculate(((dp->t)->left)->value, ((dp->t)->right)->value, (dp->t)->type, &(dp->error));

					if (dp->error != 0)
#ifdef _WIN32
						return dp->error;
#else
						return &(dp->error);
#endif

					(dp->t)->type = NUMBER;
					p = (dp->t)->left;
					free(p);
					p = (dp->t)->right;
					free(p);
					(dp->t)->left = (dp->t)->right = NULL;
#ifdef _WIN32
					return dp->error;
#else
					return &(dp->error);
#endif
				}
			} // END OPERATOR POWER

			// trivial cases
			if (isConstant(((dp->t)->left)->type) && isConstant(((dp->t)->right)->type)){
				/*printf("doCalculate %c\n", t->type);*/
				(dp->t)->value = doCalculate(((dp->t)->left)->value, ((dp->t)->right)->value, (dp->t)->type, &(dp->error));
				if (dp->error != 0)
#ifdef _WIN32
					return dp->error;
#else
					return &(dp->error);
#endif
				(dp->t)->type = NUMBER;
				p = (dp->t)->left;
				free(p);
				p = (dp->t)->right;
				free(p);
				(dp->t)->left = (dp->t)->right = NULL;
#ifdef _WIN32
				return dp->error;
#else
				return &(dp->error);
#endif
			}
		}
	}

	/*
	Right now, I dont take case specicial case for functions like SIN(PI), LN(E)
	*/
	switch ((dp->t)->type){
	case SIN:
	case COS:
	case TAN:
	case COTAN:
	case ASIN:
	case ACOS:
	case ATAN:
	case LN:
	case SQRT:
		/*printf("Process function %d\n", t->type);*/
		if ((dp->t)->right != NULL){
			if (((dp->t)->right)->type == NUMBER){
				(dp->t)->value = doCalculate(0, ((dp->t)->right)->value, (dp->t)->type, &(dp->error));
				if (dp->error != 0)
#ifdef _WIN32
					return dp->error;
#else
					return &(dp->error);
#endif
				(dp->t)->type = NUMBER;
				if ((dp->t)->left != NULL){
					p = (dp->t)->left;
					free(p);
				}
				p = (dp->t)->right;
				free(p);
				(dp->t)->left = (dp->t)->right = NULL;
#ifdef _WIN32
				return dp->error;
#else
				return &(dp->error);
#endif
			}
		}
		/*printf("End Process function %d\n", t->type);*/
		break;

		/* Power is an operator
		case POWER:
		break;
		*/

	case LOG:
		if (((dp->t)->left != NULL) && ((dp->t)->right != NULL) && isConstant(((dp->t)->left)->type)
			&& isConstant(((dp->t)->right)->type)){
			/*printf("Process 2-opt function %d\n", (t->right)->type);*/
			(dp->t)->value = doCalculate(((dp->t)->left)->value, ((dp->t)->right)->value, (dp->t)->type, &(dp->error));

			if (dp->error != 0)
#ifdef _WIN32
				return dp->error;
#else
				return &(dp->error);
#endif

			(dp->t)->type = NUMBER;
			p = (dp->t)->left;
			free(p);
			p = (dp->t)->right;
			free(p);
			(dp->t)->left = (dp->t)->right = NULL;
#ifdef _WIN32
			return dp->error;
#else
			return &(dp->error);
#endif
		}
		break;
	}
#ifdef _WIN32
	return dp->error;
#else
	return &(dp->error);
#endif
}

#ifdef _WIN32
unsigned int __stdcall nmath::calc_t(void *param){
	HANDLE thread_1 = 0, thread_2 = 0;
#else
void* nmath::calc_t(void *param){
	pthread_t thrLeft, thrRight;
	int idThrLeft = -1, idThrRight = -1;
#endif
	DParam *dp = (DParam *)param;
	NMAST *t = dp->t;
	DParam this_param_left;
	DParam this_param_right;
	int var_index = -1;

	this_param_left.error = this_param_right.error = 0;
	this_param_left.variables[0] = this_param_right.variables[0] = dp->variables[0];
	this_param_left.variables[1] = this_param_right.variables[1] = dp->variables[1];
	this_param_left.variables[2] = this_param_right.variables[2] = dp->variables[2];
	this_param_left.variables[3] = this_param_right.variables[3] = dp->variables[3];
	//memcpy(this_param_left.variables, dp->variables, 4);
	//memcpy(this_param_right.variables, dp->variables, 4);
	memcpy(this_param_left.values, dp->values, 8);
	memcpy(this_param_right.values, dp->values, 8);

	/* If the input tree is NULL, we do nothing */
	if (t == NULL) return 0;


	if (t->type == VARIABLE){
		var_index = isInArray(dp->variables, t->variable);
		dp->retv = (t->sign>0) ? (dp->values[var_index]) : (-dp->values[var_index]);
#ifdef _WIN32
		return dp->error;
#else
		return &(dp->error);
#endif
	}

	if ((t->type == NUMBER) || (t->type == PI_TYPE) || (t->type == E_TYPE)){
		dp->retv = t->value;
#ifdef _WIN32
		return dp->error;
#else
		return &(dp->error);
#endif
	}

	this_param_left.t = t->left;
	this_param_right.t = t->right;
#ifdef _WIN32
	thread_1 = (HANDLE)_beginthreadex(NULL, 0, &calc_t, (void*)&this_param_left, 0, NULL);
	thread_2 = (HANDLE)_beginthreadex(NULL, 0, &calc_t, (void*)&this_param_right, 0, NULL);
	if (thread_1 != 0){
		WaitForSingleObject(thread_1, INFINITE);
		CloseHandle(thread_1);
	}
	if (thread_2 != 0){
		WaitForSingleObject(thread_2, INFINITE);
		CloseHandle(thread_2);
	}
#else
	idThrLeft = pthread_create(&thrLeft, NULL, calc_t, (void*)&this_param_left);
	idThrRight = pthread_create(&thrRight, NULL, calc_t, (void*)&this_param_right);
	if (idThrLeft == NMATH_NO_ERROR){
		pthread_join(thrLeft, NULL);
	}
	if (idThrRight == NMATH_NO_ERROR){
		pthread_join(thrRight, NULL);
	}
#endif
	/*******************************************************************************/

	/* Actually, we don't need to check error here b'cause the reduce phase does that
	if(this_param_left.error != 0){
	dp->error = this_param_left.error;
	return dp->error;
	}

	if(this_param_right.error != 0){
	dp->error = this_param_right.error;
	return dp->error;
	}*/

	dp->retv = t->sign * doCalculate(this_param_left.retv, this_param_right.retv, t->type, &(dp->error));
#ifdef _WIN32
	return dp->error;
#else
	return &(dp->error);
#endif
}

#ifdef _WIN32
unsigned int __stdcall nmath::calcF_t(void *param){
	HANDLE thread_1 = 0, thread_2 = 0;
#else
void* nmath::calcF_t(void *param){
	pthread_t thrLeft, thrRight;
	int idThrLeft = -1, idThrRight = -1;
#endif
	DParamF *dp = (DParamF *)param;
	NMAST *t = dp->t;
	DParamF this_param_left;
	DParamF this_param_right;
	int var_index = -1;

	this_param_left.error = this_param_right.error = 0;
	this_param_left.variables[0] = this_param_right.variables[0] = dp->variables[0];
	this_param_left.variables[1] = this_param_right.variables[1] = dp->variables[1];
	this_param_left.variables[2] = this_param_right.variables[2] = dp->variables[2];
	this_param_left.variables[3] = this_param_right.variables[3] = dp->variables[3];
	//memcpy(this_param_left.variables, dp->variables, 4);
	//memcpy(this_param_right.variables, dp->variables, 4);
	memcpy(this_param_left.values, dp->values, 8);
	memcpy(this_param_right.values, dp->values, 8);

	/* If the input tree is NULL, we do nothing */
	if (t == NULL) return 0;


	if (t->type == VARIABLE){
		var_index = isInArray(dp->variables, t->variable);
		dp->retv = (t->sign>0) ? (dp->values[var_index]) : (-dp->values[var_index]);
#ifdef _WIN32
		return dp->error;
#else
		return &(dp->error);
#endif
	}

	if ((t->type == NUMBER) || (t->type == PI_TYPE) || (t->type == E_TYPE)){
		dp->retv = (float)(t->value);
#ifdef _WIN32
		return dp->error;
#else
		return &(dp->error);
#endif
	}

	this_param_left.t = t->left;
	this_param_right.t = t->right;
#ifdef _WIN32
	thread_1 = (HANDLE)_beginthreadex(NULL, 0, &calcF_t, (void*)&this_param_left, 0, NULL);
	thread_2 = (HANDLE)_beginthreadex(NULL, 0, &calcF_t, (void*)&this_param_right, 0, NULL);
	if (thread_1 != 0){
		WaitForSingleObject(thread_1, INFINITE);
		CloseHandle(thread_1);
	}
	if (thread_2 != 0){
		WaitForSingleObject(thread_2, INFINITE);
		CloseHandle(thread_2);
	}
#else
	idThrLeft = pthread_create(&thrLeft, NULL, calcF_t, (void*)&this_param_left);
	idThrRight = pthread_create(&thrRight, NULL, calcF_t, (void*)&this_param_right);
	if (idThrLeft == NMATH_NO_ERROR){
		pthread_join(thrLeft, NULL);
	}
	if (idThrRight == NMATH_NO_ERROR){
		pthread_join(thrRight, NULL);
	}
#endif
	/*******************************************************************************/

	/* Actually, we don't need to check error here b'cause the reduce phase does that
	if(this_param_left.error != 0){
	dp->error = this_param_left.error;
	return dp->error;
	}

	if(this_param_right.error != 0){
	dp->error = this_param_right.error;
	return dp->error;
	}*/
#ifdef _ADEBUG
	//LOGI(2, "sign: %d, operand1= %f, operand2=%f, operator: %d", t->sign, this_param_left.retv, this_param_right.retv, t->type);	
#endif
	dp->retv = t->sign * doCalculateF(this_param_left.retv, this_param_right.retv, t->type, &(dp->error));
#ifdef _WIN32
	return dp->error;
#else
	return &(dp->error);
#endif
}

#ifdef _WIN32
unsigned int __stdcall nmath::derivative(void *p){
	HANDLE tdu = 0, tdv = 0;
#else
void* nmath::derivative(void *p){
	pthread_t tdu, tdv;
	int id_du = -1, id_dv = -1;
#endif
	DParam *dp = (DParam*)p;
	NMAST *t = dp->t;
	char x = dp->variables[0];
	NMAST *u, *du, *v, *dv;
	DParam pdu, pdv;

	dp->returnValue = NULL;
	if (t == NULL){
		return 0;
	}

	if (t->type == NUMBER || t->type == PI_TYPE || t->type == E_TYPE){
		u = (NMAST*)malloc(sizeof(NMAST));
		u->sign = 1;
		u->type = NUMBER;
		u->value = 0.0;
		u->parent = NULL;
		u->left = u->right = NULL;
		u->variable = 0;
		dp->returnValue = u;
#ifdef _WIN32
		return 0;
#else
		return u;
#endif
	}

	/*
	IMPORTANT:
	In case of multi-variable function, we need to tell which variable that we are
	getting derivative of
	*/
	if (t->type == VARIABLE){
		u = (NMAST*)malloc(sizeof(NMAST));
		u->type = NUMBER;
		u->sign = 1;
		u->value = 1.0;
		u->parent = NULL;
		u->left = u->right = NULL;
		u->variable = 0;
		if (dp->variables[0] == t->variable){
			u->value = 1.0;
			dp->returnValue = u;
#ifdef _WIN32
			return 0;
#else
			return u;
#endif
		}
		u->value = 0.0;
		
#ifdef _WIN32
		return 0;
#else
		return u;
#endif
}

	dv = du = NULL;

	u = t->left;
	v = t->right;
#ifdef _WIN32
	if (u != NULL){
		pdu.t = t->left;
		pdu.variables[0] = x;
		tdu = (HANDLE)_beginthreadex(NULL, 0, &derivative, (void*)&pdu, 0, NULL);
	}

	if (v != NULL){
		pdv.t = t->right;
		pdv.variables[0] = x;
		tdv = (HANDLE)_beginthreadex(NULL, 0, &derivative, (void*)&pdv, 0, NULL);
	}

	if (tdu != 0){
		WaitForSingleObject(tdu, INFINITE);
		du = pdu.returnValue;
		CloseHandle(tdu);
	}
	if (tdv != 0){
		WaitForSingleObject(tdv, INFINITE);
		dv = pdv.returnValue;
		CloseHandle(tdv);
	}

	switch (t->type){
	case SIN:
		dp->returnValue = d_sin(t, u, du, v, dv, x);
		return 0;

	case COS:
		dp->returnValue = d_cos(t, u, du, v, dv, x);
		return 0;

	case TAN:
		dp->returnValue = d_tan(t, u, du, v, dv, x);
		return 0;

	case COTAN:
		dp->returnValue = d_cotan(t, u, du, v, dv, x);
		return 0;

	case ASIN:
		dp->returnValue = d_asin(t, u, du, v, dv, x);
		return 0;

	case ACOS:
		dp->returnValue = d_acos(t, u, du, v, dv, x);
		return 0;

	case ATAN:
		dp->returnValue = d_atan(t, u, du, v, dv, x);
		return 0;

	case SQRT:
		dp->returnValue = d_sqrt(t, u, du, v, dv, x);
		return 0;

	case PLUS:
	case MINUS:
		dp->returnValue = d_sum_subtract(t, t->type, u, du, v, dv, x);
		return 0;

	case MULTIPLY:
		dp->returnValue = d_product(t, u, du, v, dv, x);
		return 0;

	case DIVIDE:
		dp->returnValue = d_quotient(t, u, du, v, dv, x);
		return 0;

	case POWER:
		dp->returnValue = d_pow_exp(t, u, du, v, dv, x);
		return 0;
	}
	dp->returnValue = NULL;
	return 0;
#else
	if (u != NULL) {
		pdu.t = t->left;
		pdu.variables[0] = x;
		id_du = pthread_create(&tdu, NULL, derivative, (void*)(&pdu));
	}

	if (v != NULL){
		pdv.t = t->right;
		pdv.variables[0] = x;
		id_dv = pthread_create(&tdv, NULL, derivative, (void*)(&pdv));
	}
	if (id_du == 0)
		pthread_join(tdu, (void**)&du);
	if (id_dv == 0)
		pthread_join(tdv, (void**)&dv);

	/****************************************************************/
	// 2.0 get done here	
	switch (t->type){
	case SIN:
		dp->returnValue = d_sin(t, u, du, v, dv, x);
		return dp->returnValue;

	case COS:
		dp->returnValue = d_cos(t, u, du, v, dv, x);
		return dp->returnValue;

	case TAN:
		dp->returnValue = d_tan(t, u, du, v, dv, dp->variables[0]);
		return dp->returnValue;

	case COTAN:
		dp->returnValue = d_cotan(t, u, du, v, dv, dp->variables[0]);
		return dp->returnValue;

	case ASIN:
		dp->returnValue = d_asin(t, u, du, v, dv, dp->variables[0]);
		return dp->returnValue;

	case ACOS:
		dp->returnValue = d_acos(t, u, du, v, dv, dp->variables[0]);
		return dp->returnValue;

	case ATAN:
		dp->returnValue = d_atan(t, u, du, v, dv, dp->variables[0]);
		return dp->returnValue;

	case SQRT:
		dp->returnValue = d_sqrt(t, u, du, v, dv, x);
		return dp->returnValue;

	case PLUS:
	case MINUS:
		dp->returnValue = d_sum_subtract(t, t->type, u, du, v, dv, x);
		return dp->returnValue;

	case MULTIPLY:
		dp->returnValue = d_product(t, u, du, v, dv, x);
		return dp->returnValue;

	case DIVIDE:
		dp->returnValue = d_quotient(t, u, du, v, dv, x);
		return dp->returnValue;

	case POWER:
		dp->returnValue = d_pow_exp(t, u, du, v, dv, x);
		return dp->returnValue;
	}
	/* WHERE du AND dv GO IF WE NOT TO USE THEM ????? */
	return NULL;
#endif
}

/* (u.v) = u'v + uv' */
NMAST* nmath::d_product(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r = NULL;
	if (dv == NULL && du != NULL) {
		if(du->type == NUMBER && du->value == 0.0) {
			r = du;			
		} else {
			r = (NMAST *)malloc(sizeof(NMAST));
			r->sign = 1;
			r->variable = 0;
			r->type = MULTIPLY;
			r->parent = NULL;
			r->left = du;
			du->parent = r;
			r->right = cloneTree(v, r);
		}
		return r;
	}
	
	if(dv != NULL && du == NULL) {
		if(dv->type==NUMBER && dv->value == 0.0) {
			r = dv;
		} else {
			r = (NMAST *)malloc(sizeof(NMAST));
			r->sign = 1;
			r->variable = 0;
			r->type = MULTIPLY;
			r->left = cloneTree(u, r);
			r->right = dv;
			dv->parent = r;
		}
		
		return r;
	}
	
	r = (NMAST *)malloc(sizeof(NMAST));
	r->sign = 1;
	r->variable = 0;
	r->type = PLUS;
	r->parent = NULL;
		
	
	if(dv->type == NUMBER && dv->value == 0.0) {
		r->left = dv;
		dv->parent = r;
	} else {
		r->left = (NMAST *)malloc(sizeof(NMAST));
		r->left->sign = 1;
		r->left->variable = 0;
		r->left->type = MULTIPLY;
		r->left->parent = r;
		r->left->left = cloneTree(u, r->left);
		r->left->right = dv;
		dv->parent = r->left;
	}
	
	if(du->type==NUMBER && du->value == 0.0) {
		r->right = du;
		du->parent = r;
	}else {
		r->right = (NMAST *)malloc(sizeof(NMAST));
		r->right->sign = 1;
		r->right->variable = 0;
		r->right->type = MULTIPLY;
		r->right->parent = r;
		r->right->left = du;
		du->parent = r->right;
		r->right->right = cloneTree(v, r->right);
	}
	
	return r;
}

/* (sin(v))' = cos(v)*dv */
NMAST* nmath::d_sin(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r;
	/* (cos(v))' = -sin(v)*dv */
	r = (NMAST *)malloc(sizeof(NMAST));
	r->type = MULTIPLY;
	r->sign = 1;
	r->parent = NULL;

	r->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->type = COS;
	r->left->sign = 1;
	r->left->parent = r;
	r->left->left = NULL;
	r->left->right = cloneTree(v, r);

	r->right = dv;
	if (dv != NULL)
		dv->parent = r;

	return r;
}

/* (cos(v))' = -sin(v)dv */
NMAST* nmath::d_cos(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r;
	/* (cos(v))' = -sin(v)*dv */
	r = (NMAST *)malloc(sizeof(NMAST));
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
	r->type = MULTIPLY;
	r->sign = 1;
	r->parent = NULL;

	r->left = (NMAST *)malloc(sizeof(NMAST));
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
	r->left->type = SIN; /* <== negative here */
	r->left->sign = -1;
	r->left->parent = r;
	r->left->left = NULL;
	r->left->right = cloneTree(v, r);

	r->right = dv;
	if (dv != NULL)
		dv->parent = r;

	return r;
}

/* tan(v)' =  (sec(v)^2)*dv  */
NMAST* nmath::d_tan(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r;

	r = (NMAST *)malloc(sizeof(NMAST));
	r->type = MULTIPLY;
	r->sign = 1;
	r->parent = NULL;

	r->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->type = POWER;
	r->left->sign = 1;
	r->left->parent = r;

	r->left->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->left->parent = r->left;
	r->left->left->type = SEC;

	r->left->left->left = NULL;
	r->left->left->right = cloneTree(v, r->left->left);

	r->left->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->parent = r->left;
	r->left->right->type = NUMBER;
	r->left->right->value = 2;
	r->left->right->sign = 1;

	r->right = dv;
	if (dv != NULL)
		dv->parent = r;

	return r;
}

/* cotan(v)' = -(1 -sqrt(cotan(v))) * dv  */
NMAST* nmath::d_cotan(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r;

	r = (NMAST *)malloc(sizeof(NMAST));
	r->type = MULTIPLY;
	r->sign = 1;
	r->parent = NULL;

	r->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->type = PLUS;
	r->left->value = 1.0;
	r->left->valueType = TYPE_FLOATING_POINT;
	//r->left->frValue.numerator = 1;
	//r->left->frValue.denomerator = 1;
	r->left->sign = -1;
	r->left->parent = r;

	r->left->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->left->type = NUMBER;
	r->left->left->value = 1.0;
	r->left->left->valueType = TYPE_FLOATING_POINT;
	//r->left->left->frValue.numerator = 1;
	//r->left->left->frValue.denomerator = 1;
	r->left->left->sign = 1;
	r->left->left->parent = r->left;

	r->left->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->type = SQRT;
	r->left->right->value = 1.0;
	r->left->right->valueType = TYPE_FLOATING_POINT;
	//r->left->right->frValue.numerator = 1;
	//r->left->right->frValue.denomerator = 1;
	r->left->right->sign = 1;
	r->left->right->parent = r->left;

	r->left->right->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->left->type = COTAN;
	r->left->right->left->value = 1.0;
	r->left->right->left->valueType = TYPE_FLOATING_POINT;
	//r->left->right->left->frValue.numerator = 1;
	//r->left->right->left->frValue.denomerator = 1;
	r->left->right->left->sign = 1;
	r->left->right->left->parent = r->left->right;

	r->left->right->left->left = cloneTree(v, r->left->right->left);

	r->right = dv;
	if (dv != NULL)
		dv->parent = r;

	return r;
}

/* arcsin(v)' = (1/sqrt(1-v^2))*dv */
NMAST* nmath::d_asin(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r;
	r = (NMAST *)malloc(sizeof(NMAST));
	r->type = MULTIPLY;
	r->sign = 1;
	r->parent = NULL;

	r->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->type = DIVIDE;
	r->left->sign = 1;
	r->left->parent = r;

	r->left->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->left->type = NUMBER;
	r->left->left->value = 1;
	r->left->left->sign = 1;
	r->left->left->parent = r->left;
	r->left->left->left = r->left->left->right = NULL;

	/* sqrt(...) */
	r->left->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->type = SQRT;
	r->left->right->parent = r->left;
	r->left->right->left = NULL;

	r->left->right->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->right->type = MINUS;
	r->left->right->right->parent = r->left->right;

	r->left->right->right->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->right->left->type = NUMBER;
	r->left->right->right->left->value = 1;
	r->left->right->right->left->sign = 1;
	r->left->right->right->left->parent = r->left->right->right;
	r->left->right->right->left->left = r->left->right->right->left->right = NULL;

	r->left->right->right->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->right->right->type = POWER;
	r->left->right->right->right->value = 0;
	r->left->right->right->right->sign = 1;
	r->left->right->right->right->parent = r->left->right->right;

	r->left->right->right->right->left = cloneTree(v, r->left->right->right->right);

	r->left->right->right->right->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->right->right->right->type = NUMBER;
	r->left->right->right->right->right->value = 2;
	r->left->right->right->right->right->sign = 1;
	r->left->right->right->right->right->parent = r->left->right->right->right;
	r->left->right->right->right->right->left = r->left->right->right->right->right->right = NULL;

	r->right = dv;
	if (dv != NULL)
		dv->parent = r;
	return r;
}

/* arccos(v)' = (-1/sqrt(1-v^2))*dv */
NMAST* nmath::d_acos(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r;
	r = (NMAST *)malloc(sizeof(NMAST));
	r->type = MULTIPLY;
	r->sign = 1;
	r->parent = NULL;

	r->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->type = DIVIDE;
	r->left->sign = 1;
	r->left->parent = r;

	r->left->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->left->type = NUMBER;
	r->left->left->value = -1;
	r->left->left->sign = 1;
	r->left->left->parent = r->left;
	r->left->left->left = r->left->left->right = NULL;

	/* sqrt(...) */
	r->left->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->type = SQRT;
	r->left->right->parent = r->left;
	r->left->right->left = NULL;

	r->left->right->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->right->type = MINUS;
	r->left->right->right->parent = r->left->right;

	r->left->right->right->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->right->left->type = NUMBER;
	r->left->right->right->left->value = 1;
	r->left->right->right->left->sign = 1;
	r->left->right->right->left->parent = r->left->right->right;
	r->left->right->right->left->left = r->left->right->right->left->right = NULL;

	r->left->right->right->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->right->right->type = POWER;
	r->left->right->right->right->value = 0;
	r->left->right->right->right->sign = 1;
	r->left->right->right->right->parent = r->left->right->right;

	r->left->right->right->right->left = cloneTree(v, r->left->right->right->right);

	r->left->right->right->right->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->right->right->right->type = NUMBER;
	r->left->right->right->right->right->value = 2;
	r->left->right->right->right->right->sign = 1;
	r->left->right->right->right->right->parent = r->left->right->right->right;
	r->left->right->right->right->right->left = r->left->right->right->right->right->right = NULL;

	r->right = dv;
	if (dv != NULL)
		dv->parent = r;
	return r;
}

/* arctan(v)' = (1/(v^2+1))*dv */
NMAST* nmath::d_atan(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r;
	r = (NMAST *)malloc(sizeof(NMAST));
	r->type = MULTIPLY;
	r->sign = 1;
	r->parent = NULL;

	r->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->type = DIVIDE;
	r->left->sign = 1;
	r->left->parent = r;

	r->left->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->left->type = NUMBER;
	r->left->left->value = 1;
	r->left->left->sign = 1;
	r->left->left->parent = r->left;
	r->left->left->left = r->left->left->right = NULL;

	/* (v^2+1) */
	r->left->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->type = PLUS;
	r->left->right->parent = r->left;

	r->left->right->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->left->type = POWER;
	r->left->right->left->value = 0;
	r->left->right->left->sign = 1;
	r->left->right->left->parent = r->left->right;
	r->left->right->left->left = cloneTree(v, r->left->right->left);

	r->left->right->left->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->left->right->type = NUMBER;
	r->left->right->left->right->value = 2;
	r->left->right->left->right->sign = 1;
	r->left->right->left->right->parent = r->left->right->left;
	r->left->right->left->right->left = r->left->right->left->right->right = NULL;

	r->left->right->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->right->type = NUMBER;
	r->left->right->right->value = 1;
	r->left->right->right->sign = 1;
	r->left->right->right->parent = r->left->right;
	r->left->right->right->left = r->left->right->right->right = NULL;

	r->right = dv;
	if (dv != NULL)
		dv->parent = r;
	return r;
}

/*
* (u/v)' = (u'v - uv')/v^2
* */
NMAST* nmath::d_quotient(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r;

	r = (NMAST*)malloc(sizeof(NMAST));
	r->variable = 0;
	r->type = DIVIDE;
	r->value = 0;
	r->sign = 1;
	r->parent = NULL;

	r->left = (NMAST*)malloc(sizeof(NMAST));
	r->left->variable = 0;
	(r->left)->parent = r;
	(r->left)->type = MINUS;
	(r->left)->value = 0;
	(r->left)->sign = 1;

	/* ========================================== */
	(r->left)->left = (NMAST*)malloc(sizeof(NMAST));
	r->left->left->variable = 0;
	(r->left)->left->type = MULTIPLY;
	(r->left)->left->sign = 1;
	(r->left)->left->parent = r->right;
	((r->left)->left)->left = du;
	if (du != NULL)
		du->parent = (r->left)->left;
	((r->left)->left)->right = cloneTree(v, (r->left)->left);

	/* =================================================== */

	(r->left)->right = (NMAST*)malloc(sizeof(NMAST));
	(r->left)->right->type = MULTIPLY;
	(r->left)->right->sign = 1;
	(r->left)->right->parent = r->right;
	((r->left)->right)->left = cloneTree(u, (r->left)->right);
	((r->left)->right)->right = dv;
	if (dv != NULL)
		dv->parent = (r->left)->right;

	/* ==================================================== */

	r->right = (NMAST*)malloc(sizeof(NMAST));
	(r->right)->parent = r;
	(r->right)->type = POWER;
	r->value = 0;
	r->sign = 1;

	(r->right)->left = cloneTree(v, r->right);

	(r->right)->right = (NMAST*)malloc(sizeof(NMAST));
	(r->right)->right->type = NUMBER;
	(r->right)->right->value = 2;
	(r->right)->right->sign = 1;
	(r->right)->right->parent = r->right;
	((r->right)->right)->left = ((r->right)->right)->right = NULL;
	return r;
} //2.0 got here

/*
* (u +- v) = u' +- v'
* */
NMAST* nmath::d_sum_subtract(NMAST *t, int type, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r;

	if(dv == 0 && du != 0) {
		return du;
	}

	if(dv != 0 && du == 0)
		return dv;

	r = (NMAST *)malloc(sizeof(NMAST));
	r->sign = 1;
	r->type = type;
	r->value = 0.0;
	r->parent = NULL;

	r->left = du;
	if (du != NULL)
		du->parent = r;

	r->right = dv;
	if (dv != NULL)
		dv->parent = r;

	return r;
}


NMAST* nmath::d_pow_exp(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r;
	int isXLeft = isContainVar(u, x);
	int isXRight = isContainVar(v, x);

	/* power: (u^a)' = au^(a-1)*u' */
	if (isXLeft != 0 && isXRight == 0){
		r = (NMAST *)malloc(sizeof(NMAST));
		r->sign = 1;
		r->type = MULTIPLY;
		r->value = 0.0;
		r->parent = NULL;

		/* ===================================================== */
		r->left = (NMAST *)malloc(sizeof(NMAST));
		(r->left)->parent = r;
		(r->left)->type = MULTIPLY;

		(r->left)->left = (NMAST *)malloc(sizeof(NMAST));
		((r->left)->left)->type = NUMBER;
		((r->left)->left)->value = v->value;
		((r->left)->left)->sign = v->sign;
		((r->left)->left)->parent = (r->left);
		((r->left)->left)->left = ((r->left)->left)->right = NULL;

		/*printf(" Add Left Value 1/(v-1) \n");*/

		(r->left)->right = (NMAST *)malloc(sizeof(NMAST));
		((r->left)->right)->type = POWER;
		((r->left)->right)->value = 0.0;
		((r->left)->right)->sign = 1;
		((r->left)->right)->parent = (r->left);

		/*printf(" Add Right POWER \n");*/

		((r->left)->right)->left = cloneTree(u, ((r->left)->right));

		/*printf(" Add LEFT Copy U \n");*/

		((r->left)->right)->right = cloneTree(v, ((r->left)->right));

		(((r->left)->right)->right)->value = v->value - 1;
		/*printf(" Add Right v-1 \n");*/

		r->right = du;
		if (du != NULL) /* <-- is in need? */
			du->parent = r;
		/*printf(" Add Right du \n");*/

		return r;
	}

	/* power: (a^v)' = ln(a)*a^v*v' */
	if (isXLeft == 0 && isXRight != 0){
		r = (NMAST *)malloc(sizeof(NMAST));
		r->type = MULTIPLY;
		r->value = 0.0;
		r->parent = NULL;

		/* ===================================================== */
		r->left = (NMAST *)malloc(sizeof(NMAST));
		(r->left)->parent = r;
		(r->left)->type = MULTIPLY;

		(r->left)->left = (NMAST *)malloc(sizeof(NMAST));
		((r->left)->left)->type = LN;
		((r->left)->left)->value = 0;
		((r->left)->left)->sign = 1;
		((r->left)->left)->parent = r->left;
		((r->left)->left)->left = NULL;
		((r->left)->left)->right = cloneTree(u, (r->left)->left);

		(r->left)->right = (NMAST *)malloc(sizeof(NMAST));
		((r->left)->right)->type = POWER;
		((r->left)->right)->value = 0;
		((r->left)->right)->sign = 1;
		((r->left)->right)->parent = r->left;
		((r->left)->right)->left = cloneTree(u, (r->left)->right);
		((r->left)->right)->right = cloneTree(v, (r->left)->right);

		/* ===================================================== */
		r->right = dv;
		if (dv != NULL)
			dv->parent = r;

		return r;
	}

	/* power: (u^v)' = (dv*ln(u) + v(du/u))*u^v */
	if (isXLeft != 0 && isXRight != 0){
		r = (NMAST *)malloc(sizeof(NMAST));
		r->type = MULTIPLY;
		r->value = 0.0;
		r->parent = NULL;

		/* ===================================================== */
		r->left = (NMAST *)malloc(sizeof(NMAST));
		(r->left)->parent = r;
		(r->left)->type = PLUS;

		(r->left)->left = (NMAST *)malloc(sizeof(NMAST));
		((r->left)->left)->type = MULTIPLY;
		((r->left)->left)->value = 0;
		((r->left)->left)->sign = 1;
		((r->left)->left)->parent = r->left;

		((r->left)->left)->left = dv;
		if (dv != NULL)
			dv->parent = (r->left)->left;

		((r->left)->left)->right = (NMAST *)malloc(sizeof(NMAST));
		((r->left)->left)->right->type = LN;
		((r->left)->left)->right->left = NULL;
		((r->left)->left)->right->right = cloneTree(u, ((r->left)->left)->right);

		(r->left)->right = (NMAST *)malloc(sizeof(NMAST));
		((r->left)->right)->type = MULTIPLY;
		((r->left)->right)->value = 0;
		((r->left)->right)->sign = 1;
		((r->left)->right)->parent = r->left;
		(r->left)->right->left = cloneTree(v, (r->left)->right);
		(r->left)->right->right = (NMAST *)malloc(sizeof(NMAST));
		(r->left)->right->right->type = DIVIDE;
		(r->left)->right->right->value = 0;
		(r->left)->right->right->left = du;
		if (du != NULL)
			du->parent = (r->left)->right->right;
		(r->left)->right->right->right = cloneTree(u, (r->left)->right->right);

		/* ===================================================== */
		r->right = (NMAST *)malloc(sizeof(NMAST));
		r->right->type = POWER;
		r->right->left = cloneTree(u, r->right);
		r->right->right = cloneTree(v, r->right);

		return r;
	}

	return NULL;
}


/* (sqrt(v))' = dv/(2*sqrt(v)) */
NMAST* nmath::d_sqrt(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r;

	r = (NMAST *)malloc(sizeof(NMAST));
	r->type = DIVIDE;
	r->sign = 1;
	r->parent = NULL;

	r->left = dv;
	if (dv != NULL)
		dv->parent = r;

	/*Right child: product operator */
	r->right = (NMAST *)malloc(sizeof(NMAST));
	r->right->value = 0;
	r->right->sign = 1;
	r->right->type = MULTIPLY;
	r->right->parent = r;

	r->right->left = (NMAST *)malloc(sizeof(NMAST));
	r->right->left->type = NUMBER;
	r->right->left->value = 2;
	r->right->left->sign = 1;
	r->right->left->valueType = TYPE_FLOATING_POINT;
	r->right->left->parent = r->right;
	r->right->left->right = r->right->left->left = NULL;

	r->right->right = cloneTree(t, r->right);
	return r;
}
