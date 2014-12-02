#include "combinedcriteria.h"

using namespace nmath;

CombinedCriteria::CombinedCriteria() {
	list = 0;
	mLoggedSize = 0;
	mSize = 0;
}

CombinedCriteria::~CombinedCriteria() {
	
}

void CombinedCriteria::release() {
	int i;
	if (mSize <= 0) return;

	for (i = 0; i < mSize; i++)
		delete list[i];

	delete[] list;

	list = 0;
	mSize = 0;
	mLoggedSize = 0;
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
	list = new (SimpleCriteria**)[src.loggedSize()];
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

	out->list = new (SimpleCriteria**)[mLoggedSize];
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
			tmp = c & *(list[i]);
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
	((CombinedCriteria*)out)->add(c.clone);
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

Criteria* CombinedCriteria::and(const CompositeCriteria& c) {
	Criteria* tmp;

	CompositeCriteria* out = new CompositeCriteria();

	for(i=0; i<c.sise(); i++) {
		tmp = this->and(c[i]);

		if(tmp != NULL)
			out->add(tmp);
	}
	return out;
}

/**
	Combine (AND) this criteria with each pair of value in bounds
*/
CombinedCriteria* CombinedCriteria::getInterval(const float *bounds, int varCount) {	
	Criteria *interval;
	CombinedCriteria* outListInterval = new CombinedCriteria();
	int i, k;
	
	for(k=0; k<varCount; k++){
		interval = new Criteria(GT_LT, 'x', 0, 0, FALSE, FALSE);
		
		interval = list[k]->and(bounds + k*2, varCount);
		if( (interval == NULL) || (interval->flag & AVAILABLE) != AVAILABLE ) {
			for(i=0; i<outListInterval->size; i++)
				delete (outListInterval->list[i]);
			free(outListInterval->list);
			outListInterval->list = NULL;
			outListInterval->size = 0;
			outListInterval->loggedSize = 0;
			return;
		}
		
		if(outListInterval->size >= outListInterval->loggedSize){
			outListInterval->loggedSize += INCLEN;
			outListInterval->list = (Criteria**)realloc(outListInterval->list, sizeof(Criteria*) * outListInterval->loggedSize);
		}
		outListInterval->list[outListInterval->size++] = interval;
	}
}

int CombinedCriteria::isInInterval(const float *values) {
	int i;
	
	for(i=0; i<this->size; i++) {
		if( list[i]->isInInterval(*(values+i)) == FALSE)
			return FALSE;
	}
	
	return TRUE;
}