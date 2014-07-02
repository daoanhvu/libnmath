#include <stdlib.h>
#include "interval.h"

Interval *newInterval(char var, double lval, double rval) {
	Interval *result = (Interval *)malloc(sizeof(Interval));
	result->type = 0;
	result->variable = var;
	result->leftVal = lval;
	result->rightVal = rval;
	result->fcheck = isInInterval;
	result->fgetInterval = getInterval;
	result->isLeftInfinity = FALSE;
	result->isRightInfinity = FALSE;
	return result;
}

CombinedInterval *newCombinedInterval() {
	CombinedInterval *result = (CombinedInterval *)malloc(sizeof(CombinedInterval));
	result->fcheck = isInCombinedInterval;
	result->fgetInterval = getCombinedInterval;
	return result;
}

CompositeInterval *newCompositeInterval() {
	CompositeInterval *result = (CompositeInterval *)malloc(sizeof(CompositeInterval));
	result->fcheck = isInCompositeInterval;
	result->fgetInterval = getCompositeInterval;
	return result;
}

int isInInterval(void *interval, double *values, int varCount) {
	int result = FALSE;
	Interval *criteria = (Interval*)interval;
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

//AND Interval
int isInCombinedInterval(void *interval, double *values, int varCount) {
	CombinedInterval *criteria = (CombinedInterval*)interval;
	int i;
	
	for(i=0; i<criteria->size; i++){
		if((criteria->list[i]->fcheck(criteria->list[i], values+i, 1)) == FALSE)
			return FALSE;
	}
	
	return TRUE;
}


//OR CombinedInterval
int isInCompositeInterval(void *interval, double *values, int varCount) {
	CompositeInterval *criteria = (CompositeInterval*)interval;
	int i;
	
	for(i=0; i<criteria->size; i++){
		if(((CombinedInterval*)(criteria->list[i]))->fcheck( criteria->list[i], values+i, 1) == TRUE)
			return TRUE;
	}
	
	return FALSE;
}

void getInterval(void *interval, double *values, int varCount, double **outInterval, int *outlen){
	Interval *criteria = (Interval*)interval;
	
	*outlen = 2;
	if(criteria->isLeftInfinity && criteria->isRightInfinity){
		(*outInterval)[0] = values[0];
		(*outInterval)[1] = values[1];
		return;
	}
		
	if(criteria->isLeftInfinity){
		/** HERE we don't need to take care of leftVal */
		(*outInterval)[0] = values[0];
		switch(criteria->type){
			case GT_LT:
			case GTE_LT:
				// x < rightVal
				if(values[1] < criteria->rightVal)
					(*outInterval)[1] = values[1];
				else 
					(*outInterval)[1] = criteria->rightVal;
			break;
			
			case GT_LTE:
			case GTE_LTE:
				// x <= rightVal
				if(values[1] <= criteria->rightVal)
					(*outInterval)[1] = values[1];
				else 
					(*outInterval)[1] = criteria->rightVal;
			break;
		}
	}else if (criteria->isRightInfinity){
		/** HERE we don't need to take care of rightVal */
		(*outInterval)[1] = values[1];
		switch(criteria->type){
			case GT_LT:
			case GT_LTE:
				// leftVal < x
				if(criteria->leftVal < values[0])
					(*outInterval)[0] = values[0];
				else 
					(*outInterval)[0] = criteria->leftVal;
			break;
			
			case GTE_LT:
			case GTE_LTE:
				// leftVal <= x
				if(criteria->leftVal <= values[0])
					(*outInterval)[0] = values[0];
				else 
					(*outInterval)[0] = criteria->leftVal;
			break;
		}
	}else{
		switch(criteria->type){
			case GT_LT:
				// leftVal < x < rightVal
				if(criteria->leftVal < values[0])
					(*outInterval)[0] = values[0];
				else 
					(*outInterval)[0] = criteria->leftVal;
				
				if(values[1] < criteria->rightVal)
					(*outInterval)[1] = values[1];
				else 
					(*outInterval)[1] = criteria->rightVal;
					
			break;
			
			case GT_LTE:
				if(criteria->leftVal < values[0])
					(*outInterval)[0] = values[0];
				else 
					(*outInterval)[0] = criteria->leftVal;
					
				if(values[1] <= criteria->rightVal)
					(*outInterval)[1] = values[1];
				else 
					(*outInterval)[1] = criteria->rightVal;
					
			break;
			
			case GTE_LT:
				// leftVal <= x < rightVal
				if(criteria->leftVal <= values[0])
					(*outInterval)[0] = values[0];
				else 
					(*outInterval)[0] = criteria->leftVal;
					
				if(values[1] < criteria->rightVal)
					(*outInterval)[1] = values[1];
				else 
					(*outInterval)[1] = criteria->rightVal;
			break;
			
			case GTE_LTE:
				// leftVal <= x <= rightVal
				if(criteria->leftVal <= values[0])
					(*outInterval)[0] = values[0];
				else 
					(*outInterval)[0] = criteria->leftVal;
					
				if(values[1] <= criteria->rightVal)
					(*outInterval)[1] = values[1];
				else 
					(*outInterval)[1] = criteria->rightVal;
			break;
		}
	}
}

/**
	Number of variable of expression, it equals length of values / 2
	
	outInterval
		This output parameter, it's a matrix N row and 2 columns which each row is for each continuous interval of a single variable
		It means that N = varCount
*/
void getCombinedInterval(void *interval, double *values, int varCount, double **outInterval, int *outlen){
	CombinedInterval *criteria = (CombinedInterval*)interval;
	int i;
	
	for(i=0; i<varCount; i++){
		((Interval*)(criteria->list[i]))->fgetInterval(criteria->list[i], values + i*2, varCount, outInterval + i*2, outlen+i);
	}
}

/**
	@param outInterval
		This output parameter, it's a matrix N row and M columns which each row is for each continuous space for the expression
		It means that each row will hold a combined-interval for n-tule variables
*/
void getCompositeInterval(void *interval, double *values, int varCount, double **outInterval, int *outlen){
	//CompositeInterval *criteria = (CompositeInterval*)interval;
}