#include <stdlib.h>
#include "criteria.h"

Criteria *newCriteria(int type, char var, DATA_TYPE_FP lval, DATA_TYPE_FP rval, 
										int leftInfinity, int rightInfinity) {
	Criteria *result = (Criteria *)malloc(sizeof(Criteria));
	result->objectType = SIMPLE_CRITERIA;
	result->available = TRUE;
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
	result->objectType = COMBINED_CRITERIA;
	result->fcheck = isInCombinedInterval;
	result->fgetInterval = getCombinedInterval;
	result->list = NULL;
	result->loggedSize = 0;
	result->size = 0;
	return result;
}

CompositeCriteria *newCompositeInterval() {
	CompositeCriteria *result = (CompositeCriteria *)malloc(sizeof(CompositeCriteria));
	result->objectType = COMPOSITE_CRITERIA;
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
	Criteria *outInterval = (Criteria*)outIntervalObj;
	
	outInterval->variable = criteria->variable;
	if(criteria->isLeftInfinity && criteria->isRightInfinity){
		outInterval->leftVal = values[0];
		outInterval->rightVal = values[1];
		outInterval->type = criteria->type;
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
				outInterval->type = GTE_LT; //TODO: need to test here
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
				
				outInterval->type = GTE_LTE; //TODO: need to test here
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
				outInterval->type = GT_LTE; //TODO: need to test here
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
				
				outInterval->type = GTE_LTE; //TODO: need to test here
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
	CombinedCriteria *outListInterval = (CombinedCriteria *)outListIntervalObj;
	Criteria *interval;
	int i, k;
	
	for(k=0; k<varCount; k++){
		interval = newCriteria(GT_LT, 'x', 0, 0, FALSE, FALSE);
		
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
			outListInterval->list = (Criteria**)realloc(outListInterval->list, sizeof(Criteria*) * outListInterval->loggedSize);
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
	CompositeCriteria *outDomain = (CompositeCriteria *)outDomainObj;
	CombinedCriteria *listIn;
	int i;
	
	for(i=0; i<criteria->size; i++){
		listIn = newCombinedInterval();
		
		((CombinedCriteria*)(criteria->list[i]))->fgetInterval(criteria->list[i], values, varCount, listIn);
		if(listIn->size > 0 ){
			if(outDomain->size >= outDomain->loggedSize){
				outDomain->loggedSize += INCLEN;
				outDomain->list = (CombinedCriteria**)realloc(outDomain->list, sizeof(CombinedCriteria*) * outDomain->loggedSize);
			}
			outDomain->list[outDomain->size++] = listIn;
		}else{
			free(listIn);
		}
	}
}

/**
	obj [OUT] MUST an level-2 NULL pointer
*/
void buildCompositeCriteria(NMAST *ast, void **obj){
	void **leftResult = NULL;
	void **rightResult = NULL;
	int objTypeLeft, objTypeRight /*, type*/;
	
	if(ast == NULL){
		return;
	}
	
	switch(ast->type){
		case GT_LT:
		case GTE_LT:
		case GT_LTE:
		case GTE_LTE:
			*obj = newCriteria(ast->type, ast->variable, ast->left->value, ast->right->value, FALSE, FALSE);
		break;
		
		case LT:
		break;
		
		case LTE:
		break;
		
		case GT:
		break;
		
		case GTE:
		break;
		
		case AND:
			if(ast->left == NULL || ast->right == NULL) //Do we need check NULL here?
				return;
			
			if(isComparationOperator(ast->left->type) && isComparationOperator(ast->right->type)){
				/*
				if(ast->left->type == LT)
					type = ast->right->type==LT?LT_LT:LTE_LTE;
				else if (ast->left->type == LTE)
					type = ast->right->type==LT?GT_LT:GT_LTE;
				*/	
				
				return;
			}
			
		break;
		
		case OR:
			if(ast->left == NULL || ast->right == NULL) //Do we need check NULL here?
				return;
				
			buildCompositeCriteria(ast->left, leftResult);
			buildCompositeCriteria(ast->right, rightResult);

			objTypeLeft = *((int*)(*leftResult));
			objTypeRight = *((int*)(*rightResult));
			
			if( objTypeLeft == SIMPLE_CRITERIA && objTypeRight == SIMPLE_CRITERIA ) {
			} else if( objTypeLeft == SIMPLE_CRITERIA && objTypeRight == COMBINED_CRITERIA ) {
			} else if( objTypeLeft == SIMPLE_CRITERIA && objTypeRight == COMPOSITE_CRITERIA ) {
			
			} else if( objTypeLeft == COMBINED_CRITERIA && objTypeRight == SIMPLE_CRITERIA ) {
			} else if( objTypeLeft == COMBINED_CRITERIA && objTypeRight == COMBINED_CRITERIA ) {
			} else if( objTypeLeft == COMBINED_CRITERIA && objTypeRight == COMPOSITE_CRITERIA ) {
			
			} else if( objTypeLeft == COMPOSITE_CRITERIA && objTypeRight == SIMPLE_CRITERIA ) {
			} else if( objTypeLeft == COMPOSITE_CRITERIA && objTypeRight == COMBINED_CRITERIA ) {
			} else if( objTypeLeft == COMPOSITE_CRITERIA && objTypeRight == COMPOSITE_CRITERIA ) {
			
			}
		break;
		
		default:
		break;
	}
}

/**
	Need to test here
*/
int andTwoSimpleCriteria(Criteria *c1, Criteria *c2, void **out){
	double d[2];
	Criteria *interval;
	if(c1->variable == c2->variable){
		interval = newCriteria(GT_LT, 'x', 0, 0, FALSE, FALSE);
		d[0] = c2->leftVal;
		d[1] = c2->rightVal;
		c1->fgetInterval(c1, d, 1, (void*)interval);
		if(interval->available == FALSE){
			free(interval);
			return FALSE;
		}
		*out = interval;
	}else{
		*out = newCombinedInterval();
		(*out)->list = (Criteria**)malloc(sizeof(Criteria*)*2);
		(*out)->loggedSize = 2;
		(*out)->size = 2;
		(*out)->list[0] = c1;
		(*out)->list[1] = c2;
	}
	
	return TRUE;
}

/**
	Need to implement
*/
int orTwoSimpleCriteria(Criteria *c1, Criteria *c2, void **out){
	return FALSE;
}