#include <cstdlib>
#include "SimpleCriteria.h"
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

bool CompositeCriteria::containsVar(char var) {
	int i;
	
	for(i=0; i<mSize; i++) {
		if(list[i]->containsVar(var))
			return true;
	}
	
	return false;
}
#ifdef _WIN32
istream& CompositeCriteria::operator >>(istream& is) {
	return is;
}

ostream& CompositeCriteria::operator <<(ostream& os) {
	int i;

	os << "(";
	if (logicOp == OR) {
		for (i = 0; i < mSize; i++)
			os << list[i] << " OR ";
	}
	else {
		for (i = 0; i < mSize; i++)
			os << list[i] << " AND ";
	}
	
	os << ")\n";

	return os;
}
#endif

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
			list = buffer;
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

Criteria* CompositeCriteria::andSelf(Criteria& c) {

	if (c.getCClassType() == SIMPLE)
		return andSimpleSelf((SimpleCriteria&)c);

	return andCompositeSelf((CompositeCriteria&)c);

}
Criteria* CompositeCriteria::andCompositeSelf(CompositeCriteria& c) {
	CompositeCriteria* out;
	Criteria *tmp;
	int i, size;

	if (c.logicOperator() == AND) {
		if (logicOp == AND) {
			size = c.size();
			for (i = 0; i<size; i++) {
				this->andSelf(*c[i]);
			}
			out = this;
		}
		else {
			//AND and OR
			for (i = 0; i<mSize; i++) {
				tmp = list[i]->andSelf(c);
				if (tmp != NULL) {
					list[i] = tmp;
				}
			}
			out = this;
		}
	}
	else {
		if (logicOp == AND) { // OR & AND
			out = new CompositeCriteria();
			out->setOperator(OR);
			for (i = 0; i<c.size()-1; i++) {
				CompositeCriteria* cc1 = (CompositeCriteria*)clone();
				tmp = cc1->andSelf(*c[i]);
				if (tmp != NULL) {
					out->add(tmp);
				}
			}
			
			tmp = this->andSelf(*c[i]);
			if (tmp != NULL) {
				out->add(tmp);
			}
		}
		else { //OR & OR
			for (i = 0; i<c.size(); i++) {
				this->add(c[i]->clone());
			}
			out = this;
		}
	}
	return out;
}

/*
	Add directly to this object, don't clone it
*/
Criteria* CompositeCriteria::andSimpleSelf(SimpleCriteria& c) {
	int i;
	Criteria * itm;

	if (logicOp == AND){
		for(i=0; i<mSize; i++) {
			if(list[i]->containsVar(c.getVariable())) {
				itm = list[i]->andSelf(c);
				list[i] = itm;
				return this;
			}
		}
		
		add(c.clone());
		return this;
	}
	
	//OR
	for(i=0; i<mSize; i++) {
		itm = list[i]->andSelf(c);
		list[i] = itm;
	}
	
	return this;
}

CompositeCriteria* CompositeCriteria::andCriteria(SimpleCriteria& c) {
	Criteria *tmp;
	CompositeCriteria* out;
	int i;
	if (logicOp == AND){
		out = (CompositeCriteria*)clone();
		out->add(c.clone());
		return out;
	}

	out = new CompositeCriteria();
	out->setOperator(OR);
	for (i = 0; i<mSize; i++) {

		if(list[i]->getCClassType() == SIMPLE) {
		}

		tmp = (*list[i]) & (Criteria&)c;
		if (tmp != NULL) {
			out->add(tmp);
		}
	}
	
	return out;
}

CompositeCriteria* CompositeCriteria::andCriteria(CompositeCriteria& c) {
	CompositeCriteria* out;
	Criteria *tmp;
	int i, size;

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
			out = this->andCriteria((SimpleCriteria&)c);
			break;

		case COMPOSITE:
			out = this->andCriteria((CompositeCriteria&)c);
			break;
	}

	return out;
}

CompositeCriteria* CompositeCriteria::orCriteria(SimpleCriteria& c) {
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

CompositeCriteria* CompositeCriteria::orCriteria(CompositeCriteria& c) {
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
	Criteria* out;
	switch(c.getCClassType()){
		case SIMPLE:
			out = andCriteria((SimpleCriteria&)c);
		break;
		
		case COMPOSITE:
			out = andCriteria((CompositeCriteria&)c);
		break;
	}
	return out;
}

/**
values [IN]
This is a matrix N rows x 2 columns which N equals varCount

@param outInterval
This output parameter, it's a matrix N row and M columns which each row is for each continuous space for the expression
It means that each row will hold a combined-interval for n-tule variables and M equal varCount * 2
*/
/*
Criteria* CompositeCriteria::getIntervalF(const float *values, const char* var, int varCount) {
	Criteria *listIn;
	int i;
	CompositeCriteria *out = new CompositeCriteria();
	out->setOperator(logicOp);

	for (i = 0; i<mSize; i++) {
		listIn = list[i]->getIntervalF(values, var, varCount);
		if (listIn != NULL) {
			out->add(listIn);
		}
	}

	return out;
}

Criteria* CompositeCriteria::getInterval(const double *values, const char* var, int varCount) {
	Criteria *listIn;
	int i;
	CompositeCriteria *out = new CompositeCriteria();
	out->setOperator(logicOp);
	
	for (i = 0; i<mSize; i++) {
		listIn = list[i]->getInterval(values, var, varCount);
		if (listIn != NULL) {
			out->add(listIn);
		}
	}

	return out;
}
*/
CompositeCriteria& CompositeCriteria::normalize(const char* vars, int varcount) {
	bool orFlag = false;
	int k, i;
	CompositeCriteria *ncc;
	SimpleCriteria *sc;

	for (i = 0; i<mSize; i++) {
		for (k = 0; k < varcount; k++) {
			if (!list[i]->containsVar(vars[k])){
				sc = new SimpleCriteria(GTE_LTE, vars[k], 0, 0, true, true);
				ncc = new CompositeCriteria();
				ncc->setOperator(AND);

				ncc->add(list[i]);
				ncc->add(sc);

				list[i] = ncc;
			}
		}
	}

	return *this;
}