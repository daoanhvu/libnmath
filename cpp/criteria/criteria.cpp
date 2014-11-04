#include <stdlib.h>
#include "nmath.h"
#include "criteria.h"

#ifdef _TARGET_HOST_ANDROID
	#include <jni.h>
	#include <android/log.h>
	#define LOG_TAG "NativeFunction"
	#define LOG_LEVEL 10
	#define LOGI(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__);}
	#define LOGE(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__);}
#endif

extern int gErrorCode;

int andTwoSimpleCriteria(const Criteria* c1, const Criteria* c2, OutBuiltCriteria* out);
int orTwoSimpleCriteria(const Criteria* c1, const Criteria* c2, OutBuiltCriteria* out);
int andTwoCriteria(const void* c1, const void* c2, OutBuiltCriteria* out);

Criteria::Criteria() {
	this->flag = 7;
	this->type = type;
	this->variable = var;
	this->leftVal = lval;
	this->rightVal = rval;
}

Criteria::Criteria(int type, char var, float lval, float rval, 
										int leftInfinity, int rightInfinity) {
	this->flag = AVAILABLE | (leftInfinity & LEFT_INF) | (rightInfinity & RIGHT_INF) ;
	this->type = type;
	this->variable = var;
	this->leftVal = lval;
	this->rightVal = rval;
	this->fcheck = isInInterval;
	this->fgetInterval = getInterval;
}

int Criteria::isInInterval(float values) {
	int result = FALSE;
	
	if( (this->flag & 0x03) == 0x03) //Check left and right infinity is set (bit 0 and 1 = TRUE)
		return TRUE;
		
	if( (this->flag & LEFT_INF) == LEFT_INF){
		/** HERE we don't need to take care of leftVal */
		switch(this->type){
			case GT_LT:
			case GTE_LT:
				// x < rightVal
				if(values < this->rightVal)
					result = TRUE;
			break;
			
			case GT_LTE:
			case GTE_LTE:
				// x <= rightVal
				if(values <= this->rightVal)
					result = TRUE;
			break;
		}
	}else if ( (this->flag & RIGHT_INF) == RIGHT_INF ){
		/** HERE we don't need to take care of rightVal */
		switch(this->type){
			case GT_LT:
			case GT_LTE:
				// leftVal < x
				if(this->leftVal < values)
					result = TRUE;
			break;
			
			case GTE_LT:
			case GTE_LTE:
				// leftVal <= x
				if(this->leftVal <= values)
					result = TRUE;
			break;
		}
	}else{
		switch(this->type){
			case GT_LT:
				// leftVal < x < rightVal
				if( (this->leftVal < values) && (values < this->rightVal))
					result = TRUE;
			break;
			
			case GT_LTE:
				// leftVal < x <= rightVal
				if( (this->leftVal < values) && (values <= this->rightVal))
					result = TRUE;
			break;
			
			case GTE_LT:
				// leftVal <= x < rightVal
				if( (this->leftVal <= values) && (values < this->rightVal))
					result = TRUE;
			break;
			
			case GTE_LTE:
				// leftVal <= x <= rightVal
				if( (this->leftVal <= values) && (values <= this->rightVal))
					result = TRUE;
			break;
		}
	}
	
	return result;
}

Criteria* Criteria::and(const float *values ){
	Criteria *outInterval = new Criteria();
	
	outInterval->flag = outInterval->flag & (this->flag | 0xfb);
	if( (this->flag & 0x03) == 0x03){ //Check if left and right is set (bit 0 and 1 is TRUE)
		outInterval->leftVal = values[0];
		outInterval->rightVal = values[1];
		outInterval->type = GTE_LTE;
		return;
	}
		
	if( (this->flag & LEFT_INF) == LEFT_INF){
		/** HERE we don't need to take care of leftVal */
		outInterval->leftVal = values[0];
		switch(this->type){
			case GT_LT:
			case GTE_LT:
				// x < rightVal
				if(this->rightVal <= values[0]){
					//return empty set, available bit set to FALSE
					outInterval->flag = outInterval->flag & 0xfb;
					return;
				}
				outInterval->type = GTE_LT; //TODO: need to test here
				if(values[1] < this->rightVal)
					outInterval->rightVal = values[1];
				else 
					outInterval->rightVal = this->rightVal;
					//outInterval->rightVal = this->rightVal - epsilon;
			break;
			
			case GT_LTE:
			case GTE_LTE:
				// x <= rightVal
				if(this->rightVal < values[0]){
					//return empty set, available bit set to FALSE
					outInterval->flag = outInterval->flag & 0xfb;
					return;
				}
				
				outInterval->type = GTE_LTE; //TODO: need to test here
				if(values[1] <= this->rightVal)
					outInterval->rightVal = values[1];
				else 
					outInterval->rightVal = this->rightVal;
			break;
		}
	}else if ( (this->flag & RIGHT_INF) == RIGHT_INF){
		/** HERE we don't need to take care of rightVal */
		outInterval->rightVal = values[1];
		switch(this->type){
			case GT_LT:
			case GT_LTE:
				// leftVal < x
				if(this->leftVal >= values[1]){
					//return empty set, available bit set to FALSE
					outInterval->flag = outInterval->flag & 0xfb;
					return;
				}
				outInterval->type = GT_LTE; //TODO: need to test here
				if(this->leftVal < values[0])
					outInterval->leftVal = values[0];
				else 
					outInterval->leftVal = this->leftVal;
			break;
			
			case GTE_LT:
			case GTE_LTE:
				// leftVal <= x
				if(this->leftVal > values[1]){
					//return empty set, available bit set to FALSE
					outInterval->flag = outInterval->flag & 0xfb;
					return;
				}
				
				outInterval->type = GTE_LTE; //TODO: need to test here
				if(this->leftVal <= values[0])
					outInterval->leftVal = values[0];
				else 
					outInterval->leftVal = this->leftVal;
			break;
		}
	}else{
		switch(this->type){
			case GT_LT:
				// leftVal < x < rightVal
				if(this->leftVal >= values[1] || this->rightVal <= values[0]){
					//return empty set, available bit set to FALSE
					outInterval->flag = outInterval->flag & 0xfb;
					return;
				}
				
				if(this->leftVal < values[0])
					outInterval->leftVal = values[0];
				else 
					outInterval->leftVal = this->leftVal;
				
				if(values[1] < this->rightVal)
					outInterval->rightVal = values[1];
				else 
					outInterval->rightVal = this->rightVal;
					
			break;
			
			case GT_LTE:
				if(this->leftVal >= values[1] || this->rightVal < values[0]){
					//return empty set, available bit set to FALSE
					outInterval->flag = outInterval->flag & 0xfb;
					return;
				}
				
				if(this->leftVal < values[0])
					outInterval->leftVal = values[0];
				else 
					outInterval->leftVal = this->leftVal;
					
				if(values[1] <= this->rightVal)
					outInterval->rightVal = values[1];
				else 
					outInterval->rightVal = this->rightVal;
					
			break;
			
			case GTE_LT:
				// leftVal <= x < rightVal
				if(this->leftVal > values[1] || this->rightVal <= values[0]){
					//return empty set, available bit set to FALSE
					outInterval->flag = outInterval->flag & 0xfb;
					return;
				}
				
				if(this->leftVal <= values[0])
					outInterval->leftVal = values[0];
				else 
					outInterval->leftVal = this->leftVal;
					
				if(values[1] < this->rightVal)
					outInterval->rightVal = values[1];
				else 
					outInterval->rightVal = this->rightVal;
			break;
			
			case GTE_LTE:
				// leftVal <= x <= rightVal
				if(this->leftVal > values[1] || this->rightVal < values[0]){
					//return empty set, available bit set to FALSE
					outInterval->flag = outInterval->flag & 0xfb;
					return;
				}
				
				if(this->leftVal <= values[0])
					outInterval->leftVal = values[0];
				else 
					outInterval->leftVal = this->leftVal;
					
				if(values[1] <= this->rightVal)
					outInterval->rightVal = values[1];
				else 
					outInterval->rightVal = this->rightVal;
			break;
		}
	}

	return outInterval;
}

/**
	Number of variable of expression, it equals length of values / 2
	
	values [IN]
		This is a matrix N rows x 2 columns which N equals varCount
	
	outInterval [OUT]
		This is a matrix N row and 2 columns which each row is for each continuous interval of a single variable
		It means that N = varCount
*/
void getCombinedInterval(const void *intervalObj, const float *values, int varCount, void *outListIntervalObj){
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
void getCompositeInterval(const void *interval, const float *values, int varCount, void *outDomainObj){
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
	@param domain
	result [OUT] MUST an level-2 NOT-NULL pointer
*/
void buildCompositeCriteria(const NMAST *domain, const char *vars, int varCount, OutBuiltCriteria *result){
	OutBuiltCriteria leftResult;
	OutBuiltCriteria rightResult;
	CombinedCriteria *outCombinedCriteria;
	Criteria *interval;
	int i, objTypeLeft, objTypeRight /*, type*/;
	
	if(domain == NULL){
		if(varCount == 1){
			result->cr = newCriteria(GT_LT, vars[0], -9999, 9999, TRUE, TRUE);
			return;
		}

		outCombinedCriteria = newCombinedInterval();
		outCombinedCriteria->loggedSize = varCount;
		outCombinedCriteria->size = 0;
		outCombinedCriteria->list = (Criteria**)malloc(sizeof(Criteria*) * outCombinedCriteria->loggedSize);
		for(i=0; i<varCount; i++) {
			interval = newCriteria(GT_LT, vars[i], -9999, 9999, TRUE, TRUE);
			outCombinedCriteria->list[(outCombinedCriteria->size)++] = interval;
		}
		result->cr = outCombinedCriteria;
		return;
	}
	
	leftResult.cr = NULL;
	rightResult.cr = NULL;
	switch(domain->type){
		case GT_LT:
		case GTE_LT:
		case GT_LTE:
		case GTE_LTE:
			result->cr = (void*)newCriteria(domain->type, domain->variable, domain->left->value, domain->right->value, FALSE, FALSE);
		break;
		
		case LT:
			result->cr = (void*)newCriteria(GT_LT, domain->left->variable, 99999, domain->right->value, TRUE, FALSE);
		break;
		
		case LTE:
			result->cr = (void*)newCriteria(GT_LTE, domain->left->variable, 99999, domain->right->value, TRUE, FALSE);
		break;
		
		case GT:
			result->cr = (void*)newCriteria(GT_LT, domain->left->variable, domain->right->value, 99999, FALSE, TRUE);
		break;
		
		case GTE:
			result->cr = (void*)newCriteria(GTE_LT, domain->left->variable, domain->right->value, 99999, FALSE, TRUE);
		break;
		
		case AND:
			if(domain->left == NULL || domain->right == NULL) //Do we need check NULL here?
				return;
			buildCompositeCriteria(domain->left, vars, varCount, &leftResult);
			buildCompositeCriteria(domain->right, vars, varCount, &rightResult);
			andTwoCriteria(leftResult.cr, rightResult.cr, result);
		break;
		
		case OR:
			if(domain->left == NULL || domain->right == NULL) //Do we need check NULL here?
				return;
			buildCompositeCriteria(domain->left, vars, varCount, &leftResult);
			buildCompositeCriteria(domain->right, vars, varCount, &rightResult);

			objTypeLeft = *((char*)(leftResult.cr));
			objTypeRight = *((char*)(rightResult.cr));
			
			if( objTypeLeft == SIMPLE_CRITERIA && objTypeRight == SIMPLE_CRITERIA ) {
				orTwoSimpleCriteria((Criteria*)leftResult.cr, (Criteria*)rightResult.cr, result);
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
	float d[2];
	Criteria *interval;
	if(c1->variable == c2->variable){
		interval = newCriteria(GT_LT, 'x', 0, 0, FALSE, FALSE);
		d[0] = c2->leftVal;
		d[1] = c2->rightVal;
		c1->fgetInterval(c1, d, 1, (void*)interval);
		if( (interval->flag & AVAILABLE) != AVAILABLE){
			free(interval);
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
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
	float d[2];
	Criteria *interval, *cr;
	CombinedCriteria *cb, *comb1, *comb2;
	CompositeCriteria *inputComp, *outComp, *comp1, *comp2;
	void *tmp;
	OutBuiltCriteria outTmp;
	
	if( objTypeLeft == SIMPLE_CRITERIA && objTypeRight == SIMPLE_CRITERIA ) {
		result = andTwoSimpleCriteria((const Criteria*)c1, (const Criteria*)c2, out);
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
					copyCombinedCriteria(cb, (CombinedCriteria*)out->cr);
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

		/*
			We got here because c1 (SimpleCriteria) has variable that not same as vaiable of any criteria in CombinedCriteria
		*/
		cr = (Criteria*)c1;
		comb1 = newCombinedInterval();
		int i;
		comb1->loggedSize = cb->loggedSize + 1;
		comb1->size = 0;
		comb1->list = (Criteria**)malloc(sizeof(Criteria*) * comb1->loggedSize);
#ifdef DEBUG
		incNumberOfDynamicObject();
#endif
		for(i=0; i<cb->size; i++){
			comb1->list[(comb1->size)++] = newCriteria(cb->list[i]->type, 
												cb->list[i]->variable,
												cb->list[i]->leftVal,
												cb->list[i]->rightVal, 
												(cb->list[i]->flag & LEFT_INF) >> 1,
												cb->list[i]->flag & RIGHT_INF);
		}
		comb1->list[(comb1->size)++] = newCriteria(cr->type, 
												cr->variable,
												cr->leftVal,
												cr->rightVal, 
												(cr->flag & LEFT_INF) >> 1,
												cr->flag & RIGHT_INF);
		result = TRUE;
		out->cr = comb1;
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
					comp1->list = (CombinedCriteria**)(tmp==NULL?comp1->list:tmp);
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
					copyCombinedCriteria(cb, (CombinedCriteria*)out->cr);
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

		/*	
			We got here because c1 (SimpleCriteria) has variable that not same as vaiable of any criteria in CombinedCriteria
		*/
		cr = (Criteria*)c2;
		comb1 = newCombinedInterval();
		int i;
		comb1->loggedSize = cb->loggedSize + 1;
		comb1->size = 0;
		comb1->list = (Criteria**)malloc(sizeof(Criteria*) * comb1->loggedSize);
#ifdef DEBUG
		incNumberOfDynamicObject();
#endif
		for(i=0; i<cb->size; i++){
			comb1->list[(comb1->size)++] = newCriteria(cb->list[i]->type, 
												cb->list[i]->variable,
												cb->list[i]->leftVal,
												cb->list[i]->rightVal, 
												(cb->list[i]->flag & LEFT_INF) >> 1,
												cb->list[i]->flag & RIGHT_INF);
		}
		comb1->list[(comb1->size)++] = newCriteria(cr->type, 
												cr->variable,
												cr->leftVal,
												cr->rightVal, 
												(cr->flag & LEFT_INF) >> 1,
												cr->flag & RIGHT_INF);
		result = TRUE;
		out->cr = comb1;
	} else if( objTypeLeft == COMBINED_CRITERIA && objTypeRight == COMBINED_CRITERIA ) {
		comb1 = (CombinedCriteria*)c1;
		comb2 = (CombinedCriteria*)c2;
		cb = newCombinedInterval();
		cb->loggedSize = comb2->size;
		cb->list = (Criteria**)malloc(sizeof(Criteria*) * cb->loggedSize);
		for(i=0; i<comb2->size; i++) {
			cr = comb2->list[i];
			andTwoCriteria(comb1, cr, &outTmp);

			if(cb->size >= cb->loggedSize){
				cb->loggedSize += 5;
				tmp = realloc(cb->list, sizeof(Criteria*) * cb->loggedSize);
				cb->list = (tmp==NULL)?cb->list:((Criteria**)tmp);
			}
			cb->list[cb->size++] = (Criteria*)outTmp.cr;
			outTmp.cr = NULL;
		}
		out->cr = cb;
		result = TRUE;
	} else if( objTypeLeft == COMBINED_CRITERIA && objTypeRight == COMPOSITE_CRITERIA ) {
		comb1 = (CombinedCriteria*)c1;
		comp2 = (CompositeCriteria*)c2;
		outComp = newCompositeInterval();
		outComp->loggedSize = comp2->size;
		outComp->list = (CombinedCriteria**)malloc(sizeof(CombinedCriteria*) * outComp->loggedSize);
		for(i=0; i<comp2->size; i++) {
			cb = comp2->list[i];
			andTwoCriteria(comb1, cb, &outTmp);

			if(outComp->size >= outComp->loggedSize) {
				outComp->loggedSize += 5;
				tmp = realloc(outComp->list, sizeof(CombinedCriteria*) * outComp->loggedSize);
				outComp->list = (tmp==NULL)?outComp->list:((CombinedCriteria**)tmp);
			}

			outComp->list[(outComp->size)++] = (CombinedCriteria*)outTmp.cr;
			outTmp.cr = NULL;
		}
		out->cr = outComp;
		result = TRUE;
	} else if( objTypeLeft == COMPOSITE_CRITERIA && objTypeRight == SIMPLE_CRITERIA ) {
		inputComp = (CompositeCriteria*)c1;
		comp1 = newCompositeInterval();
		for(i=0; i<inputComp->size; i++) {
			cb = inputComp->list[i];
			outTmp.cr = NULL;
			if( andTwoCriteria((Criteria*)c2, cb, &outTmp) ){
				if(comp1->size >= comp1->loggedSize){
					comp1->loggedSize += 5;
					tmp = realloc(comp1->list, sizeof(CombinedCriteria*) * comp1->loggedSize);
					comp1->list = (CombinedCriteria**)(tmp==NULL?comp1->list:tmp);
				}
				comp1->list[comp1->size++] = (CombinedCriteria*)(outTmp.cr);
			}
		}
		result = TRUE;
		out->cr = comp1;
	} else if( objTypeLeft == COMPOSITE_CRITERIA && objTypeRight == COMBINED_CRITERIA ) {
		comb1 = (CombinedCriteria*)c2;
		comp2 = (CompositeCriteria*)c1;
		outComp = newCompositeInterval();
		outComp->loggedSize = comp2->size;
		outComp->list = (CombinedCriteria**)malloc(sizeof(CombinedCriteria*) * outComp->loggedSize);
		for(i=0; i<comp2->size; i++) {
			cb = comp2->list[i];
			andTwoCriteria(comb1, cb, &outTmp);

			if(outComp->size >= outComp->loggedSize) {
				outComp->loggedSize += 5;
				tmp = realloc(outComp->list, sizeof(CombinedCriteria*) * outComp->loggedSize);
				outComp->list = (tmp==NULL)?outComp->list:((CombinedCriteria**)tmp);
			}

			outComp->list[(outComp->size)++] = (CombinedCriteria*)outTmp.cr;
			outTmp.cr = NULL;
		}
		out->cr = outComp;
		result = TRUE;
	} else if( objTypeLeft == COMPOSITE_CRITERIA && objTypeRight == COMPOSITE_CRITERIA ) {
		comp1 = (CompositeCriteria*)c1;
		comp2 = (CompositeCriteria*)c2;
		outComp = newCompositeInterval();
		outComp->loggedSize = comp2->size;
		outComp->list = (CombinedCriteria**)malloc(sizeof(CombinedCriteria*) * outComp->loggedSize);
		for(i=0; i<comp2->size; i++) {
			cb = comp2->list[i];
			andTwoCriteria(comp1, cb, &outTmp);

			if(outComp->size >= outComp->loggedSize) {
				outComp->loggedSize += 5;
				tmp = realloc(outComp->list, sizeof(CombinedCriteria*) * outComp->loggedSize);
				outComp->list = (tmp==NULL)?outComp->list:((CombinedCriteria**)tmp);
			}

			outComp->list[(outComp->size)++] = (CombinedCriteria*)outTmp.cr;
			outTmp.cr = NULL;
		}
		out->cr = outComp;
		result = TRUE;
	}
	return result;
}

/**
	Need to implement
*/
CompositeCriteria Criteria::or(const Criteria *c) {
	float d[2];
	Criteria *interval;
	CombinedCriteria* cb;
	CompositeCriteria* out = new CompositeCriteria();

	if(this->variable == c->variable) {

		/*
			If this interval is close
		*/
		if( (this->flag | 0x04) == 0x04) {
			if( (c->flag | 0x04) == 0x04) {
			} else if( (c->flag | 0x05) == 0x05) {
			} else if( (c->flag | 0x06) == 0x06) {
			}
		} if( (this->flag | 0x05) == 0x05 ) { /* This interval is close on RIGHT and open on LEFT  */

		} if( (this->flag | 0x06) == 0x06 ) { /* This interval is close on LEFT and open on RIGHT  */

		}

		if( ((this->flag & 0x05)==0x05) && ((c->flag & 0x06)==0x06) ) {
			if(this->rightVal >= c->leftVal) {
				interval = new Criteria(this->variable, -9999, 9999, RIGHT_INF, LEFT_INF);
				cb = new CombinedCriteria();
				cb->and(interval);
				out->or(cb);
			} else {
				interval = this->clone();
				cb = new CombinedCriteria();
				cb->and(interval);
				out->or(cb);

				interval = c->clone();
				cb = new CombinedCriteria();
				cb->and(interval);
				out->or(cb);
			}
		} else if( ((this->flag & 0x05)==0x05) && ((c->flag & 0x05)==0x05) ) {
			if(this->rightVal >= c->rightVal) {
				interval = this->clone();
			} else {
				interval = c->clone();
			}
			cb = new CombinedCriteria();
			cb->and(interval);
			out->or(cb);

		} else if( ((this->flag & 0x05)==0x05) && ((c->flag & 0x05)==0x05) ) {

		}


		interval = newCriteria(GT_LT, 'x', 0, 0, FALSE, FALSE);
		d[0] = c2->leftVal;
		d[1] = c2->rightVal;
		c1->fgetInterval(c1, d, 1, (void*)interval);
		if( (interval->flag & AVAILABLE) == 0){
			free(interval);
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
			return TRUE;
		}
		
		interval->leftVal = (c1->leftVal < c2->leftVal)?c1->leftVal:c2->leftVal;
		interval->rightVal = (c1->rightVal < c2->rightVal)?c2->rightVal:c1->rightVal;
		
		out->cr = interval;
	}else{
		interval = this->clone();
		cb = new CombinedCriteria();
		cb->and(interval);
		out->or(cb);

		interval = c->clone();
		cb = new CombinedCriteria();
		cb->and(interval);
		out->or(cb);
	}
	
	return out;
}

/**
 * 	This get a continuous 3D space
 *	@param exp
 *	@param bd
 *	@param bdlen MUST be greater than or equals 2
 *	@param epsilon
 */
FData* generateOneUnknows(NMAST* exp, const char *variables /*1 in length*/,
						const Criteria *c, const float *bd, int bdlen, float epsilon){
	int elementOnRow;
	Criteria out1;
	float right1, lastX;
	float y;
	float values[1];
	void *tmpP;
	FData *mesh = NULL;
	DParamF param;
	
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
	param.variables[0] = variables[0];
	param.values = values;
	param.error = 0;
	
	mesh = (FData*)malloc(sizeof(FData));
	mesh->dimension = 2;
	mesh->loggedSize = 20;
	mesh->dataSize = 0;
	mesh->data = (float*)malloc(sizeof(float) * mesh->loggedSize);
	mesh->rowCount = 0;
	mesh->loggedRowCount = 1;
	mesh->rowInfo = (int*)malloc(sizeof(int));
	
	param.values[0] = out1.leftVal;
	if(out1.type == GT_LT || out1.type == GT_LTE)
		param.values[0] = out1.leftVal + epsilon;
		
	right1 = out1.rightVal;
	if(out1.type == GT_LT || out1.type == GTE_LT)
		right1 = out1.rightVal - epsilon;
		
	elementOnRow = 0;
	while(param.values[0] <= right1){
		calcF_t((void*)&param);
		y = param.retv;
		if(mesh->dataSize >= mesh->loggedSize - 2){
			mesh->loggedSize += 20;
			tmpP = realloc(mesh->data, sizeof(float) * mesh->loggedSize);
			if(tmpP != NULL)
				mesh->data = (float*)tmpP;
		}
		mesh->data[mesh->dataSize++] = param.values[0];
		mesh->data[mesh->dataSize++] = y;
		elementOnRow++;
		lastX = param.values[0];
		param.values[0] += epsilon;
	}

	if(lastX < right1 && param.values[0]>right1){
		param.values[0] = right1;
		calcF_t((void*)&param);
		y = param.retv;
		if(mesh->dataSize >= mesh->loggedSize - 2){
			mesh->loggedSize += 4;
			tmpP = realloc(mesh->data, sizeof(float) * mesh->loggedSize);
			if(tmpP != NULL)
				mesh->data = (float*)tmpP;
		}
		mesh->data[mesh->dataSize++] = param.values[0];
		mesh->data[mesh->dataSize++] = y;
		elementOnRow++;
	}

	mesh->rowInfo[mesh->rowCount++] = elementOnRow;
	return mesh;
}


/**
 * 	This get a continuous 3D space
 *	@param exp
 *	@param bd
 *	@param bdlen MUST be 4 
 *	@param epsilon
 */
FData* generateTwoUnknowsFromCombinedCriteria(NMAST* exp, const char *variables, const CombinedCriteria *c, const float *bd, 
													int bdlen, float epsilon) {
	int elementOnRow;
	Criteria out1, out2;
	float right1, left2, right2;
	float y;
	float values[2];
	void *tmpP;
	FData *mesh = NULL;
	DParamF param;
	
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
	param.variables[0] = variables[0];
	param.variables[1] = variables[1];
	param.values = values;
	param.error = 0;
	
	mesh = (FData*)malloc(sizeof(FData));
	mesh->dimension = 3;
	mesh->loggedSize = 20;
	mesh->dataSize = 0;
	mesh->data = (float*)malloc(sizeof(float) * mesh->loggedSize);
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
			calcF_t((void*)&param);
			y = param.retv;
			if(mesh->dataSize >= mesh->loggedSize - 3){
				mesh->loggedSize += 20;
				tmpP = realloc(mesh->data, sizeof(float) * mesh->loggedSize);
				if(tmpP != NULL)
					mesh->data = (float*)tmpP;
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
	return mesh;
}

ListFData *getValueRangeForOneVariableF(Function *f, const float *bd, int bdlen, float epsilon){
	ListFData *lst = NULL;
	FData *sp;
	int i;

	lst = (ListFData*)malloc(sizeof(ListFData));
	lst->loggedSize = f->prefix->size;
	lst->size = 0;
	lst->list = (FData**)malloc(sizeof(FData*) * f->prefix->size);
	for(i=0; i<f->prefix->size; i++){
		sp = generateOneUnknows(f->prefix->list[i], f->variable, (const Criteria*)(f->criterias->list[i]), bd, 2, epsilon);
		if(sp != NULL)
			lst->list[(lst->size)++] = sp;
	}

	return lst;
}

ListFData *getSpaces(Function *f, const float *bd, int bdlen, float epsilon) {
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
				c.flag = 0x07;// LEFT INFINITY & RIGHT INFINITY & AVAILABLE;
				c.type = GTE_LTE;
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
				buildCompositeCriteria(f->domain->list[0], f->variable, f->valLen, &outCriteria);
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
	descNumberOfDynamicObjectBy(j+1+2);
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
				}
				free(comb->list);
				free(comb);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(i+1+2);
#endif
			} else {
				buildCompositeCriteria(f->domain->list[0], f->variable, f->valLen, &outCriteria);
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
						}
						free(comb->list);
						free(comb);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(i+1+2);
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
							}
							free(comb->list);
							free(comb);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(j+1+2);
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

ListFData *getSpaces2(Function *f, const float *bd, int bdlen, float epsilon){
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
				buildCompositeCriteria(f->domain->list[0], f->variable, f->valLen, &outCriteria);
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
	descNumberOfDynamicObjectBy(j+1+2);
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
				}
				free(comb->list);
				free(comb);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(i+1+2);
#endif
			} else {
				buildCompositeCriteria(f->domain->list[0], f->variable, f->valLen, &outCriteria);
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
						}
						free(comb->list);
						free(comb);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(i+1+2);
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
							}
							free(comb->list);
							free(comb);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(j+1+2);
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

/**
	Get the normal vector at each point in values
*/
double *getNormalVectors(Function *f, double *values, int numOfPoint) {
	DParam d;
	DParam rp;
	NMAST *dx;
	//NMAST *dy;
	//NMAST *dz;

	switch(f->valLen){
		case 1:
			d.t = f->prefix->list[0];
			d.error = 0;
			d.returnValue = NULL;
			d.variables[0] = f->variable[0];
	
			derivative(&d);
			if(d.error != NMATH_NO_ERROR) {
				gErrorCode = d.error;
				return NULL;
			}
			dx = d.returnValue;

			rp.error = 0;
			rp.t = dx;
			rp.values = values;
			rp.variables[0] = f->variable[0];
			calc_t((void*)&rp);

			/**
				The tangen of the f at (x0) is:
				y = dx(x0)(x-x0)+y0 

				Note that if above calc_t produces a ERROR_DIV_BY_ZERO then its very likely the tangen is 
				perspendicular X axis and it have form as x = x0
			*/
			if(rp.error == NMATH_NO_ERROR){

			}else if(rp.error == ERROR_DIV_BY_ZERO){

			}
			//return rp.retv;
		break;

		case 2:
			d.t = f->prefix->list[0];
			d.error = 0;
			d.returnValue = NULL;
			d.variables[0] = f->variable[0];
			d.variables[1] = f->variable[1];
	
			derivative(&d);
			if(d.error != NMATH_NO_ERROR) {
				gErrorCode = d.error;
				return NULL;
			}
			dx = d.returnValue;
		break;

		case 3:
		break;
	}
	
	return NULL;
}

void buildCriteria(Function *f) {
	int i;
	OutBuiltCriteria outResult;

	f->criterias = (ListCriteria*)malloc(sizeof(ListCriteria));
	f->criterias->loggedSize = f->prefix->loggedSize;
	f->criterias->size = 0;
	f->criterias->list = (void**)malloc(sizeof(void*) * f->criterias->loggedSize);
	for(i=0; i<f->prefix->size; i++){
		outResult.cr = NULL;
		if(f->domain==NULL || i>=f->domain->size)
			buildCompositeCriteria(NULL, f->variable, f->valLen, &outResult);
		else
			buildCompositeCriteria(f->domain->list[i], f->variable, f->valLen, &outResult);
		f->criterias->list[(f->criterias->size)++] = outResult.cr;
	}
}