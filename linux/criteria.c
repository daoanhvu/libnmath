#include <stdlib.h>
#include "criteria.h"

Criteria *newCriteria(int type, char var, DATA_TYPE_FP lval, DATA_TYPE_FP rval, 
										int leftInfinity, int rightInfinity) {
	Criteria *result = (Criteria *)malloc(sizeof(Criteria));
	result->type = type;
	result->variable = var;
	result->leftVal = lval;
	result->rightVal = rval;
	result->fcheck = isInInterval;
	result->fgetInterval = getInterval;
	result->isLeftInfinity = leftInfinity;
	result->isRightInfinity = rightInfinity;
	return result;
}

CombinedCriteria *newCombinedInterval() {
	CombinedCriteria *result = (CombinedCriteria *)malloc(sizeof(CombinedCriteria));
	result->fcheck = isInCombinedInterval;
	result->fgetInterval = getCombinedInterval;
	result->list = NULL;
	result->loggedSize = 0;
	result->size = 0;
	return result;
}

CompositeCriteria *newCompositeInterval() {
	CompositeCriteria *result = (CompositeCriteria *)malloc(sizeof(CompositeCriteria));
	result->fcheck = isInCompositeInterval;
	result->fgetInterval = getCompositeInterval;
	result->list = NULL;
	result->loggedSize = 0;
	result->size = 0;
	return result;
}

int isInInterval(void *interval, DATA_TYPE_FP *values, int varCount) {
	int result = FALSE;
	Criteria *criteria = (Criteria*)interval;
	if(criteria->isLeftInfinity && criteria->isRightInfinity)
		return (values!=NULL)?TRUE:FALSE;
		
	if(criteria->isLeftInfinity){
		/** HERE we don't need to take care of leftVal */
		switch(criteria->type){
			case GT_LT:
			case GTE_LT:
				// x < rightVal
				if(*values < criteria->rightVal)
					result = TRUE;
			break;
			
			case GT_LTE:
			case GTE_LTE:
				// x <= rightVal
				if(*values <= criteria->rightVal)
					result = TRUE;
			break;
		}
	}else if (criteria->isRightInfinity){
		/** HERE we don't need to take care of rightVal */
		switch(criteria->type){
			case GT_LT:
			case GT_LTE:
				// leftVal < x
				if(criteria->leftVal < (*values))
					result = TRUE;
			break;
			
			case GTE_LT:
			case GTE_LTE:
				// leftVal <= x
				if(criteria->leftVal <= (*values))
					result = TRUE;
			break;
		}
	}else{
		switch(criteria->type){
			case GT_LT:
				// leftVal < x < rightVal
				if(criteria->leftVal < (*values) && (*values < criteria->rightVal))
					result = TRUE;
			break;
			
			case GT_LTE:
				// leftVal < x <= rightVal
				if(criteria->leftVal < (*values) && (*values <= criteria->rightVal))
					result = TRUE;
			break;
			
			case GTE_LT:
				// leftVal <= x < rightVal
				if(criteria->leftVal <= (*values) && (*values < criteria->rightVal))
					result = TRUE;
			break;
			
			case GTE_LTE:
				// leftVal <= x <= rightVal
				if(criteria->leftVal <= (*values) && (*values <= criteria->rightVal))
					result = TRUE;
			break;
		}
	}
	
	return result;
}

//AND Criteria
int isInCombinedInterval(void *interval, DATA_TYPE_FP *values, int varCount) {
	CombinedCriteria *criteria = (CombinedCriteria*)interval;
	int i;
	
	for(i=0; i<criteria->size; i++){
		if((criteria->list[i]->fcheck(criteria->list[i], values+i, 1)) == FALSE)
			return FALSE;
	}
	
	return TRUE;
}


//OR CombinedCriteria
int isInCompositeInterval(void *interval, DATA_TYPE_FP *values, int varCount) {
	CompositeCriteria *criteria = (CompositeCriteria*)interval;
	int i;
	
	for(i=0; i<criteria->size; i++){
		if(((CombinedCriteria*)(criteria->list[i]))->fcheck( criteria->list[i], values+i, 1) == TRUE)
			return TRUE;
	}
	
	return FALSE;
}

void getInterval(void *interval, DATA_TYPE_FP *values, int varCount, void *outIntervalObj){
	Criteria *criteria = (Criteria*)interval;
	Interval *outInterval = (Interval*)outIntervalObj;
	
	if(criteria->isLeftInfinity && criteria->isRightInfinity){
		outInterval->leftVal = values[0];
		outInterval->rightVal = values[1];
		return;
	}
		
	if(criteria->isLeftInfinity){
		/** HERE we don't need to take care of leftVal */
		outInterval->leftVal = values[0];
		switch(criteria->type){
			case GT_LT:
			case GTE_LT:
				// x < rightVal
				if(criteria->rightVal <= values[0]){
					//return empty set
					outInterval->available = FALSE;
					return;
				}
				
				if(values[1] < criteria->rightVal)
					outInterval->rightVal = values[1];
				else 
					outInterval->rightVal = criteria->rightVal;
			break;
			
			case GT_LTE:
			case GTE_LTE:
				// x <= rightVal
				if(criteria->rightVal < values[0]){
					//return empty set
					outInterval->available = FALSE;
					return;
				}
				
				if(values[1] <= criteria->rightVal)
					outInterval->rightVal = values[1];
				else 
					outInterval->rightVal = criteria->rightVal;
			break;
		}
	}else if (criteria->isRightInfinity){
		/** HERE we don't need to take care of rightVal */
		outInterval->rightVal = values[1];
		switch(criteria->type){
			case GT_LT:
			case GT_LTE:
				// leftVal < x
				if(criteria->leftVal >= values[1]){
					//empty set
					outInterval->available = FALSE;
					return;
				}
				
				if(criteria->leftVal < values[0])
					outInterval->leftVal = values[0];
				else 
					outInterval->leftVal = criteria->leftVal;
			break;
			
			case GTE_LT:
			case GTE_LTE:
				// leftVal <= x
				if(criteria->leftVal > values[1]){
					//empty set
					outInterval->available = FALSE;
					return;
				}
				
				if(criteria->leftVal <= values[0])
					outInterval->leftVal = values[0];
				else 
					outInterval->leftVal = criteria->leftVal;
			break;
		}
	}else{
		switch(criteria->type){
			case GT_LT:
				// leftVal < x < rightVal
				if(criteria->leftVal >= values[1] || criteria->rightVal <= values[0]){
					//empty set
					outInterval->available = FALSE;
					return;
				}
				
				
				if(criteria->leftVal < values[0])
					outInterval->leftVal = values[0];
				else 
					outInterval->leftVal = criteria->leftVal;
				
				if(values[1] < criteria->rightVal)
					outInterval->rightVal = values[1];
				else 
					outInterval->rightVal = criteria->rightVal;
					
			break;
			
			case GT_LTE:
				if(criteria->leftVal >= values[1] || criteria->rightVal < values[0]){
					//empty set
					outInterval->available = FALSE;
					return;
				}
				
				if(criteria->leftVal < values[0])
					outInterval->leftVal = values[0];
				else 
					outInterval->leftVal = criteria->leftVal;
					
				if(values[1] <= criteria->rightVal)
					outInterval->rightVal = values[1];
				else 
					outInterval->rightVal = criteria->rightVal;
					
			break;
			
			case GTE_LT:
				// leftVal <= x < rightVal
				if(criteria->leftVal > values[1] || criteria->rightVal <= values[0]){
					//empty set
					outInterval->available = FALSE;
					return;
				}
				
				if(criteria->leftVal <= values[0])
					outInterval->leftVal = values[0];
				else 
					outInterval->leftVal = criteria->leftVal;
					
				if(values[1] < criteria->rightVal)
					outInterval->rightVal = values[1];
				else 
					outInterval->rightVal = criteria->rightVal;
			break;
			
			case GTE_LTE:
				// leftVal <= x <= rightVal
				if(criteria->leftVal > values[1] || criteria->rightVal < values[0]){
					//empty set
					outInterval->available = FALSE;
					return;
				}
				
				if(criteria->leftVal <= values[0])
					outInterval->leftVal = values[0];
				else 
					outInterval->leftVal = criteria->leftVal;
					
				if(values[1] <= criteria->rightVal)
					outInterval->rightVal = values[1];
				else 
					outInterval->rightVal = criteria->rightVal;
			break;
		}
	}
}

/**
	Number of variable of expression, it equals length of values / 2
	
	values [IN]
		This is a matrix N rows x 2 columns which N equals varCount
	
	outInterval [OUT]
		This is a matrix N row and 2 columns which each row is for each continuous interval of a single variable
		It means that N = varCount
*/
void getCombinedInterval(void *intervalObj, DATA_TYPE_FP *values, int varCount, void *outListIntervalObj){
	CombinedCriteria *criteria = (CombinedCriteria*)intervalObj;
	ListInterval *outListInterval = (ListInterval *)outListIntervalObj;
	Interval *interval;
	int i, k;
	
	for(k=0; k<varCount; k++){
		interval = (Interval*)malloc(sizeof(Interval));
		interval->available = TRUE;
		
		((Criteria*)(criteria->list[k]))->fgetInterval(criteria->list[k], values + k*2, varCount, interval);
		if( interval->available == FALSE ){
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
			outListInterval->list = (Interval**)realloc(outListInterval->list, sizeof(Interval*) * outListInterval->loggedSize);
		}
		outListInterval->list[outListInterval->size++] = interval;
	}
}

/**
	values [IN]
		This is a matrix N rows x 2 columns which N equals varCount
		
	@param outInterval
		This output parameter, it's a matrix N row and M columns which each row is for each continuous space for the expression
		It means that each row will hold a combined-interval for n-tule variables and M equal varCount * 2
*/
void getCompositeInterval(void *interval, DATA_TYPE_FP *values, int varCount, void *outDomainObj){
	CompositeCriteria *criteria = (CompositeCriteria*)interval;
	Domain *outDomain = (Domain *)outDomainObj;
	ListInterval *listIn;
	int i, k;
	
	for(i=0; i<criteria->size; i++){
		listIn = (ListInterval*)malloc(sizeof(ListInterval));
		listIn->list = NULL;
		listIn->loggedSize = 0;
		listIn->size = 0;
		
		((CombinedCriteria*)(criteria->list[i]))->fgetInterval(criteria->list[i], values, varCount, listIn);
		if(listIn->size > 0 ){
			if(outDomain->size >= outDomain->loggedSize){
				outDomain->loggedSize += INCLEN;
				outDomain->list = (ListInterval**)realloc(outDomain->list, sizeof(ListInterval*) * outDomain->loggedSize);
			}
			outDomain->list[outDomain->size++] = listIn;
		}else{
			free(listIn);
		}
	}
}

void buildCompositeCriteria(NMAST *ast, void **obj){
	switch(ast->type){
		case GT_LT:
			*obj = (Interval*)malloc(sizeof(Interval));
		break;
		
		case GTE_LT:
			*obj = (Interval*)malloc(sizeof(Interval));
		break;
		
		case GT_LTE:
			*obj = (Interval*)malloc(sizeof(Interval));
		break;
		
		case GTE_LTE:
		break;
		
		case LT:
			*obj = (Interval*)malloc(sizeof(Interval));
		break;
		
		case LTE:
		break;
		
		case GT:
		break;
		
		case GTE:
		break;
		
		case AND:
		break;
		
		case OR:
		break;
		
		default:
		break;
	}
}