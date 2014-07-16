#include <stdlib.h>
#include "nmath.h"
#include "criteria.h"

int andTwoSimpleCriteria(const Criteria* c1, const Criteria* c2, OutBuiltCriteria* out);
int orTwoSimpleCriteria(const Criteria* c1, const Criteria* c2, OutBuiltCriteria* out);
int andTwoCriteria(const void* c1, const void* c2, OutBuiltCriteria* out);

void copyCombinedCriteria(CombinedCriteria *from, CombinedCriteria *target){
	int i;
	
	target->loggedSize = from->loggedSize;
	target->size = from->size;
	target->list = (Criteria**)malloc(sizeof(Criteria*) * target->loggedSize);
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
	for(i=0; i<target->size; i++){
		target->list[i] = newCriteria(from->list[i]->type, 
											from->list[i]->variable,
											from->list[i]->leftVal,
											from->list[i]->rightVal, 
											(from->list[i]->flag & LEFT_INF) >> 1,
											from->list[i]->flag & RIGHT_INF);
	}
	
}

Criteria *newCriteria(int type, char var, DATA_TYPE_FP lval, DATA_TYPE_FP rval, 
										int leftInfinity, int rightInfinity) {
	Criteria *result = (Criteria *)malloc(sizeof(Criteria));
	result->objectType = SIMPLE_CRITERIA;
	result->flag = AVAILABLE | (leftInfinity & LEFT_INF) | (rightInfinity & RIGHT_INF) ;
	result->type = type;
	result->variable = var;
	result->leftVal = lval;
	result->rightVal = rval;
	result->fcheck = isInInterval;
	result->fgetInterval = getInterval;
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
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
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
	return result;
}

CompositeCriteria *newCompositeInterval() {
	CompositeCriteria *result = (CompositeCriteria*)malloc(sizeof(CompositeCriteria));
	result->objectType = COMPOSITE_CRITERIA;
	result->fcheck = isInCompositeInterval;
	result->fgetInterval = getCompositeInterval;
	result->list = NULL;
	result->loggedSize = 0;
	result->size = 0;
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
	return result;
}

int isInInterval(const void *interval, DATA_TYPE_FP *values, int varCount) {
	int result = FALSE;
	Criteria* criteria = (Criteria*)interval;
	if( (criteria->flag & 0x03) == 0x03) //Check left and right infinity is set (bit 0 and 1 = TRUE)
		return (values!=NULL)?TRUE:FALSE;
		
	if( (criteria->flag & LEFT_INF) == LEFT_INF){
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
	}else if ( (criteria->flag & RIGHT_INF) == RIGHT_INF ){
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
int isInCombinedInterval(const void *interval, DATA_TYPE_FP *values, int varCount) {
	CombinedCriteria *criteria = (CombinedCriteria*)interval;
	int i;
	
	for(i=0; i<criteria->size; i++){
		if((criteria->list[i]->fcheck(criteria->list[i], values+i, 1)) == FALSE)
			return FALSE;
	}
	
	return TRUE;
}


//OR CombinedCriteria
int isInCompositeInterval(const void *interval, DATA_TYPE_FP *values, int varCount) {
	CompositeCriteria *criteria = (CompositeCriteria*)interval;
	int i;
	
	for(i=0; i<criteria->size; i++){
		if((criteria->list[i])->fcheck( criteria->list[i], values+i, 1) == TRUE)
			return TRUE;
	}
	
	return FALSE;
}

void getInterval(const void *interval, const DATA_TYPE_FP *values, int unused, void *outIntervalObj){
	Criteria *criteria = (Criteria*)interval;
	Criteria *outInterval = (Criteria*)outIntervalObj;
	
	outInterval->flag = outInterval->flag & (criteria->flag | 0xfb);
	if( (criteria->flag & 0x03) == 0x03){ //Check if left and right is set (bit 0 and 1 is TRUE)
		outInterval->leftVal = values[0];
		outInterval->rightVal = values[1];
		outInterval->type = GTE_LTE;
		return;
	}
		
	if( (criteria->flag & LEFT_INF) == LEFT_INF){
		/** HERE we don't need to take care of leftVal */
		outInterval->leftVal = values[0];
		switch(criteria->type){
			case GT_LT:
			case GTE_LT:
				// x < rightVal
				if(criteria->rightVal <= values[0]){
					//return empty set, available bit set to FALSE
					outInterval->flag = outInterval->flag & 0xfb;
					return;
				}
				outInterval->type = GTE_LT; //TODO: need to test here
				if(values[1] < criteria->rightVal)
					outInterval->rightVal = values[1];
				else 
					outInterval->rightVal = criteria->rightVal;
					//outInterval->rightVal = criteria->rightVal - epsilon;
			break;
			
			case GT_LTE:
			case GTE_LTE:
				// x <= rightVal
				if(criteria->rightVal < values[0]){
					//return empty set, available bit set to FALSE
					outInterval->flag = outInterval->flag & 0xfb;
					return;
				}
				
				outInterval->type = GTE_LTE; //TODO: need to test here
				if(values[1] <= criteria->rightVal)
					outInterval->rightVal = values[1];
				else 
					outInterval->rightVal = criteria->rightVal;
			break;
		}
	}else if ( (criteria->flag & RIGHT_INF) == RIGHT_INF){
		/** HERE we don't need to take care of rightVal */
		outInterval->rightVal = values[1];
		switch(criteria->type){
			case GT_LT:
			case GT_LTE:
				// leftVal < x
				if(criteria->leftVal >= values[1]){
					//return empty set, available bit set to FALSE
					outInterval->flag = outInterval->flag & 0xfb;
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
					//return empty set, available bit set to FALSE
					outInterval->flag = outInterval->flag & 0xfb;
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
					//return empty set, available bit set to FALSE
					outInterval->flag = outInterval->flag & 0xfb;
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
					//return empty set, available bit set to FALSE
					outInterval->flag = outInterval->flag & 0xfb;
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
					//return empty set, available bit set to FALSE
					outInterval->flag = outInterval->flag & 0xfb;
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
					//return empty set, available bit set to FALSE
					outInterval->flag = outInterval->flag & 0xfb;
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
void getCombinedInterval(const void *intervalObj, const DATA_TYPE_FP *values, int varCount, void *outListIntervalObj){
	CombinedCriteria *criteria = (CombinedCriteria*)intervalObj;
	CombinedCriteria *outListInterval = (CombinedCriteria *)outListIntervalObj;
	Criteria *interval;
	int i, k;
	
	for(k=0; k<varCount; k++){
		interval = newCriteria(GT_LT, 'x', 0, 0, FALSE, FALSE);
		
		(criteria->list[k])->fgetInterval(criteria->list[k], values + k*2, varCount, interval);
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

/**
	values [IN]
		This is a matrix N rows x 2 columns which N equals varCount
		
	@param outInterval
		This output parameter, it's a matrix N row and M columns which each row is for each continuous space for the expression
		It means that each row will hold a combined-interval for n-tule variables and M equal varCount * 2
*/
void getCompositeInterval(const void *interval, const DATA_TYPE_FP *values, int varCount, void *outDomainObj){
	CompositeCriteria *criteria = (CompositeCriteria*)interval;
	CompositeCriteria *outDomain = (CompositeCriteria *)outDomainObj;
	CombinedCriteria *listIn;
	int i;
	
	for(i=0; i<criteria->size; i++){
		listIn = newCombinedInterval();
		
		(criteria->list[i])->fgetInterval(criteria->list[i], values, varCount, listIn);
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
	obj [OUT] MUST an level-2 NOT-NULL pointer
*/
void buildCompositeCriteria(NMAST *ast, OutBuiltCriteria *result){
	OutBuiltCriteria leftResult;
	OutBuiltCriteria rightResult;
	int objTypeLeft, objTypeRight /*, type*/;
	
	if(ast == NULL){
		return;
	}
	
	leftResult.cr = NULL;
	rightResult.cr = NULL;
	switch(ast->type){
		case GT_LT:
		case GTE_LT:
		case GT_LTE:
		case GTE_LTE:
			result->cr = (void*)newCriteria(ast->type, ast->variable, ast->left->value, ast->right->value, FALSE, FALSE);
		break;
		
		case LT:
			result->cr = (void*)newCriteria(GT_LT, ast->left->variable, 99999, ast->right->value, TRUE, FALSE);
		break;
		
		case LTE:
			result->cr = (void*)newCriteria(GT_LTE, ast->left->variable, 99999, ast->right->value, TRUE, FALSE);
		break;
		
		case GT:
			result->cr = (void*)newCriteria(GT_LT, ast->left->variable, ast->right->value, 99999, FALSE, TRUE);
		break;
		
		case GTE:
			result->cr = (void*)newCriteria(GTE_LT, ast->left->variable, ast->right->value, 99999, FALSE, TRUE);
		break;
		
		case AND:
			if(ast->left == NULL || ast->right == NULL) //Do we need check NULL here?
				return;
			buildCompositeCriteria(ast->left, &leftResult);
			buildCompositeCriteria(ast->right, &rightResult);
			andTwoCriteria(leftResult.cr, rightResult.cr, result);
		break;
		
		case OR:
			if(ast->left == NULL || ast->right == NULL) //Do we need check NULL here?
				return;
			buildCompositeCriteria(ast->left, &leftResult);
			buildCompositeCriteria(ast->right, &rightResult);

			objTypeLeft = *((char*)(leftResult.cr));
			objTypeRight = *((char*)(rightResult.cr));
			
			if( objTypeLeft == SIMPLE_CRITERIA && objTypeRight == SIMPLE_CRITERIA ) {
				orTwoSimpleCriteria(leftResult.cr, rightResult.cr, result);
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
int andTwoSimpleCriteria(const Criteria *c1, const Criteria *c2, OutBuiltCriteria *out){
	DATA_TYPE_FP d[2];
	Criteria *interval;
	if(c1->variable == c2->variable){
		interval = newCriteria(GT_LT, 'x', 0, 0, FALSE, FALSE);
		d[0] = c2->leftVal;
		d[1] = c2->rightVal;
		c1->fgetInterval(c1, d, 1, (void*)interval);
		if( (interval->flag & AVAILABLE) != AVAILABLE){
			free(interval);
			return FALSE;
		}
		out->cr = interval;
	}else{
		out->cr = newCombinedInterval();
		((CombinedCriteria*)(out->cr))->list = (Criteria**)malloc(sizeof(Criteria*)*2);
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
		((CombinedCriteria*)(out->cr))->loggedSize = 2;
		((CombinedCriteria*)(out->cr))->size = 2;
		((CombinedCriteria*)(out->cr))->list[0] = newCriteria(c1->type, c1->variable, c1->leftVal, c1->rightVal, (c1->flag & LEFT_INF) >> 1, c1->flag & RIGHT_INF);
		((CombinedCriteria*)(out->cr))->list[1] = newCriteria(c2->type, c2->variable, c2->leftVal, c2->rightVal, (c2->flag & LEFT_INF) >> 1, c2->flag & RIGHT_INF);

	}
	
	return TRUE;
}

/**
	@return 
		FALSE: if it is contradiction
		TRUE: 
*/
int andTwoCriteria(const void *c1, const void *c2, OutBuiltCriteria *out){
	int objTypeLeft = *((char*)c1);
	int objTypeRight = *((char*)c2);
	int i, result = FALSE;
	DATA_TYPE_FP d[2];
	Criteria *interval;
	CombinedCriteria *cb;
	CompositeCriteria *inputComp, *comp1;
	void *tmp;
	OutBuiltCriteria outTmp;
	
	if( objTypeLeft == SIMPLE_CRITERIA && objTypeRight == SIMPLE_CRITERIA ) {
		result = andTwoSimpleCriteria(c1, c2, out);
	} else if( objTypeLeft == SIMPLE_CRITERIA && objTypeRight == COMBINED_CRITERIA ) {
		cb = (CombinedCriteria*)c2;
		i = 0;
		while(i<cb->size){
			if( ((Criteria*)c1)->variable == cb->list[i]->variable ){
				interval = newCriteria(GT_LT, 'x', 0, 0, FALSE, FALSE);
				d[0] = cb->list[i]->leftVal;
				d[1] = cb->list[i]->rightVal;
				((Criteria*)c1)->fgetInterval(c1, d, 1, (void*)interval);
				if( (interval->flag & AVAILABLE) == AVAILABLE){
					out->cr = newCombinedInterval();
					copyCombinedCriteria(cb, out->cr);
					((CombinedCriteria*)(out->cr))->list[i]->variable = interval->variable;
					((CombinedCriteria*)(out->cr))->list[i]->leftVal = interval->leftVal;
					((CombinedCriteria*)(out->cr))->list[i]->rightVal = interval->rightVal;
					((CombinedCriteria*)(out->cr))->list[i]->flag = interval->flag;
					free(interval);
					return TRUE;
				}else{
					/** ERROR: AND two contracting criteria */
					return FALSE;
				}
				break;
			}
			i++;
		}
		
	} else if( objTypeLeft == SIMPLE_CRITERIA && objTypeRight == COMPOSITE_CRITERIA ) {
		/**
			c1 and (c2 or  c3) = (c1 and c2) or (c1 and c3)
		*/
		
		inputComp = (CompositeCriteria*)c2;
		comp1 = newCompositeInterval();
		for(i=0; i<inputComp->size; i++) {
			cb = inputComp->list[i];
			outTmp.cr = NULL;
			if( andTwoCriteria((Criteria*)c1, cb, &outTmp) ){
				if(comp1->size >= comp1->loggedSize){
					comp1->loggedSize += 5;
					tmp = realloc(comp1->list, sizeof(CombinedCriteria*) * comp1->loggedSize);
					comp1->list = tmp==NULL?comp1->list:tmp;
				}
				comp1->list[comp1->size++] = (CombinedCriteria*)(outTmp.cr);
			}
		}
		result = TRUE;
		out->cr = comp1;
	} else if( objTypeLeft == COMBINED_CRITERIA && objTypeRight == SIMPLE_CRITERIA ) {
		cb = (CombinedCriteria*)c1;
		i = 0;
		while(i<cb->size){
			if( ((Criteria*)c2)->variable == cb->list[i]->variable ){
				interval = newCriteria(GT_LT, 'x', 0, 0, FALSE, FALSE);
				d[0] = cb->list[i]->leftVal;
				d[1] = cb->list[i]->rightVal;
				((Criteria*)c2)->fgetInterval(c2, d, 1, (void*)interval);
				if( (interval->flag & AVAILABLE) == AVAILABLE){
					out->cr = newCombinedInterval();
					copyCombinedCriteria(cb, out->cr);
					((CombinedCriteria*)(out->cr))->list[i]->variable = interval->variable;
					((CombinedCriteria*)(out->cr))->list[i]->leftVal = interval->leftVal;
					((CombinedCriteria*)(out->cr))->list[i]->rightVal = interval->rightVal;
					((CombinedCriteria*)(out->cr))->list[i]->flag = interval->flag;
					free(interval);
					return TRUE;
				}else{
					/** ERROR: AND two contracting criteria */
					free(interval);
					return FALSE;
				}
				break;
			}
			i++;
		}
	} else if( objTypeLeft == COMBINED_CRITERIA && objTypeRight == COMBINED_CRITERIA ) {
	} else if( objTypeLeft == COMBINED_CRITERIA && objTypeRight == COMPOSITE_CRITERIA ) {
			
	} else if( objTypeLeft == COMPOSITE_CRITERIA && objTypeRight == SIMPLE_CRITERIA ) {
	} else if( objTypeLeft == COMPOSITE_CRITERIA && objTypeRight == COMBINED_CRITERIA ) {
	} else if( objTypeLeft == COMPOSITE_CRITERIA && objTypeRight == COMPOSITE_CRITERIA ) {
			
	}
	return result;
}

/**
	Need to implement
*/
int orTwoSimpleCriteria(const Criteria *c1, const Criteria *c2, OutBuiltCriteria *out){
	DATA_TYPE_FP d[2];
	Criteria *interval;
	if(c1->variable == c2->variable){
		interval = newCriteria(GT_LT, 'x', 0, 0, FALSE, FALSE);
		d[0] = c2->leftVal;
		d[1] = c2->rightVal;
		c1->fgetInterval(c1, d, 1, (void*)interval);
		if( (interval->flag & AVAILABLE) == 0){
			free(interval);
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
			out->cr = newCompositeInterval();
			((CompositeCriteria*)(out->cr))->list = (CombinedCriteria**)malloc(sizeof(CombinedCriteria*)*2);
			((CompositeCriteria*)(out->cr))->loggedSize = 2;
			((CompositeCriteria*)(out->cr))->size = 2;
			
			((CompositeCriteria*)(out->cr))->list[0] = newCombinedInterval();
			((CompositeCriteria*)(out->cr))->list[0]->list = (Criteria**)malloc(sizeof(Criteria*));
			((CompositeCriteria*)(out->cr))->list[0]->loggedSize = 1;
			((CompositeCriteria*)(out->cr))->list[0]->size = 1;
			((CompositeCriteria*)(out->cr))->list[0]->list[0] = (Criteria*)malloc(sizeof(Criteria));
			((CompositeCriteria*)(out->cr))->list[0]->list[0]->objectType = SIMPLE_CRITERIA;
			((CompositeCriteria*)(out->cr))->list[0]->list[0]->type = c1->type;
			((CompositeCriteria*)(out->cr))->list[0]->list[0]->variable = c1->variable;
			((CompositeCriteria*)(out->cr))->list[0]->list[0]->leftVal = c1->leftVal;
			((CompositeCriteria*)(out->cr))->list[0]->list[0]->rightVal = c1->rightVal;
			((CompositeCriteria*)(out->cr))->list[0]->list[0]->fcheck = isInInterval;
			((CompositeCriteria*)(out->cr))->list[0]->list[0]->fgetInterval = getInterval;
			//set available bit, unset left and right infinity
			((CompositeCriteria*)(out->cr))->list[0]->list[0]->flag = (AVAILABLE) | (c1->flag | 0xfc);
			
			((CompositeCriteria*)(out->cr))->list[1] = newCombinedInterval();
			((CompositeCriteria*)(out->cr))->list[1]->list = (Criteria**)malloc(sizeof(Criteria*));
			((CompositeCriteria*)(out->cr))->list[1]->loggedSize = 1;
			((CompositeCriteria*)(out->cr))->list[1]->size = 1;
			((CompositeCriteria*)(out->cr))->list[1]->list[0] = (Criteria*)malloc(sizeof(Criteria));
			((CompositeCriteria*)(out->cr))->list[1]->list[0]->objectType = SIMPLE_CRITERIA;
			((CompositeCriteria*)(out->cr))->list[1]->list[0]->type = c2->type;
			((CompositeCriteria*)(out->cr))->list[1]->list[0]->variable = c2->variable;
			((CompositeCriteria*)(out->cr))->list[1]->list[0]->leftVal = c2->leftVal;
			((CompositeCriteria*)(out->cr))->list[1]->list[0]->rightVal = c2->rightVal;
			((CompositeCriteria*)(out->cr))->list[1]->list[0]->fcheck = isInInterval;
			((CompositeCriteria*)(out->cr))->list[1]->list[0]->fgetInterval = getInterval;
			//set available bit, unset left and right infinity
			((CompositeCriteria*)(out->cr))->list[1]->list[0]->flag = AVAILABLE | (c2->flag | 0xfc);
			
#ifdef DEBUG
	incNumberOfDynamicObject();
	incNumberOfDynamicObject();
	incNumberOfDynamicObject();
#endif
			return TRUE;
		}
		
		interval->leftVal = (c1->leftVal < c2->leftVal)?c1->leftVal:c2->leftVal;
		interval->rightVal = (c1->rightVal < c2->rightVal)?c2->rightVal:c1->rightVal;
		
		out->cr = interval;
	}else{
		out->cr = newCompositeInterval();
		((CompositeCriteria*)(out->cr))->list = (CombinedCriteria**)malloc(sizeof(CombinedCriteria*)*2);
		((CompositeCriteria*)(out->cr))->loggedSize = 2;
		((CompositeCriteria*)(out->cr))->size = 2;
		
		((CompositeCriteria*)(out->cr))->list[0] = newCombinedInterval();
		((CompositeCriteria*)(out->cr))->list[0]->list = (Criteria**)malloc(sizeof(Criteria*));
		((CompositeCriteria*)(out->cr))->list[0]->loggedSize = 1;
		((CompositeCriteria*)(out->cr))->list[0]->size = 1;
		((CompositeCriteria*)(out->cr))->list[0]->list[0] = (Criteria*)malloc(sizeof(Criteria));
		((CompositeCriteria*)(out->cr))->list[0]->list[0]->objectType = SIMPLE_CRITERIA;
		((CompositeCriteria*)(out->cr))->list[0]->list[0]->type = c1->type;
		((CompositeCriteria*)(out->cr))->list[0]->list[0]->variable = c1->variable;
		((CompositeCriteria*)(out->cr))->list[0]->list[0]->leftVal = c1->leftVal;
		((CompositeCriteria*)(out->cr))->list[0]->list[0]->rightVal = c1->rightVal;
		((CompositeCriteria*)(out->cr))->list[0]->list[0]->fcheck = isInInterval;
		((CompositeCriteria*)(out->cr))->list[0]->list[0]->fgetInterval = getInterval;
		//set available bit, unset left and right infinity
		((CompositeCriteria*)(out->cr))->list[0]->list[0]->flag = (AVAILABLE) | (c1->flag | 0xfc);
			
		((CompositeCriteria*)(out->cr))->list[1] = newCombinedInterval();
		((CompositeCriteria*)(out->cr))->list[1]->list = (Criteria**)malloc(sizeof(Criteria*));
		((CompositeCriteria*)(out->cr))->list[1]->loggedSize = 1;
		((CompositeCriteria*)(out->cr))->list[1]->size = 1;
		((CompositeCriteria*)(out->cr))->list[1]->list[0] = (Criteria*)malloc(sizeof(Criteria));
		((CompositeCriteria*)(out->cr))->list[1]->list[0]->objectType = SIMPLE_CRITERIA;
		((CompositeCriteria*)(out->cr))->list[1]->list[0]->type = c2->type;
		((CompositeCriteria*)(out->cr))->list[1]->list[0]->variable = c2->variable;
		((CompositeCriteria*)(out->cr))->list[1]->list[0]->leftVal = c2->leftVal;
		((CompositeCriteria*)(out->cr))->list[1]->list[0]->rightVal = c2->rightVal;
		((CompositeCriteria*)(out->cr))->list[1]->list[0]->fcheck = isInInterval;
		((CompositeCriteria*)(out->cr))->list[1]->list[0]->fgetInterval = getInterval;
		//set available bit, unset left and right infinity
		((CompositeCriteria*)(out->cr))->list[1]->list[0]->flag = AVAILABLE | (c2->flag | 0xfc);

#ifdef DEBUG
	incNumberOfDynamicObject();
	incNumberOfDynamicObject();
	incNumberOfDynamicObject();
#endif
	}
	
	return TRUE;
}

/**
 * 	This get a continuous 3D space
 *	@param exp
 *	@param bd
 *	@param bdlen MUST be greater than or equals 2
 *	@param epsilon
 */
FData* generateOneUnknows(NMAST* exp, const char *variables /*1 in length*/,
						const Criteria *c, const DATA_TYPE_FP *bd, int bdlen, DATA_TYPE_FP epsilon){
	int elementOnRow;
	Criteria out1;
	DATA_TYPE_FP right1, y;
	void *tmpP;
	FData *mesh = NULL;
	RParam param;
	
	out1.objectType = SIMPLE_CRITERIA;
	out1.type = GT_LT;
	out1.variable = variables[0];
	out1.leftVal = 0;
	out1.rightVal = 0;
	out1.fcheck = isInInterval;
	out1.fgetInterval = getInterval;
	out1.flag = AVAILABLE;
	
	getInterval(c, bd, 0, &out1);
		
	if( (out1.flag && AVAILABLE) == 0){
		return NULL;
	}
	
	param.t = exp;
	param.variables = (char*)malloc(1); //size of 1 character
	param.variables[0] = variables[0];
	param.values = (DATA_TYPE_FP*)malloc(sizeof(DATA_TYPE_FP));
	param.error = 0;
	
	mesh = (FData*)malloc(sizeof(FData));
	mesh->dimension = 2;
	mesh->loggedSize = 20;
	mesh->dataSize = 0;
	mesh->data = (DATA_TYPE_FP*)malloc(sizeof(DATA_TYPE_FP) * mesh->loggedSize);
	mesh->rowCount = 0;
	mesh->loggedRowCount = 1;
	mesh->rowInfo = (int*)malloc(sizeof(int));
	
	param.values[0] = out1.leftVal;
	if(out1.type == GT_LT || out1.type == GT_LTE)
		param.values[0] = out1.leftVal + epsilon;
		
	right1 = out1.rightVal;
	if(out1.type == GT_LT || out1.type == GTE_LT)
		param.values[0] = out1.leftVal - epsilon;
		
	elementOnRow = 0;
	while(param.values[0] <= right1){
		calc_t((void*)&param);
		y = param.retv;
		if(mesh->dataSize >= mesh->loggedSize - 2){
			mesh->loggedSize += 20;
			tmpP = realloc(mesh->data, sizeof(DATA_TYPE_FP) * mesh->loggedSize);
			if(tmpP != NULL)
				mesh->data = (DATA_TYPE_FP*)tmpP;
		}
		mesh->data[mesh->dataSize++] = param.values[0];
		mesh->data[mesh->dataSize++] = y;
		elementOnRow++;
		param.values[0] += epsilon;
	}
	mesh->rowInfo[mesh->rowCount++] = elementOnRow;
	
	free(param.values);
	free(param.variables);
	return mesh;
}


/**
 * 	This get a continuous 3D space
 *	@param exp
 *	@param bd
 *	@param bdlen MUST be 4 
 *	@param epsilon
 */
FData* generateTwoUnknowsFromCombinedCriteria(NMAST* exp, const char *variables, const CombinedCriteria *c, const DATA_TYPE_FP *bd, int bdlen, DATA_TYPE_FP epsilon){
	int elementOnRow;
	Criteria out1, out2;
	DATA_TYPE_FP right1, left2, right2, y;
	void *tmpP;
	FData *mesh = NULL;
	RParam param;
	
	out1.objectType = SIMPLE_CRITERIA;
	out1.flag = AVAILABLE;
	out1.type = GT_LT;
	out1.variable = variables[0];
	out1.leftVal = 0;
	out1.rightVal = 0;
	out1.fcheck = isInInterval;
	out1.fgetInterval = getInterval;
	
	out2.objectType = SIMPLE_CRITERIA;
	out2.flag = AVAILABLE;
	out2.type = GT_LT;
	out2.variable = variables[1];
	out2.leftVal = 0;
	out2.rightVal = 0;
	out2.fcheck = isInInterval;
	out2.fgetInterval = getInterval;
	
	getInterval(c->list[0], bd, 0, &out1);
	getInterval(c->list[1], bd+2, 0, &out2);
		
	if( (out1.flag & AVAILABLE) == 0 || (out2.flag & AVAILABLE) == 0){
		return NULL;
	}
	
	param.t = exp;
	param.variables = (char*)malloc(2);  //size of 1 character
	param.variables[0] = variables[0];
	param.variables[1] = variables[1];
	param.values = (DATA_TYPE_FP*)malloc(sizeof(DATA_TYPE_FP) * 2);
	param.error = 0;
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
	
	mesh = (FData*)malloc(sizeof(FData));
	mesh->dimension = 3;
	mesh->loggedSize = 20;
	mesh->dataSize = 0;
	mesh->data = (DATA_TYPE_FP*)malloc(sizeof(DATA_TYPE_FP) * mesh->loggedSize);
	mesh->loggedRowCount = 0;
	mesh->rowCount = 0;
	mesh->rowInfo= NULL;
#ifdef DEBUG
	incNumberOfDynamicObject();
	incNumberOfDynamicObject();
#endif

	param.values[0] = out1.leftVal;
	if(out1.type == GT_LT || out1.type == GT_LTE)
		param.values[0] = out1.leftVal + epsilon;
		
	right1 = out1.rightVal;
	if(out1.type == GT_LT || out1.type == GTE_LT)
		param.values[0] = out1.leftVal - epsilon;
	
	left2 = out2.leftVal;
	if(out2.type == GT_LT || out2.type == GT_LTE)
		left2 = out2.leftVal + epsilon;
		
	right2 = out2.rightVal;
	if(out2.type == GT_LT || out2.type == GTE_LT)
		right2 = out2.leftVal - epsilon;
	
	while(param.values[0] <= right1){
		param.values[1] = left2;
		elementOnRow = 0;
		while(param.values[1] <= right2){
			calc_t((void*)&param);
			y = param.retv;
			if(mesh->dataSize >= mesh->loggedSize - 3){
				mesh->loggedSize += 20;
				tmpP = realloc(mesh->data, sizeof(DATA_TYPE_FP) * mesh->loggedSize);
				if(tmpP != NULL)
					mesh->data = (DATA_TYPE_FP*)tmpP;
			}
			mesh->data[mesh->dataSize++] = param.values[0];
			mesh->data[mesh->dataSize++] = param.values[1];
			mesh->data[mesh->dataSize++] = y;
			param.values[1] += epsilon;
			elementOnRow++;
		}
		if(mesh->rowCount >= mesh->loggedRowCount){
			mesh->loggedRowCount += 10;
			tmpP = realloc(mesh->rowInfo, sizeof(int) * mesh->loggedRowCount);
			if(tmpP != NULL)
				mesh->rowInfo = (int*)tmpP;
		}
		
		mesh->rowInfo[mesh->rowCount++] = elementOnRow;
		param.values[0] += epsilon;
	}
	free(param.values);
	free(param.variables);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(2);
#endif
	return mesh;
}

ListFData *getSpaces(Function *f, const DATA_TYPE_FP *bd, int bdlen, DATA_TYPE_FP epsilon){
	ListFData *lst = NULL;
	FData *sp;
	CombinedCriteria *comb;
	CompositeCriteria *composite;
	Criteria c;
	OutBuiltCriteria outCriteria;
	int i, j, outCriteriaType;
	
	outCriteria.cr = NULL;
	switch(f->valLen){
		case 1:
			lst = (ListFData*)malloc(sizeof(ListFData));
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
			if(f->domain == NULL){
				c.objectType = SIMPLE_CRITERIA;
				c.flag = AVAILABLE;
				c.type = GT_LT;
				c.variable = f->variable[0];
				c.leftVal = bd[0];
				c.rightVal = bd[1];
				c.fcheck = isInInterval;
				c.fgetInterval = getInterval;
				
				sp = generateOneUnknows(f->prefix->list[0], f->variable, &c, bd, 2, epsilon);
				if(sp != NULL){
					lst->loggedSize = 1;
					lst->size = 1;
					lst->list = (FData**)malloc(sizeof(FData*));
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
					lst->list[0] = sp;
				}
			} else {
				buildCompositeCriteria(f->domain->list[0], &outCriteria);
				outCriteriaType = *((char*)(outCriteria.cr));
				switch(outCriteriaType){
					case SIMPLE_CRITERIA:
						sp = generateOneUnknows(f->prefix->list[0], f->variable, (Criteria*)(outCriteria.cr), bd, 2, epsilon);
						if(sp != NULL){
							lst->loggedSize = 1;
							lst->size = 1;
							lst->list = (FData**)malloc(sizeof(FData*));
							lst->list[0] = sp;
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
						}
						free(outCriteria.cr);
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
					break;
					
					case COMBINED_CRITERIA:
					break;
					
					case COMPOSITE_CRITERIA:
						composite = (CompositeCriteria*)(outCriteria.cr);
						lst->list = (FData**)malloc(sizeof(FData*) * composite->size );
						lst->loggedSize = composite->size;
						lst->size = 0;
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
						for(i=0; i<composite->size; i++){
							comb = composite->list[i];
							sp = generateOneUnknows(f->prefix->list[0], f->variable, comb->list[0], bd, 2, epsilon);
							if(sp != NULL){
								lst->list[lst->size++] = sp;
							}
							for(j=0; j<comb->size; j++){
								free(comb->list[j]);
							}
							free(comb->list);
							free(comb);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(2);
#endif
						}
						free(composite->list);
						free(composite);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(2);
#endif
					break;
				}//end switch
			}
		break;
		
		case 2:
			lst = (ListFData*)malloc(sizeof(ListFData));
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
			if(f->domain == NULL){
				comb = newCombinedInterval();
				comb->loggedSize = 2;
				comb->size = 2;
				comb->list = (Criteria **)malloc(sizeof(Criteria *) * comb->loggedSize);
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
				comb->list[0] = newCriteria(GT_LT, f->variable[0], bd[0], bd[1], TRUE, TRUE);
				comb->list[1] = newCriteria(GT_LT, f->variable[1], bd[2], bd[3], TRUE, TRUE);
				sp = generateTwoUnknowsFromCombinedCriteria(f->prefix->list[0], f->variable, comb, bd, 4, epsilon);
				if(sp != NULL){
					lst->loggedSize = 1;
					lst->size = 1;
					lst->list = (FData**)malloc(sizeof(FData*));
					lst->list[0] = sp;
				}
				for(i=0; i<comb->size; i++) {
					free(comb->list[i]);
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
				}
				free(comb->list);
				free(comb);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(2);
#endif
			} else {
				buildCompositeCriteria(f->domain->list[0], &outCriteria);
				outCriteriaType = *((char*)(outCriteria.cr));
				switch(outCriteriaType){
					case SIMPLE_CRITERIA:
						free(outCriteria.cr);
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
					break;
					
					case COMBINED_CRITERIA:
						comb = (CombinedCriteria*)(outCriteria.cr);
						sp = generateTwoUnknowsFromCombinedCriteria(f->prefix->list[0], f->variable, comb, bd, 4, epsilon);
						if(sp != NULL){
							lst->loggedSize = 1;
							lst->size = 1;
							lst->list = (FData**)malloc(sizeof(FData*));
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
							lst->list[0] = sp;
						}
						for(i=0; i<comb->size; i++){
							free(comb->list[i]);
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
						}
						free(comb->list);
						free(comb);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(2);
#endif
					break;
					
					case COMPOSITE_CRITERIA:
						composite = (CompositeCriteria*)(outCriteria.cr);
						lst->list = (FData**)malloc(sizeof(FData*) * composite->size );
						lst->loggedSize = composite->size;
						lst->size = 0;
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
						for(i=0; i<composite->size; i++){
							comb = composite->list[i];
							sp = generateTwoUnknowsFromCombinedCriteria(f->prefix->list[0], f->variable, comb, bd, 4, epsilon);
							if(sp != NULL)
								lst->list[lst->size++] = sp;
							for(j=0; j<comb->size; j++){
								free(comb->list[j]);
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
							}
							free(comb->list);
							free(comb);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(2);
#endif
						}
						free(composite->list);
						free(composite);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(2);
#endif
					break;
				}
			}
		break;
		
		case 3:
		break;
		
		default:
		break;
	}
	return lst;
}