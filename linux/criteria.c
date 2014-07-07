#include <stdlib.h>
#include "nmath.h"
#include "criteria.h"

int andTwoSimpleCriteria(Criteria *c1, Criteria *c2, void **out);
int orTwoSimpleCriteria(Criteria *c1, Criteria *c2, void **out);
int andTwoCriteria(void *c1, void *c2, void **out);

void copyCombinedCriteria(CombinedCriteria *from, CombinedCriteria *target){
	int i;
	
	target->loggedSize = from->loggedSize;
	target->size = from->size;
	target->list = (Criteria**)malloc(sizeof(Criteria*) * target->loggedSize);
	
	for(i=0; i<target->size; i++){
		target->list[i] = newCriteria(from->list[i]->type, 
											from->list[i]->variable,
											from->list[i]->leftVal,
											from->list[i]->rightVal, 
											from->list[i]->isLeftInfinity,
											from->list[i]->isRightInfinity);
	}
	
}

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
		if((criteria->list[i])->fcheck( criteria->list[i], values+i, 1) == TRUE)
			return TRUE;
	}
	
	return FALSE;
}

void getInterval(void *interval, const DATA_TYPE_FP *values, int unused, void *outIntervalObj){
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
void getCombinedInterval(void *intervalObj, const DATA_TYPE_FP *values, int varCount, void *outListIntervalObj){
	CombinedCriteria *criteria = (CombinedCriteria*)intervalObj;
	CombinedCriteria *outListInterval = (CombinedCriteria *)outListIntervalObj;
	Criteria *interval;
	int i, k;
	
	for(k=0; k<varCount; k++){
		interval = newCriteria(GT_LT, 'x', 0, 0, FALSE, FALSE);
		
		(criteria->list[k])->fgetInterval(criteria->list[k], values + k*2, varCount, interval);
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
void getCompositeInterval(void *interval, const DATA_TYPE_FP *values, int varCount, void *outDomainObj){
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
void buildCompositeCriteria(NMAST *ast, void **outCriteria){
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
			*outCriteria = (void*)newCriteria(ast->type, ast->variable, ast->left->value, ast->right->value, FALSE, FALSE);
		break;
		
		case LT:
			*outCriteria = (void*)newCriteria(GT_LT, ast->left->variable, 99999, ast->right->value, TRUE, FALSE);
		break;
		
		case LTE:
			*outCriteria = (void*)newCriteria(GT_LTE, ast->left->variable, 99999, ast->right->value, TRUE, FALSE);
		break;
		
		case GT:
			*outCriteria = (void*)newCriteria(GT_LT, ast->left->variable, ast->right->value, 99999, FALSE, TRUE);
		break;
		
		case GTE:
			*outCriteria = (void*)newCriteria(GTE_LT, ast->left->variable, ast->right->value, 99999, FALSE, TRUE);
		break;
		
		case AND:
			if(ast->left == NULL || ast->right == NULL) //Do we need check NULL here?
				return;
			leftResult = (void**)malloc(sizeof(void*));
			rightResult = (void**)malloc(sizeof(void*));
			buildCompositeCriteria(ast->left, leftResult);
			buildCompositeCriteria(ast->right, rightResult);
			andTwoCriteria(*leftResult, *rightResult, outCriteria);
			free(leftResult);
			free(rightResult);
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
	DATA_TYPE_FP d[2];
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
		((CombinedCriteria*)(*out))->list = (Criteria**)malloc(sizeof(Criteria*)*2);
		((CombinedCriteria*)(*out))->loggedSize = 2;
		((CombinedCriteria*)(*out))->size = 2;
		((CombinedCriteria*)(*out))->list[0] = newCriteria(c1->type, c1->variable, c1->leftVal, c1->rightVal, c1->isLeftInfinity, c1->isRightInfinity);
		((CombinedCriteria*)(*out))->list[1] = newCriteria(c2->type, c2->variable, c2->leftVal, c2->rightVal, c2->isLeftInfinity, c2->isRightInfinity);
	}
	
	return TRUE;
}

/**
	@return 
		FALSE: if it is contradiction
		TRUE: 
*/
int andTwoCriteria(void *c1, void *c2, void **out){
	int objTypeLeft = *((int*)c1);
	int objTypeRight = *((int*)c2);
	int i, result = FALSE;
	DATA_TYPE_FP d[2];
	Criteria *interval;
	CombinedCriteria* cb;
	
			
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
				if(interval->available == TRUE){
					*out = newCombinedInterval();
					copyCombinedCriteria(cb, *out);
					((CombinedCriteria*)(*out))->list[i]->variable = interval->variable;
					((CombinedCriteria*)(*out))->list[i]->leftVal = interval->leftVal;
					((CombinedCriteria*)(*out))->list[i]->rightVal = interval->rightVal;
					((CombinedCriteria*)(*out))->list[i]->isLeftInfinity = interval->isLeftInfinity;
					((CombinedCriteria*)(*out))->list[i]->isRightInfinity = interval->isRightInfinity;
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
			
	} else if( objTypeLeft == COMBINED_CRITERIA && objTypeRight == SIMPLE_CRITERIA ) {
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
int orTwoSimpleCriteria(Criteria *c1, Criteria *c2, void **out){
	DATA_TYPE_FP d[2];
	Criteria *interval;
	if(c1->variable == c2->variable){
		interval = newCriteria(GT_LT, 'x', 0, 0, FALSE, FALSE);
		d[0] = c2->leftVal;
		d[1] = c2->rightVal;
		c1->fgetInterval(c1, d, 1, (void*)interval);
		if(interval->available == FALSE){
			free(interval);
			
			*out = newCompositeInterval();
			((CompositeCriteria*)(*out))->list = (CombinedCriteria**)malloc(sizeof(CombinedCriteria*)*2);
			((CompositeCriteria*)(*out))->loggedSize = 2;
			((CompositeCriteria*)(*out))->size = 2;
			
			((CompositeCriteria*)(*out))->list[0] = newCombinedInterval();
			((CompositeCriteria*)(*out))->list[0]->list = (Criteria**)malloc(sizeof(Criteria*));
			((CompositeCriteria*)(*out))->list[0]->loggedSize = 1;
			((CompositeCriteria*)(*out))->list[0]->size = 1;
			((CompositeCriteria*)(*out))->list[0]->list[0] = c1;
			
			((CompositeCriteria*)(*out))->list[1] = newCombinedInterval();
			((CompositeCriteria*)(*out))->list[1]->list = (Criteria**)malloc(sizeof(Criteria*));
			((CompositeCriteria*)(*out))->list[1]->loggedSize = 1;
			((CompositeCriteria*)(*out))->list[1]->size = 1;
			((CompositeCriteria*)(*out))->list[1]->list[0] = c2;
			return TRUE;
		}
		
		interval->leftVal = (c1->leftVal < c2->leftVal)?c1->leftVal:c2->leftVal;
		interval->rightVal = (c1->rightVal < c2->rightVal)?c2->rightVal:c1->rightVal;
		
		*out = interval;
	}else{
		*out = newCompositeInterval();
		((CompositeCriteria*)(*out))->list = (CombinedCriteria**)malloc(sizeof(CombinedCriteria*)*2);
		((CompositeCriteria*)(*out))->loggedSize = 2;
		((CompositeCriteria*)(*out))->size = 2;
		
		((CompositeCriteria*)(*out))->list[0] = newCombinedInterval();
		((CompositeCriteria*)(*out))->list[0]->list = (Criteria**)malloc(sizeof(Criteria*));
		((CompositeCriteria*)(*out))->list[0]->loggedSize = 1;
		((CompositeCriteria*)(*out))->list[0]->size = 1;
		((CompositeCriteria*)(*out))->list[0]->list[0] = c1;
			
		((CompositeCriteria*)(*out))->list[1] = newCombinedInterval();
		((CompositeCriteria*)(*out))->list[1]->list = (Criteria**)malloc(sizeof(Criteria*));
		((CompositeCriteria*)(*out))->list[1]->loggedSize = 1;
		((CompositeCriteria*)(*out))->list[1]->size = 1;
		((CompositeCriteria*)(*out))->list[1]->list[0] = c2;
	}
	
	return TRUE;
}

/**
 * 	This get a continuous 3D space
 *	@param exp
 *	@param bd
 *	@param bdlen MUST be 4 
 *	@param epsilon
 */
FData* generateOneUnknows(NMAST* exp, const char *variables /*1 in length*/,
						Criteria *c, const DATA_TYPE_FP *bd, int bdlen, DATA_TYPE_FP epsilon){
	int elementOnRow;
	Criteria *out1;
	DATA_TYPE_FP y;
	void *tmpP;
	FData *mesh = NULL;
	RParam param;
	
	out1 = newCriteria(GT_LT, variables[0], 0, 0, FALSE, FALSE);
	getInterval(c, bd, 0, out1);
		
	if(out1->available == FALSE){
		free(out1);
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
	mesh->rowInfo = realloc(mesh->rowInfo, sizeof(int));
	param.values[0] = out1->leftVal;
	elementOnRow = 0;
	while(param.values[0] < out1->rightVal){
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
	
	free(out1);
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
FData* generateTwoUnknowsFromCombinedCriteria(NMAST* exp, const char *variables, CombinedCriteria *c, const DATA_TYPE_FP *bd, int bdlen, DATA_TYPE_FP epsilon){
	int elementOnRow;
	Criteria *out1, *out2;
	DATA_TYPE_FP y;
	void *tmpP;
	FData *mesh = NULL;
	RParam param;
	
	out1 = newCriteria(GT_LT, variables[0], 0, 0, FALSE, FALSE);
	out2 = newCriteria(GT_LT, variables[1], 0, 0, FALSE, FALSE);
	getInterval(c->list[0], bd, 0, out1);
	getInterval(c->list[1], bd+2, 0, out2);
		
	if(out1->available == FALSE || out2->available == FALSE){
		free(out1);
		free(out2);
		return NULL;
	}
	
	param.t = exp;
	param.variables = (char*)malloc(2);  //size of 1 character
	param.variables[0] = variables[0];
	param.variables[1] = variables[1];
	param.values = (DATA_TYPE_FP*)malloc(sizeof(DATA_TYPE_FP) * 2);
	param.error = 0;
	
	mesh = (FData*)malloc(sizeof(FData));
	mesh->dimension = 3;
	mesh->loggedSize = 20;
	mesh->dataSize = 0;
	mesh->data = (DATA_TYPE_FP*)malloc(sizeof(DATA_TYPE_FP) * mesh->loggedSize);
	mesh->rowCount = 0;
	param.values[0] = out1->leftVal;
	while(param.values[0] < out1->rightVal){
		param.values[1] = out2->leftVal;
		elementOnRow = 0;
		while(param.values[1] < out2->rightVal){
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
	free(out1);
	free(out2);
	free(param.values);
	free(param.variables);
	return mesh;
}

ListFData *getSpaces(Function *f, const DATA_TYPE_FP *bd, int bdlen, DATA_TYPE_FP epsilon){
	ListFData *lst = NULL;
	FData *sp;
	CombinedCriteria *comb;
	CompositeCriteria *composite;
	Criteria *c;
	void **outCriteria;
	int i, outCriteriaType;
	
	switch(f->valLen){
		case 1:
			lst = (ListFData*)malloc(sizeof(ListFData));
			if(f->domain == NULL){
				c = newCriteria(GT_LT, f->variable[0], bd[0], bd[1], FALSE, FALSE);
				sp = generateOneUnknows(f->prefix->list[0], f->variable, c, bd, 2, epsilon);
				if(sp != NULL){
					lst->loggedSize = 1;
					lst->size = 1;
					lst->list = (FData**)malloc(sizeof(FData*));
					lst->list[0] = sp;
				}
				free(c);
			} else {
				outCriteria = (void**)malloc(sizeof(void*));
				buildCompositeCriteria(f->domain->list[0], outCriteria);
				outCriteriaType = *((int*)(*outCriteria));
				switch(outCriteriaType){
					case SIMPLE_CRITERIA:
						c = (Criteria*)(*outCriteria);
						sp = generateOneUnknows(f->prefix->list[0], f->variable, c, bd, 2, epsilon);
						if(sp != NULL){
							lst->loggedSize = 1;
							lst->size = 1;
							lst->list = (FData**)malloc(sizeof(FData*));
							lst->list[0] = sp;
						}
						free(c);
					break;
					
					case COMBINED_CRITERIA:
					break;
					
					case COMPOSITE_CRITERIA:
						composite = (CompositeCriteria*)(*outCriteria);
						lst->list = (FData**)malloc(sizeof(FData*) * composite->size );
						for(i=0; i<composite->size; i++){
							comb = composite->list[i];
							c = comb->list[0];
							sp = generateOneUnknows(f->prefix->list[0], f->variable, c, bd, 2, epsilon);
							if(sp != NULL)
								lst->list[i] = sp;
							for(i=0; i<comb->size; i++){
								free(comb->list[i]);
							}
							free(comb->list);
							free(comb);
						}
						free(composite->list);
						free(composite);
					break;
				}//end switch
				free(outCriteria);
			}
		break;
		
		case 2:
			lst = (ListFData*)malloc(sizeof(ListFData));
			if(f->domain == NULL){
				comb = newCombinedInterval();
				comb->loggedSize = 2;
				comb->size = 2;
				comb->list = (Criteria **)malloc(sizeof(Criteria *) * comb->loggedSize);
				comb->list[0] = newCriteria(GT_LT, f->variable[0], bd[0], bd[1], FALSE, FALSE);
				comb->list[1] = newCriteria(GT_LT, f->variable[1], bd[2], bd[3], FALSE, FALSE);
				sp = generateTwoUnknowsFromCombinedCriteria(f->prefix->list[0], f->variable, comb, bd, 4, epsilon);
				if(sp != NULL){
					lst->loggedSize = 1;
					lst->size = 1;
					lst->list = (FData**)malloc(sizeof(FData*));
					lst->list[0] = sp;
				}
				for(i=0; i<comb->size; i++)
					free(comb->list[i]);
				free(comb->list);
				free(comb);
			} else {
				outCriteria = (void**)malloc(sizeof(void*));
				buildCompositeCriteria(f->domain->list[0], outCriteria);
				outCriteriaType = *((int*)(*outCriteria));
				switch(outCriteriaType){
					case SIMPLE_CRITERIA:
						free(*outCriteria);
					break;
					
					case COMBINED_CRITERIA:
						comb = (CombinedCriteria*)(*outCriteria);
						sp = generateTwoUnknowsFromCombinedCriteria(f->prefix->list[0], f->variable, comb, bd, 4, epsilon);
						if(sp != NULL){
							lst->loggedSize = 1;
							lst->size = 1;
							lst->list = (FData**)malloc(sizeof(FData*));
							lst->list[0] = sp;
						}
						for(i=0; i<comb->size; i++){
							free(comb->list[i]);
						}
						free(comb->list);
						free(comb);
					break;
					
					case COMPOSITE_CRITERIA:
						composite = (CompositeCriteria*)(*outCriteria);
						lst->list = (FData**)malloc(sizeof(FData*) * composite->size );
						for(i=0; i<composite->size; i++){
							comb = composite->list[i];
							sp = generateTwoUnknowsFromCombinedCriteria(f->prefix->list[0], f->variable, comb, bd, 4, epsilon);
							if(sp != NULL)
								lst->list[i] = sp;
							for(i=0; i<comb->size; i++){
								free(comb->list[i]);
							}
							free(comb->list);
							free(comb);
						}
						free(composite->list);
						free(composite);
					break;
				}
				free(outCriteria);
			}
		break;
		
		case 3:
		break;
		
		default:
		break;
	}
	
	return lst;
}