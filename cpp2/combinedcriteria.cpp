#include <stdlib.h>
#include "combinedcriteria.h"
#include "compositecriteria.h"

using namespace nmath;

CombinedCriteria::CombinedCriteria(): list(0), mLoggedSize(0), mSize(0) {
}

CombinedCriteria::~CombinedCriteria() {
	int i;
	if (mSize <= 0) return;

	for (i = 0; i < mSize; i++)
		delete list[i];

	free(list);
}

void CombinedCriteria::release() {
	int i;
	if (mSize <= 0) return;

	for (i = 0; i < mSize; i++)
		delete list[i];

	free(list);
	list = 0;
	mSize = 0;
	mLoggedSize = 0;
}

void CombinedCriteria::add(SimpleCriteria* c) {
	SimpleCriteria **buffer;
	int newLogSize;
	if ( (mLoggedSize<=0) || (mLoggedSize <= mSize)){
		newLogSize = mLoggedSize + 10;
		buffer = (SimpleCriteria**)realloc(list, sizeof(SimpleCriteria*) * newLogSize);
		if (buffer != NULL) {
			mLoggedSize = newLogSize;
		}
	}
	list[mSize++] = c;
}

SimpleCriteria* CombinedCriteria::operator [](int index) const {
	if (index >=0 && index<mSize)
		return list[index];

	return (SimpleCriteria*)0;
}

SimpleCriteria* CombinedCriteria::remove(int index) {
	int i;
	if (index >= 0 && index < mSize) {
		for (i = index; i < mSize-1; i++)
			list[i] = list[i + 1];
		mSize--;
		list[mSize] = (SimpleCriteria*)0;
		return list[index];
	}

	return (SimpleCriteria*)0;
}

CombinedCriteria& CombinedCriteria::operator =(const CombinedCriteria& src) {
	int i;

	this->release();

	list = (SimpleCriteria**)malloc(sizeof(SimpleCriteria*) * src.loggedSize());
	mSize = src.size();
	mLoggedSize = src.loggedSize();
	for(i=0; i<mSize; i++) {
		list[i] = new SimpleCriteria(src.list[i]->getType(), 
											src.list[i]->getVariable(),
											src.list[i]->getLeftValue(),
											src.list[i]->getRightValue(), 
											src.list[i]->isLeftInfinity(),
											src.list[i]->isRightInfinity());
	}
	return *this;
}

CombinedCriteria* CombinedCriteria::clone() {
	int i;

	CombinedCriteria* out = new CombinedCriteria();

	out->list = (SimpleCriteria**)malloc(sizeof(SimpleCriteria*) * mLoggedSize);
	out->mSize = mSize;
	out->mLoggedSize = mLoggedSize;

	for (i = 0; i<mSize; i++) {
		out->list[i] = new SimpleCriteria(list[i]->getType(),
			list[i]->getVariable(),
			list[i]->getLeftValue(),
			list[i]->getRightValue(),
			list[i]->isLeftInfinity(),
			list[i]->isRightInfinity());
	}

	return out;
}

Criteria* CombinedCriteria::and(SimpleCriteria& c) {
	Criteria *tmp;
	CombinedCriteria* out;
	int i = 0;
	for(i=0; i<mSize; i++){
		if( c.getVariable() == list[i]->getVariable() ){
			tmp = c.and(*(list[i]));
			if (tmp != NULL) {
				return tmp;
			}else{
				/** ERROR: AND two contracting criteria */
				return 0;
			}
			break;
		}
		i++;
	}

	/*	
		We got here because c has variable that not same as vaiable of any criteria in CombinedCriteria
	*/
	out = this->clone();
	((CombinedCriteria*)out)->add(c.clone());
	return out;
}

Criteria* CombinedCriteria::and(CombinedCriteria& c) {
	int i, size = c.size();
	Criteria* cb;
	CombinedCriteria *out = new CombinedCriteria();
	for(i=0; i<size; i++) {
		cb = this->and(*c[i]);
		if (cb != 0){
			
		}
	}
	return out;
}

Criteria* CombinedCriteria::and(CompositeCriteria& c) {
	Criteria* tmp;
	CompositeCriteria* out = new CompositeCriteria();
	int i;

	for(i=0; i<c.size(); i++) {
		tmp = this->and(*c[i]);
		if (tmp != NULL){
			out->add((CombinedCriteria*)tmp);
		}
	}
	return out;
}

Criteria* CombinedCriteria::operator &(Criteria& c) {
	Criteria* out = 0;
	switch (c.getCClassType()) {
		case SIMPLE:
			out = and((SimpleCriteria&)c);
			break;

		case COMBINED:
			out = and((CombinedCriteria&)c);
			break;

		case COMPOSITE:
			out = and((CompositeCriteria&)c);
			break;
	}

	return out;
}


Criteria* CombinedCriteria::or(SimpleCriteria& c) {
	CompositeCriteria* out = new CompositeCriteria();
	out->add(clone());

	CombinedCriteria * temp = new CombinedCriteria();
	temp->add(c.clone());

	out->add(temp);

	return out;
}

Criteria* CombinedCriteria::or(CombinedCriteria& c) {
	CompositeCriteria* out = new CompositeCriteria();
	out->add(clone());
	out->add(c.clone());
	return out;
}

CompositeCriteria* CombinedCriteria::or(CompositeCriteria& c) {
	CompositeCriteria* out = c.clone();
	out->add(clone());

	return out;
}

Criteria* CombinedCriteria::operator |(Criteria& c) {
	Criteria* out = 0;
	switch (c.getCClassType()) {
	case SIMPLE:
		out = or((SimpleCriteria&)c);
		break;

	case COMBINED:
		out = or((CombinedCriteria&)c);
		break;

	case COMPOSITE:
		out = or((CompositeCriteria&)c);
		break;
	}

	return out;
}

/**
	Combine (AND) this criteria with each pair of value in bounds
*/
CombinedCriteria* CombinedCriteria::getInterval(const float *bounds, int varCount) {	
	SimpleCriteria *interval;
	CombinedCriteria* outListInterval = new CombinedCriteria();
	int i;
	
	for(i = 0; i < varCount; i++){
		interval = new SimpleCriteria(GT_LT, 'x', 0, 0, FALSE, FALSE);
		interval = list[i]->and(bounds + i*2);
		if( interval == NULL) {
			delete outListInterval;
			return 0;
		}
		outListInterval->add(interval);
	}

	return outListInterval;
}

bool CombinedCriteria::check(const double *values) {
	int i;
	
	for(i=0; i<mSize; i++) {
		if( list[i]->check(values+i) )
			return false;
	}
	
	return true;
}