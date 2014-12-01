#include "combinedcriteria.h"

CombinedCriteria::CombinedCriteria() {
	list = NULL;
	loggedSize = 0;
	size = 0;
}

CombinedCriteria::~CombinedCriteria() {
	
}

void CombinedCriteria::operator =(CombinedCriteria& dest, const CombinedCriteria& src) {
	int i;
	dest->list = new (Criteria*)[dest->getLoggedSize()];
	for(i=0; i<dest->size; i++){
		dest->list[i] = new Criteria(src->list[i]->type, 
											src->list[i]->variable,
											src->list[i]->leftVal,
											src->list[i]->rightVal, 
											(src->list[i]->flag & LEFT_INF) >> 1,
											src->list[i]->flag & RIGHT_INF);
	}
}

Criteria* CombinedCriteria::operator &(const Criteria& c) {
	CombinedCriteria* out;
	int i = 0, size = v.size();
	for(i=0; i<size; i++){
		if( ((Criteria*)c2)->variable == v[i]->variable ){
			interval = c & v[i];
			if( (interval != NULL) && (interval->flag & AVAILABLE) == AVAILABLE){
				out = new CombinedCriteria();
				out = this;
				out[i] = interval;
				delete interval;
				return out;
			}else{
				/** ERROR: AND two contracting criteria */
				if(interval != NULL) delete interval;
				return NULL;
			}
			break;
		}
		i++;
	}

	/*	
		We got here because c has variable that not same as vaiable of any criteria in CombinedCriteria
	*/
	out = new CombinedCriteria();
	out = this;
	out &= c;
	return out;
}

Criteria* CombinedCriteria::and(const CombinedCriteria& c) {
	int i, size = c->size();
	CombinedCriteria *out;
	out = new CombinedInterval();
	for(i=0; i<size; i++) {
		cb = this & c[i];
		cb->moveListTo(out);
		delete cb;
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