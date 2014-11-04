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
void CombinedCriteria::getInterval(const float *bounds, int varCount, CombinedCriteria *outListInterval) {	
	Criteria *interval;
	int i, k;
	
	for(k=0; k<varCount; k++){
		interval = newCriteria(GT_LT, 'x', 0, 0, FALSE, FALSE);
		
		this->list[k]->fgetInterval(criteria->list[k], bounds + k*2, varCount, interval);
		if( (interval->flag & AVAILABLE) != AVAILABLE ){
			free(interval);
			for(i=0; i<outListInterval->size; i++)
				free(outListInterval->list[i]);
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