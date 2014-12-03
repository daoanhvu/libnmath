#include <cstdlib>
#include "compositecriteria.h"

using namespace nmath;

CompositeCriteria::CompositeCriteria(){
	cType = COMPOSITE;
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

Criteria* CompositeCriteria::clone() {
	int i;
	CompositeCriteria* out = new CompositeCriteria();
	out->setOperator(this->logicOp);
	for (i = 0; i < mSize; i++) {
		out->add(list[i]->clone());
	}
	return out;
}

void CompositeCriteria::add(Criteria* c) {
	Criteria **buffer;
	int newLogSize;
	if ((mLoggedSize <= 0) || (mLoggedSize <= mSize)){
		newLogSize = mLoggedSize + 10;
		buffer = (Criteria**)realloc(list, sizeof(Criteria*) * newLogSize);
		if (buffer != NULL) {
			mLoggedSize = newLogSize;
		}
	}
	list[mSize++] = c;
}

Criteria* CompositeCriteria::operator [](int index) const {
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
	if (logicOp == AND){
		out = (CompositeCriteria*)clone();
		out->add(c.clone());
	}
	else {
		out = new CompositeCriteria();
		out->setOperator(AND);
		for (i = 0; i<size(); i++) {
			tmp = (*list[i]) & (Criteria&)c;
			if (tmp != NULL) {
				out->add((CombinedCriteria*)tmp);
			}
		}
	}
	return out;
}

CompositeCriteria* CompositeCriteria::and(CompositeCriteria& c) {
	CompositeCriteria* out;
	Criteria *tmp;
	int i, j, size;

	if (c.logicOperator() == AND) {
		if (logicOp == AND) {
			size = c.size();
			out = (CompositeCriteria*)this->clone();
			for (i = 0; i<size; i++) {
				out->add(c[i]->clone());
			}
		}
		else {
			//AND and OR
			out = new CompositeCriteria();
			out->setOperator(OR);
			for (i = 0; i<mSize; i++) {
				tmp = (*list[i]) & c;
				if (tmp != NULL) {
					out->add(tmp);
				}
			}
		}
	}
	else {
		if (logicOp == AND) { // OR & AND
			out = new CompositeCriteria();
			out->setOperator(OR);
			for (i = 0; i<c.size(); i++) {
				tmp = (*this) & (*c[i]);
				if (tmp != NULL) {
					out->add(tmp);
				}
			}
		}
		else { //OR & OR
			size = c.size();
			out = (CompositeCriteria*)this->clone();
			for (i = 0; i<size; i++) {
				out->add(c[i]->clone());
			}
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

		case COMPOSITE:
			out = this->and((CompositeCriteria&)c);
			break;
	}

	return out;
}

CompositeCriteria* CompositeCriteria::or(SimpleCriteria& c) {
	CompositeCriteria* out;

	if (this->logicOp == AND) {
		//AND | Simple = OR (AND, Simple)
		out = new CompositeCriteria();
		out->setOperator(OR);
		out->add(clone());
		out->add(c.clone());
	}
	else {
		// OR | Simple = add Simple into this->clone()
		out = (CompositeCriteria*)clone();
		out->add(c.clone());
	}

	return out;
}

CompositeCriteria* CompositeCriteria::or(CompositeCriteria& c) {
	int i;
	CompositeCriteria *out;
	if (this->logicOp == AND) {
		if (c.logicOperator() == AND) {
			// AND | AND = OR (this->clone, c.clone())
			out = new CompositeCriteria();
			out->setOperator(OR);
			out->add(this->clone());
			out->add(c.clone());
		}
		else {
			// AND | OR
			out = (CompositeCriteria*)c.clone();
			out->add(this->clone());
		}
	}
	else {
		if (c.logicOperator() == AND) {
			out = (CompositeCriteria*)clone();
			out->add(c.clone());
		}
		else {
			out = (CompositeCriteria*)clone();
			for (i = 0; i < c.size(); i++){
				out->add(c[i]->clone());
			}
		}
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
