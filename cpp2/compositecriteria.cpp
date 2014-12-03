#include "compositecriteria.h"

using namespace nmath;

CompositeCriteria::CompositeCriteria(){
	list = 0;
	mLoggedSize = 0;
	mSize = 0;
}

CompositeCriteria::~CompositeCriteria(){

}

void CompositeCriteria::release() {
	int i;
	if (mSize <= 0) return;

	for (i = 0; i < mSize; i++)
		delete list[i];

	delete[] list;

	list = 0;
	mSize = 0;
	mLoggedSize = 0;
}

CombinedCriteria* CompositeCriteria::operator [](int index) const {
	if(list == NULL || index >= mSize) return 0;

	return list[index];
}

bool CompositeCriteria::check(const float* values) {
	int i;
	
	for(i=0; i<this->size; i++){
		if( list[i]->check(values) == TRUE)
			return TRUE;
	}
	
	return FALSE;
}

CompositeCriteria* CompositeCriteria::and(SimpleCriteria& c) {
	CombinedCriteria *cb;
	Criteria *tmp;
	CompositeCriteria* out = new CompositeCriteria();
	int i;

	for(i=0; i<size(); i++) {
		tmp = list[i]->and(c);
		if( tmp != NULL ) {
			out->add((CombinedCriteria*)tmp);
		}
	}
	return out;
}

CompositeCriteria* CompositeCriteria::and(CombinedCriteria& c) {
	CombinedCriteria* cb;
	comp2 = (CompositeCriteria*)c1;
	CompositeCriteria* out;
	int i;
	outComp->loggedSize = comp2->size;
	outComp->list = (CombinedCriteria**)malloc(sizeof(CombinedCriteria*) * outComp->loggedSize);
	for(i=0; i<mSize; i++) {
		cb = list[i];
		andTwoCriteria(comb1, cb, &outTmp);
		if(outComp->size >= outComp->loggedSize) {
			outComp->loggedSize += 5;
			tmp = realloc(outComp->list, sizeof(CombinedCriteria*) * outComp->loggedSize);
			outComp->list = (tmp==NULL)?outComp->list:((CombinedCriteria**)tmp);
		}
		outComp->list[(outComp->size)++] = (CombinedCriteria*)outTmp.cr;
		outTmp.cr = NULL;
	}
	out->cr = outComp;
	result = TRUE;
}

CompositeCriteria* CompositeCriteria::and(const CompositeCriteria& c) {
	comp1 = (CompositeCriteria*)c1;
	comp2 = (CompositeCriteria*)c2;
	outComp = newCompositeInterval();
	outComp->loggedSize = comp2->size;
	outComp->list = (CombinedCriteria**)malloc(sizeof(CombinedCriteria*) * outComp->loggedSize);
	for(i=0; i<comp2->size; i++) {
		cb = comp2->list[i];
		andTwoCriteria(comp1, cb, &outTmp);
		if(outComp->size >= outComp->loggedSize) {
			outComp->loggedSize += 5;
			tmp = realloc(outComp->list, sizeof(CombinedCriteria*) * outComp->loggedSize);
			outComp->list = (tmp==NULL)?outComp->list:((CombinedCriteria**)tmp);
		}

		outComp->list[(outComp->size)++] = (CombinedCriteria*)outTmp.cr;
		outTmp.cr = NULL;
	}
	out->cr = outComp;
	result = TRUE;
}

/**
	values [IN]
		This is a matrix N rows x 2 columns which N equals varCount
		
	@param outInterval
		This output parameter, it's a matrix N row and M columns which each row is for each continuous space for the expression
		It means that each row will hold a combined-interval for n-tule variables and M equal varCount * 2
*/
void getCompositeInterval(const void *interval, const float *values, int varCount, void *outDomainObj){
	CompositeCriteria *criteria = (CompositeCriteria*)interval;
	CompositeCriteria *outDomain = (CompositeCriteria *)outDomainObj;
	CombinedCriteria *listIn;
	int i;
	
	for(i=0; i<criteria->size; i++) {
		listIn = newCombinedInterval();
		
		(criteria->list[i])->fgetInterval(criteria->list[i], values, varCount, listIn);
		if(listIn->size > 0 ) {
			if(outDomain->size >= outDomain->loggedSize) {
				outDomain->loggedSize += INCLEN;
				outDomain->list = (CombinedCriteria**)realloc(outDomain->list, sizeof(CombinedCriteria*) * outDomain->loggedSize);
			}
			outDomain->list[outDomain->size++] = listIn;
		}else{
			free(listIn);
		}
	}
}
