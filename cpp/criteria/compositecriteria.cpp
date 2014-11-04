#include "compositecriteria.h"

CompositeCriteria::CompositeCriteria(){
	list = NULL;
	loggedSize = 0;
	size = 0;
}

CompositeCriteria::~CompositeCriteria(){

}

void CompositeCriteria::release() {
	
}

CombinedCriteria CompositeCriteria::operator [](int index){
	if(list == NULL || index >= size) return NULL;

	return list[index];
}

int CompositeCriteria::isInInterval(const float* values) {
	int i;
	
	for(i=0; i<this->size; i++){
		if( list[i]->isInInterval(values) == TRUE)
			return TRUE;
	}
	
	return FALSE;
}