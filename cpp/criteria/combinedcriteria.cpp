#include "combinedcriteria.h"

CombinedCriteria::CombinedCriteria() {
	list = NULL;
	loggedSize = 0;
	size = 0;
}

CombinedCriteria::~CombinedCriteria() {
	
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