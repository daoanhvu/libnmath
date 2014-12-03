#include <cstdlib>
#include "compositecriteria.h"

using namespace nmath;

CompositeCriteria::CompositeCriteria(){
	list = 0;
	mLoggedSize = 0;
	mSize = 0;
}

CompositeCriteria::~CompositeCriteria(){
	int i;
	if (mSize <= 0) return;

	for (i = 0; i < mSize; i++)
		delete list[i];

	free(list);
}

void CompositeCriteria::release() {
	int i;
	if (mSize <= 0) return;

	for (i = 0; i < mSize; i++)
		delete list[i];

	free(list);
	list = 0;
	mSize = 0;
	mLoggedSize = 0;
}

CompositeCriteria* CompositeCriteria::clone() {
	CompositeCriteria* out = new CompositeCriteria();
	int i;
	for (i = 0; i < mSize; i++) {
		out->add(list[i]->clone());
	}
	return out;
}

void CompositeCriteria::add(CombinedCriteria* c) {
	CombinedCriteria **buffer;
	int newLogSize;
	if ((mLoggedSize <= 0) || (mLoggedSize <= mSize)){
		newLogSize = mLoggedSize + 10;
		buffer = (CombinedCriteria**)realloc(list, sizeof(CombinedCriteria*) * newLogSize);
		if (buffer != NULL) {
			mLoggedSize = newLogSize;
		}
	}
	list[mSize++] = c;
}

CombinedCriteria* CompositeCriteria::operator [](int index) const {
	if(list == NULL || index >= mSize) return 0;

	return list[index];
}

bool CompositeCriteria::check(const double* values) {
	int i;
	
	for(i=0; i<mSize; i++){
		if( list[i]->check(values) == true)
			return true;
	}
	
	return false;
}

CompositeCriteria* CompositeCriteria::and(SimpleCriteria& c) {
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
	Criteria* cb;
	CompositeCriteria* out;
	int i;
	out = new CompositeCriteria();
	for(i=0; i<mSize; i++) {
		cb = list[i]->and(c);
		if (cb != NULL) {
			out->add((CombinedCriteria*)cb);
		}
	}

	return out;
}

CompositeCriteria* CompositeCriteria::and(CompositeCriteria& c) {
	CompositeCriteria* out;
	Criteria *tmp;
	int i, j;

	out = new CompositeCriteria();
	for(i=0; i<mSize; i++) {
		for (j = 0; j < c.size(); j++){
			tmp = list[i]->and(*c[j]);
			if (tmp != NULL)
				out->add((CombinedCriteria*)tmp);
		}
	}
	return out;
}

Criteria* CompositeCriteria::operator &(Criteria& c) {
	Criteria* out = 0;
	switch(c.getCClassType()) {
		case SIMPLE:
			out = this->and((SimpleCriteria&)c);
			break;

		case COMBINED:
			out = this->and((CombinedCriteria&)c);
			break;

		case COMPOSITE:
			out = this->and((CompositeCriteria&)c);
			break;
	}

	return out;
}

CompositeCriteria* CompositeCriteria::or(SimpleCriteria& c) {
	CombinedCriteria *tmp = new CombinedCriteria();
	tmp->add(c.clone());

	CompositeCriteria* out = clone();
	out->add(tmp);

	return out;
}

CompositeCriteria* CompositeCriteria::or(CombinedCriteria& c) {
	CompositeCriteria* out = clone();
	out->add(c.clone());
	return out;
}

CompositeCriteria* CompositeCriteria::or(CompositeCriteria& c) {
	int i;
	CompositeCriteria* out = clone();
	for (i = 0; i < c.size(); i++){
		out->add(c[i]->clone());
	}

	return out;
}

Criteria* CompositeCriteria::operator |(Criteria& c) {
	return NULL;
}

/**
	values [IN]
		This is a matrix N rows x 2 columns which N equals varCount
		
	@param outInterval
		This output parameter, it's a matrix N row and M columns which each row is for each continuous space for the expression
		It means that each row will hold a combined-interval for n-tule variables and M equal varCount * 2
*/
CompositeCriteria* CompositeCriteria::getInterval(const float *values, int varCount) {
	CompositeCriteria *out = new CompositeCriteria();
	CombinedCriteria *listIn;
	int i;
	
	for(i=0; i<mSize; i++) {
		listIn = list[i]->getInterval(values, varCount);
		if( listIn != NULL ) {
			out->add(listIn);
		}
	}

	return out;
}
