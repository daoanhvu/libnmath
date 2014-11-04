#include "combinedcriteria.h"

CombinedCriteria::CombinedCriteria() {
	list = NULL;
	loggedSize = 0;
	size = 0;
}

CombinedCriteria::~CombinedCriteria() {
	
}

void CombinedCriteria::operator =(CombinedCriteria& dest, CombinedCriteria& src) {
	int i;
	dest->loggedSize = src->loggedSize;
	dest->size = src->size;
	dest->list = (Criteria**)malloc(sizeof(Criteria*) * dest->loggedSize);
	for(i=0; i<dest->size; i++){
		dest->list[i] = newCriteria(src->list[i]->type, 
											src->list[i]->variable,
											src->list[i]->leftVal,
											src->list[i]->rightVal, 
											(src->list[i]->flag & LEFT_INF) >> 1,
											src->list[i]->flag & RIGHT_INF);
	}
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